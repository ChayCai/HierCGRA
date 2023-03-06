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
        sum += (a[i] * 2 + a[i + 1] * 2) * (b[i] * 2 + b[i + 3] * 2) * (b[i + 3] * 3) * b[i];
    }

    return sum;
}
