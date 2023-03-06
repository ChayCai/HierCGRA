#include <stdio.h>

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
        //DFGLoop: loop
        int a_cur = a[i];
        
        int bb = (((((a[i] * 3 * c1) >> 2)) * c1 ))
               * (((((m[i] * 3 * a[i]) >> 2)) * a[i]));
//               * ((((m[i+1] * 3))));

         //int bb = a[i] * c1 * c2 * c3;

        /*
        int bb = a_cur * 3;
        bb += a_7 * 17;
        bb += a_6 * 16;
        bb += a_5 * 15;
        bb += a_4 * 14;
        bb += a_3 * 13;
        bb += a_2 * 12;
        bb += a_1 * 11;
        bb += a_0 * 10;
        */

        b[i] = bb;
    }

    return 0;
}
