#include <stdio.h>
#include <stdlib.h>

int foo(int x,int y, int z) {
    printf("%d\n",x + y + z);
    return x + y + z;
}

void alloc(int **p, int a, int b, int c, int d) {
  *p = malloc(sizeof(int) * 4);
  (*p)[0] = a;
  (*p)[1] = b;
  (*p)[2] = c;
  (*p)[3] = d;
}