//============================================================================
// 
//
// File Name    : aes.cpp
// Description  : ADVANCED ENCRYPTION STANDARD (AES)  128-bit
// Release Date : 12/03/29
// Author       : AES Versoin 1.4 pjc.co.jp. AES Versoin 1.4 pjc.co.jp
// Modified     : PolyU
//
// Revision History
//---------------------------------------------------------------------------
// Date     	Author   	     Version     Description
//---------------------------------------------------------------------------
// 2009         PJC.CO.JP              1.0       Original ANSI C description
// 16/07/2013  PolyU                   1.1       Converted into Synthesizable SystemC
// 18/01/2015  Shunagnan Liu, PolyU   1.2       Merged encryption and decryption
//
//============================================================================
#include "../benchmark.h"

#define Nb 4 // Number of columns comprising the State

/************************************************************/
__attribute__((noinline)) int mul(int dt, int n)
{
  __loop__(); 

  int x=0;

  x <<= 1;
  if(x&0x100) x = (x ^ 0x1b) & 0xff;
  if((n & 8)) x ^= dt;

  x <<= 1;
  if(x&0x100) x = (x ^ 0x1b) & 0xff;
  if((n & 4)) x ^= dt;

  x <<= 1;
  if(x&0x100) x = (x ^ 0x1b) & 0xff;
  if((n & 2)) x ^= dt;

  return(x);
}

/************************************************************/

__attribute__((noinline)) int dataget(int *data, int n) {
  int ret;

  ret = (data[(n>>2)] >> ((n & 0x3) * 8)) & 0xff;
  return (ret);
}


/************************************************************/

__attribute__((noinline)) void MixColumns(int *data)
{
  int i,i4,x;
  int a,b,c,d;
  
	a = 2;
	b = 3;
	c = 1;
	d = 1;

  for(i=0; i<Nb; i++)
    {
      i4 = i*4;
      x  =  mul(dataget(data,i4+0),a) ^
			mul(dataget(data,i4+1),b) ^
			mul(dataget(data,i4+2),c) ^
			mul(dataget(data,i4+3),d);
      x |= (mul(dataget(data,i4+1),a) ^
			mul(dataget(data,i4+2),b) ^
			mul(dataget(data,i4+3),c) ^
			mul(dataget(data,i4+0),d)) << 8;
      x |= (mul(dataget(data,i4+2),a) ^
			mul(dataget(data,i4+3),b) ^
			mul(dataget(data,i4+0),c) ^
			mul(dataget(data,i4+1),d)) << 16;
      x |= (mul(dataget(data,i4+3),a) ^
			mul(dataget(data,i4+0),b) ^
			mul(dataget(data,i4+1),c) ^
			mul(dataget(data,i4+2),d)) << 24;
      data[i] = x;
    }
}

