#include "9cc.h"

LVar *locals;
Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *code[100];

void program() {
    int i = 0;
    while (!at_eof())
        code[i++] = function();
    code[i] = NULL;
}

Node *function() {
    Node *node;
    Token *tok = consume_type();
    if(tok == NULL)
        error("型がありません");
    tok = consume_ident();
    if(tok == NULL)
        error("関数がありません");
    node = calloc(1, sizeof(Node));
    node->kind = ND_FUNCBLOCK;
    node->function = calloc(1, sizeof(char));
    memcpy(node->function, tok->str,tok->len);
    expect("(");
    expect(")");
    expect("{");
    node->block = calloc(100,sizeof(Node));
    for(int i = 0; !consume("}"); i++)
        node->block[i] = stmt();
    return node;
}

Node *stmt() {
    Node *node;
    if(consume_if()) {
        expect("(");
        node = calloc(1,sizeof(Node));
        node->kind = ND_IF;
        node->lhs = expr();
        expect(")");
        node->rhs = stmt();
        if (consume_else()) {
            node->els = stmt();
        }
        return node;
    }
    if(consume_while()) {
        expect("(");
        node = calloc(1,sizeof(Node));
        node->kind = ND_WHILE;
        node->lhs = expr();
        expect(")");
        node->rhs = stmt();
        return node;
    }
    if(consume_for()) {
        expect("(");
        node = calloc(1,sizeof(Node));
        node->kind = ND_FOR;
        node->for_l = expr();
        expect(";");
        node->lhs = expr();
        expect(";");
        node->for_r = expr();
        expect(")");
        node->rhs = stmt();
        return node;
    }
    if(consume("{")) {
        node = calloc(1,sizeof(Node));
        node->block= calloc(100,sizeof(Node));
        node->kind = ND_BLOCK;
        for(int i = 0;!consume("}");i++)
            node->block[i] = stmt();
        return node;
    }
    if (consume_return()) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else {
        node = expr();
    }
    expect(";");
    return node;
}
Node *expr() {
    return assign();
}
Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_binary(ND_ASSIGN, node, assign());
    return node;
}
Node *equality() {
    Node *node = relational();
    for(;;){
        if (consume("=="))
            node = new_binary(ND_EQ,node,relational());
        else if (consume("!="))
            node = new_binary(ND_NE,node,relational());
        else
            return node;
    }
}
Node *relational() {
    Node *node = add();
    for(;;) {
        if(consume("<"))
            node = new_binary(ND_LT,node,add());
        else if (consume("<="))
            node = new_binary(ND_LE,node,add());
        else if(consume(">"))
            node = new_binary(ND_LT,add(),node);
        else if (consume(">="))
            node = new_binary(ND_LE,add(),node);
        else 
            return node;
    }
}
Node *add() {
    Node *node = mul();
    for(;;) {
        if(consume("+")) {
            Node *r = mul();
            if (node->type && node->type->ty != INT) {
                int n = node->type->ptr_to->ty == INT ? 4 : 8;
                r = new_binary(ND_MUL, r, new_node_num(n));
            }
            node = new_binary(ND_ADD, node, r);
        }
        else if (consume("-")) {
            Node *r = mul();
            if (node->type && node->type->ty != INT) {
                int n = node->type->ptr_to->ty == INT ? 4 : 8;
                r = new_binary(ND_MUL, r, new_node_num(n));
            }
            node = new_binary(ND_SUB, node, r);
        }
        else
            return node;
    }
}
Node *mul() {
    Node *node = unary();
    for (;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else 
            return node;
    }
}
Node *unary() {
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_binary(ND_SUB, new_node_num(0), unary());
    if (consume("*"))
        return new_binary(ND_DEREF, unary(), NULL);
    if(consume("&"))
        return new_binary(ND_ADDR, unary(), NULL);
    if(consume_sizeof()) {
        Node *node = add();
        int n = node->type && node->type->ty == PTR ? 8 : 4;
        return new_node_num(n);
    }
    return primary();
}
Node *primary() {
      // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    Token *tok = consume_type();
    if(tok) {
        Type *type;
        type = calloc(1, sizeof(Type));
        type->ty = INT;
        type->ptr_to = NULL;
        //ポインタ
        while(consume("*")) {
            Type *t;
            t = calloc(1, sizeof(Type));
            t->ty = PTR;
            t->ptr_to = type;
            type = t;
        }
        tok = consume_ident();
        if (tok) {
            //引数
            if(consume("(")) {
                Node *node = calloc(1,sizeof(Node));
                node->kind = ND_FUNCTION;
                node->function = tok->str;
                node->len = tok->len;
                node->block = calloc(6, sizeof(Node));
                for(int i = 0; !consume(")"); i++) {
                    node->block[i] = expr();
                    if (consume(")"))
                        break;
                    expect(",");
                }
                return node;
            }
            //配列
            if(consume("[")) {
                Type *t = calloc(1, sizeof(Type));
                t->array_size = expect_number();
                t->ty = ARRAY;
                t->ptr_to = type;
                type = t;
                expect("]");
            }
            //変数検索
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_LVAR;

            LVar *lvar = find_lvar(tok);
            if (lvar) 
                node->offset = lvar->offset;
            //変数宣言
            else {
                lvar = calloc(1, sizeof(LVar));
                lvar->next = locals;
                lvar->name = tok->str;
                lvar->len = tok->len;
                if(locals == NULL)
                    lvar->offset = 8;
                else 
                    lvar->offset = locals->offset + 8;
                lvar->type = type;
                node->type =lvar->type;
                node->name = lvar->name;
                node->offset = lvar->offset;
                locals = lvar;
            }
            return node;
            error("変数がありません");
        }
    }
    //宣言された変数の使用
    tok = consume_ident();
    if (tok) {
        LVar *lvar = find_lvar(tok);
        if (lvar) {
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_LVAR;
            node->offset = lvar->offset;
            node->type =lvar->type;
            if(consume("[")) {
                Node *add = calloc(1, sizeof(Node));
                add->kind = ND_ADD;
                add->lhs = node;
                add->rhs = expr();
                node = calloc(1, sizeof(Node));
                node->kind = ND_DEREF;
                node->lhs = add;
                expect("]");
            }
            return node;
        }
    }
    
    // そうでなければ数値のはず
    return new_node_num(expect_number());
}