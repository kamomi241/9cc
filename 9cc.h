#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//tokenize

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
    TK_RETURN,   // return
    TK_IF,       //if
    TK_ELSE,     //else
    TK_WHILE,    //while
    TK_FOR,      //for
    TK_BLOCK,   //{}
    TK_TYPE,    //型
    TK_SIZEOF,  //sizeof
    TK_GLOBAL,  //global変数
} TokenKind;
typedef struct Token Token;
// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};

//ポインタ,配列
typedef struct Type Type;
struct Type {
  enum { INT, PTR, ARRAY } ty;
  struct Type *ptr_to;
  size_t array_size;
};

typedef struct LVar LVar;
// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
  Type *type;
};
// ローカル変数
extern LVar *locals;
//グローバル変数

extern LVar *global;

LVar *find_lvar(Token *tok);
LVar *global_lvar(Token *tok);
Token *consume_ident();

extern char *user_input;
extern Token *token;
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str,int len);
bool startswith(char *p, char *q);
Token *tokenize();

//parse

// 抽象構文木のノードの種類
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_ASSIGN, // =
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_IF,  // if
    ND_WHILE,  //while
    ND_FOR,  //for
    ND_LVAR,   // ローカル変数
    ND_NUM, // 整数
    ND_RETURN, //return
    ND_BLOCK,  //{}
    ND_FUNCTION, //関数
    ND_FUNCBLOCK,//関数の定義
    ND_ADDR,     //単項&
    ND_DEREF,    //単項*
    ND_ARRAY,    //ポインタ
    ND_GLOBAL,   //global変数
    ND_GLOBAL_LVAR,
} NodeKind;
typedef struct Node Node;
// 抽象構文木のノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    Node *els;     //else用
    Node *for_l;   //for初期化
    Node *for_r;   //for演算
    Node **block;   //{}
    char *function; //関数
    int len;        //変数の長さ
    int val;       // kindがND_NUMの場合のみ使う
    int offset;    // kindがND_LVARの場合のみ使うRBP
    Type *type;    //ポインタか確認
    char *name;    //配列の変数
    int size;
};

extern Node *code[];

Node *new_node(NodeKind kind);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

Token *consume_sizeof();
Token *consume_return();
Token *consume_if();
Token *consume_else();
Token *consume_while();
Token *consume_for();
Token *consume_type();

void program();
Node *function();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

//codegen

void gen_lval(Node *node);
void gen(Node *node);