#include <stdio.h>

volatile int * N;
int *a;
int *b;
int *m;

int main() {
    int i, j;
    int n = *N;

    int sum = 0;

//    int a_2 = a[i];
//    int a_1 = a[i + 1];
    for (i = 1; i < n-1; i++) {
        //DFGLoop: loop
        b[i] = a[i] * 10 + a[i + 1] * 20 + a[i+ 2] * 3;
        /*int a_0 = a[i+2];
        b[i] = a_2 * 1 + a_1 * 2 + a_0 * 3;
        a_2 = a_1;
        a_1 = a_0;
        */
    }

    return 0;
}
