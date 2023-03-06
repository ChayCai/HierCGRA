#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "../benchmark.h"

#define NCOEFF  4
#define NSTATE  (NCOEFF - 1)
#define NINPUT  8
#define NOUTPUT NINPUT

int coeff[NCOEFF] = {0, }; 
int inputFull[NSTATE + NINPUT] = {0, }; 
int output[NOUTPUT] = {0, }; 
int coeffRev[NCOEFF] = {0, }; 
int *input = inputFull + NSTATE; 
int *state = inputFull; 

void setCoeff(); 
void setInput(); 
void printOutput(); 

int main()
{
    setCoeff(); 
    setInput(); 
    
    size_t idx, jdx; 
    for(idx = 0; idx < NOUTPUT; idx++)
    {
        __loop__(); 
        output[idx] =  0; 
        output[idx] += inputFull[idx + 0] * coeffRev[0]; 
        output[idx] += inputFull[idx + 1] * coeffRev[1]; 
        output[idx] += inputFull[idx + 2] * coeffRev[2]; 
        output[idx] += inputFull[idx + 3] * coeffRev[3]; 
    }
    for(idx = 0; idx < NSTATE; idx++)
    {
        state[idx] = inputFull[NOUTPUT - 1 + idx]; 
    }
    
    printOutput(); 

    return 0; 
}

void setCoeff()
{
    size_t idx; 

    coeff[0] = 10; 
    coeff[1] = -20; 
    coeff[2] = 40; 
    coeff[3] = -10; 

    for(idx = 0; idx < NCOEFF; idx++)
    {
        coeffRev[idx] = coeff[NCOEFF - 1 - idx]; 
    }
}

void setInput()
{
    input[0] = 10; 
    input[1] = 10; 
    input[2] = 10; 
    input[3] = 10; 
    input[4] = 20; 
    input[5] = 20; 
    input[6] = 30; 
    input[7] = 30; 
}

void printOutput()
{
    size_t idx; 
    for(idx = 0; idx < NOUTPUT; idx++)
    {
        printf("%d, ", output[idx]); 
    }
    printf("\n"); 
}

