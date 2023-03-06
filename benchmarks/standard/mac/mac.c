#include <stdio.h>

#include "../benchmark.h"

volatile int * N;
int *a;
int *b;

int main() {
    int i;
    int n = *N;

    int sum = 0;

    for (i = 1; i < n-1; i++) {
        __loop__(); 
        sum += a[i] * b[i];
    }

    return sum;
}
