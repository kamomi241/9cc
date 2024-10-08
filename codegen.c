#include "9cc.h"

void gen_lval(Node *node) {
  if (node->kind == ND_DEREF) {
    gen(node->lhs);
    return;
  }
  if (node->kind == ND_LVAR) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
  }
  else if(node->kind == ND_GLOBAL_LVAR)
    printf("  push offset %s\n",node->function);
  else 
    error("代入の左辺値が変数ではありません");
}

int label_num = 0; 
char name[100] = {0};
char *regis[6] = {"rdi","rsi","rdx","rcx","r8","r9"};

void gen(Node *node) {
  switch (node->kind) {
  case ND_GLOBAL:
    printf("%s:\n",node->function);
    printf("  .zero %d\n",node->size);
    return;
  case ND_GLOBAL_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
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
    int register_count = 0;
    for (int i = 0; node->block[i]; i++) {
      register_count++;
      gen(node->block[i]);
    }
    for (int i = register_count - 1; i >= 0; i--)
      printf("  pop %s\n", regis[i]);
    printf("  mov rax, rsp\n");
    printf("  and rax, 15\n");
    printf("  jnz .false%d\n", label_num);
    printf("  mov rax, 0\n");
    printf("  call %s\n", name);
    printf("  jmp .true%d\n", label_num);
    printf(".false%d:\n", label_num);
    printf("  sub rsp, 8\n");
    printf("  mov rax, 0\n");
    printf("  call %s\n", name);
    printf("  add rsp, 8\n");
    printf(".true%d:\n", label_num);
    printf("  push rax\n");
    return;
  case ND_FUNCBLOCK:
    printf("%s:\n",node->function);
    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");
    
    for (int i = 0; node->block[i]; i++)
      gen(node->block[i]);
      
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
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
    Type *t = node->type;
    if (t && t->ty == ARRAY)
      return;
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
  case ND_ADDR:
    gen_lval(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
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