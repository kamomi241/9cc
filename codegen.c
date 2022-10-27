#include "9cc.h"

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

int label_num = 0;
char name[100] = {0};
void gen(Node *node) {
  switch (node->kind) {
  case ND_IF:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lelse%d\n",label_num);
    gen(node->rhs);
    printf("  jmp .Lend%d\n",label_num);
    printf(".Lelse%d:\n",label_num);
    if(node->els)
      gen(node->els);
    printf(".Lend%d:\n",label_num);
    label_num++;
    return;
  case ND_WHILE:
    printf(".Lbengin%d:\n",label_num);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n",label_num);
    gen(node->rhs);
    printf("  jmp .Lbengin%d\n",label_num);
    printf(".Lend%d:\n",label_num);
    label_num++;
    return;
  case ND_FOR:
    gen(node->for_l);
    printf(".Lbegin%d:\n",label_num);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n",label_num);
    gen(node->rhs);
    gen(node->for_r);
    printf("  jmp .Lbegin%d\n",label_num);
    printf(".Lend%d:\n",label_num);
    label_num++;
    return;
  case ND_BLOCK:
    for(int i = 0;node->block[i];i++) {
      gen(node->block[i]);
      printf("  pop rax\n");
    }
    return;
  case ND_FUNCTION:
    memcpy(name, node->function, node->len);
    for (int i = 0; node->block[i]; i++) {
      gen(node->block[i]);
    }
    printf("  pop rsi\n");
    printf("  pop rdi\n");
    printf("  call %s\n", name);
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }
  printf("  push rax\n");
}