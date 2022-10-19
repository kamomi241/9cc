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
        code[i++] = stmt();
    code[i] = NULL;
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
        for(int loop = 0;!consume("}");loop++)
            node->block[loop] = stmt();
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
        if(consume("+"))
            node = new_binary(ND_ADD,node,mul());
        else if (consume("-"))
            node = new_binary(ND_SUB,node,mul());
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
     return primary();
}
Node *primary() {
      // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    Token *tok = consume_ident();
    if (tok) {
        if(consume("(")) {
            Node *node = calloc(1,sizeof(Node));
            node->kind = ND_FUNCTION;
            node->function = tok->str;
            node->len = tok->len;
            expect(")");
            return node;
        }
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            if(locals == NULL)
                lvar->offset = 8;
            else 
                lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }
    // そうでなければ数値のはず
    return new_node_num(expect_number());
}