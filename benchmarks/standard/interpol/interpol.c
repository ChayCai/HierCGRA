
//========================================================================================
// 
// File Name    : filter_interp.cpp
// Description  : Main interpolation filter function
// Release Date : 23/02/2018 
// Author       : PolyU DARC Lab
//                Benjamin Carrion Schafer, Anushree Mahapatra, Jianqi Chen
//
// Revision History
//---------------------------------------------------------------------------------------
// Date         Version   Author      Description
//----------------------------------------------------------------------------------------
//23/07/2013     1.0      PolyU      Interpolation filter description
//23/02/2018     1.1      UTD        Changed coefficients
//=======================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../benchmark.h"

#define TAPS 8

__attribute__((noinline)) void write(float f1, float f2, float f3, float f4); 

float results[TAPS]; 

void interpolate ( void ) {

    float buffer[TAPS] = {0.1, -0.2, 0.3, 0.4, 0.15, 0.6, 0.37, 0.8}; 
	unsigned int n;
				
	const float coeff1[TAPS] ={-0.002109243232750,
						       0.017755702728024,
						       -0.077066788574370,
						       0.287601741388967,
						       0.862310282276647,
						       -0.107706831291022,
						       0.021638102269905,
						       -0.002423612146178};

	const float coeff2[TAPS] = {-0.003143633003860,
						       0.027185874557551,
						       -0.125223235153495,
						       0.601180543048866,
						       0.601180543048866,
						       -0.125223235153495,
						       0.027185874557551,
						       -0.003143633003860};

	const float coeff3[TAPS] = {-0.002423612146178,
						       0.021638102269905,
						       -0.107706831291022,
						       0.862310282276647,
						       0.287601741388967,
						       -0.077066788574370,
						       0.017755702728024,
						       -0.002109243232750};

	const float coeff4[TAPS-1] = {-0.000000000617302,
						       0.000000003212201,
						       -0.000000007388632,
						       1.000000009584823,
						       -0.000000007388632,
						       0.000000003212201,
						       -0.000000000617302
						       };

	while (1) {
        #pragma clang loop unroll(disable)
        for(int idx = 0; idx < TAPS; idx++)
        {
            buffer[idx] = (rand() % 10 + 0.0) / (rand() % 9 + 1.0); 
        }

        __loop__(); 
		  // FIR 1 : Sum of Products of 1st filter
		float SoP1 = 0;
        #pragma clang loop unroll(enable) 
        for(int idx = 0; idx < TAPS; idx++)
        {
            SoP1 = SoP1 + buffer[idx] * coeff1[idx];
        }
		  
		  // FIR 2 : Sum of Products of 2nd filter
		float SoP2 = 0;
        #pragma clang loop unroll(enable)
        for(int idx = 0; idx < TAPS; idx++)
        {
            SoP2 = SoP2 + buffer[idx] * coeff2[idx];
        }

		  // FIR 3 : Sum of Products of 3rd filter
		float SoP3 = 0;
        #pragma clang loop unroll(enable)
        for(int idx = 0; idx < TAPS; idx++)
        {
            SoP3 = SoP3 + buffer[idx] * coeff3[idx];
        }

		  // FIR 4 : Sum of Products of 4th filter
		float SoP4 = 0;
        #pragma clang loop unroll(enable)
        for(int idx = 0; idx < TAPS-1; idx++)
        {
            SoP4 = SoP4 + buffer[idx] * coeff4[idx];
        }

        results[0] = SoP1; 
        results[1] = SoP2; 
        results[2] = SoP3; 
        results[3] = SoP4; 
        write(SoP1, SoP2, SoP3, SoP4); 
	}
}

