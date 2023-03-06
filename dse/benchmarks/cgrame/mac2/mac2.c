#include <stdio.h>

volatile int * N;
int *a;
int *b;
int *c;
int *d;

int main() {
    int i;
    int n = *N;

    int sum = 0;
    int sum2 = 0;

    for (i = 1; i < n-1; i++) {
    //DFGLoop: loop
        sum += a[i] * b[i];
        sum2 += a[i] * (b[i] + 1) * c[i] * d[i];
    }

    return sum + sum2;
}
