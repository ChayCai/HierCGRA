// Simple loop pipelining test
// Author: Andrew Canis
// Date: June 29, 2012

#include <stdio.h>

#define SIZE 20

volatile int a[SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
volatile int b[SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

int main(void) {

    int i;
    volatile int mul[SIZE];

    for (i = 0; i < SIZE; i++) {
        //DFGLoop: loop
        mul[i] = a[i] * b[i];
    }

    return mul[SIZE-1];
}
