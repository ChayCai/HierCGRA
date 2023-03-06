#include <stdio.h>

volatile int * N;
volatile int *a;
volatile int *b;
int *c;

int main() {
    int i;
    int n = *N;

    int sum = 0;
    for (i = 1; i < n-1; i++) {
    //DFGLoop: loop
        c[i] *= a[i+1] + b[i-1];
        sum += c[i];
    }

    return sum;
}
