#include <stdio.h>

static volatile int* n;

int main() {

    int N = *n;
    int sum = 0;
    int i;
    for (i = 0; i < N; i++) {
        //DFGLoop: loop
        sum += i*3;
    }
    printf("sum = %d\n", sum);

    return sum;
}
