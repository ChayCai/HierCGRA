#include <stdio.h>

#include "../benchmark.h"

volatile int * N;
int *a;
int *b;
int *m;
int c1 = 2;
int c2 = 3;
int c3 = 6;

int main() {
    int i, j;
    int n = *N;

    int sum = 0;

    int a_0 = a[0];
    int a_1 = a[1];
    int a_2 = a[2];
    int a_3 = a[3];
    int a_4 = a[4];
    int a_5 = a[5];
    int a_6 = a[6];
    int a_7 = a[7];
    for (i = 8; i < n-1; i++) {
        __loop__(); 
        
        int a_cur = a[i];
        
        int bb = (((((a[i] * 3 * c1) >> 2)) * c1 ))
               * (((((m[i] * 3 * a[i]) >> 2)) * a[i]));

        b[i] = bb;
    }

    return 0;
}
