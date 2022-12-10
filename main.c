#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }
  // トークナイズしてパースする
  // 結果はcodeに保存される
  user_input = argv[1];
  token = tokenize();
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf(".bss\n");

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    if(code[i]->kind == ND_FUNCBLOCK)
      printf(".text\n");
    gen(code[i]);
  }

  // エピローグ
  // 最後の式の結果がRAXに残っているのでそれが返り値になる
  
  return 0;
}