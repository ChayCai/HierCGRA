#include <stdio.h>

volatile int * a;//[N] = {1,2,3,4,5,6,7,8,9,10};

volatile int * n;
// Simple accumulation
int main() {

    int N = *n;
    int sum = 0;
    int i;
    for (i = 0; i < N; i++) {
        //DFGLoop: loop
        sum += a[i];
    }
    printf("sum = %d\n", sum);

    return sum;
}
