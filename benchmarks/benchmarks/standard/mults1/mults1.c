#include <stdio.h>

#include "../benchmark.h"

volatile int * N;
int *a;
int *b;
int *m;

int main() {
    int i, j;
    int n = *N;

    int sum = 0;

    for (i = 1; i < n-1; i++) {
        __loop__(); 
        sum += a[i] * 10 + a[i + 1] * 20 + a[i+ 2] * 39 + a[i + 3] * 15;
    }

    return sum;
}
