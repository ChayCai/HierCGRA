/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /home/mguthaus/.cvsroot/mibench/telecomm/gsm/src/gsm_decode.c,v 1.1.1.1 2000/11/06 19:54:25 mguthaus Exp $ */

#include "private.h"

#include "gsm.h"
#include "proto.h"

#include "../benchmark.h"

int gsm_decode P3((s, c, target), gsm s, gsm_byte * c, gsm_signal * target)
{
	word  	LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];

	/* GSM_MAGIC  = (*c >> 4) & 0xF; */

    gsm_byte tmp = *c; 

	if (((tmp >> 4) & 0x0F) != GSM_MAGIC) return -1;
    
    {
        __loop__(); 

        LARc[0]  = (tmp++ & 0xF) << 2;		/* 1 */
        LARc[0] |= (tmp >> 6) & 0x3;
        LARc[1]  = tmp++ & 0x3F;
        LARc[2]  = (tmp >> 3) & 0x1F;
        LARc[3]  = (tmp++ & 0x7) << 2;
        LARc[3] |= (tmp >> 6) & 0x3;
        LARc[4]  = (tmp >> 2) & 0xF;
        LARc[5]  = (tmp++ & 0x3) << 2;
        LARc[5] |= (tmp >> 6) & 0x3;
        LARc[6]  = (tmp >> 3) & 0x7;
        LARc[7]  = tmp++ & 0x7;
        Nc[0]  = (tmp >> 1) & 0x7F;
        bc[0]  = (tmp++ & 0x1) << 1;
        bc[0] |= (tmp >> 7) & 0x1;
        Mc[0]  = (tmp >> 5) & 0x3;
        xmaxc[0]  = (tmp++ & 0x1F) << 1;
        xmaxc[0] |= (tmp >> 7) & 0x1;
        xmc[0]  = (tmp >> 4) & 0x7;
        xmc[1]  = (tmp >> 1) & 0x7;
        xmc[2]  = (tmp++ & 0x1) << 2;
        xmc[2] |= (tmp >> 6) & 0x3;
        xmc[3]  = (tmp >> 3) & 0x7;
        xmc[4]  = tmp++ & 0x7;
        xmc[5]  = (tmp >> 5) & 0x7;
        xmc[6]  = (tmp >> 2) & 0x7;
        xmc[7]  = (tmp++ & 0x3) << 1;		/* 10 */
        xmc[7] |= (tmp >> 7) & 0x1;
        xmc[8]  = (tmp >> 4) & 0x7;
        xmc[9]  = (tmp >> 1) & 0x7;
        xmc[10]  = (tmp++ & 0x1) << 2;
        xmc[10] |= (tmp >> 6) & 0x3;
        xmc[11]  = (tmp >> 3) & 0x7;
        xmc[12]  = tmp++ & 0x7;
        Nc[1]  = (tmp >> 1) & 0x7F;
        bc[1]  = (tmp++ & 0x1) << 1;
        bc[1] |= (tmp >> 7) & 0x1;
        Mc[1]  = (tmp >> 5) & 0x3;
        xmaxc[1]  = (tmp++ & 0x1F) << 1;
        xmaxc[1] |= (tmp >> 7) & 0x1;
        xmc[13]  = (tmp >> 4) & 0x7;
        xmc[14]  = (tmp >> 1) & 0x7;
        xmc[15]  = (tmp++ & 0x1) << 2;
        xmc[15] |= (tmp >> 6) & 0x3;
        xmc[16]  = (tmp >> 3) & 0x7;
        xmc[17]  = tmp++ & 0x7;
        xmc[18]  = (tmp >> 5) & 0x7;
        xmc[19]  = (tmp >> 2) & 0x7;
        xmc[20]  = (tmp++ & 0x3) << 1;
        xmc[20] |= (tmp >> 7) & 0x1;
        xmc[21]  = (tmp >> 4) & 0x7;
        xmc[22]  = (tmp >> 1) & 0x7;
        xmc[23]  = (tmp++ & 0x1) << 2;
        xmc[23] |= (tmp >> 6) & 0x3;
        xmc[24]  = (tmp >> 3) & 0x7;
        xmc[25]  = tmp++ & 0x7;
        Nc[2]  = (tmp >> 1) & 0x7F;
        bc[2]  = (tmp++ & 0x1) << 1;		/* 20 */
        bc[2] |= (tmp >> 7) & 0x1;
        Mc[2]  = (tmp >> 5) & 0x3;
        xmaxc[2]  = (tmp++ & 0x1F) << 1;
        xmaxc[2] |= (tmp >> 7) & 0x1;
        xmc[26]  = (tmp >> 4) & 0x7;
        xmc[27]  = (tmp >> 1) & 0x7;
        xmc[28]  = (tmp++ & 0x1) << 2;
        xmc[28] |= (tmp >> 6) & 0x3;
        xmc[29]  = (tmp >> 3) & 0x7;
        xmc[30]  = tmp++ & 0x7;
        xmc[31]  = (tmp >> 5) & 0x7;
        xmc[32]  = (tmp >> 2) & 0x7;
        xmc[33]  = (tmp++ & 0x3) << 1;
        xmc[33] |= (tmp >> 7) & 0x1;
        xmc[34]  = (tmp >> 4) & 0x7;
        xmc[35]  = (tmp >> 1) & 0x7;
        xmc[36]  = (tmp++ & 0x1) << 2;
        xmc[36] |= (tmp >> 6) & 0x3;
        xmc[37]  = (tmp >> 3) & 0x7;
        xmc[38]  = tmp++ & 0x7;
        Nc[3]  = (tmp >> 1) & 0x7F;
        bc[3]  = (tmp++ & 0x1) << 1;
        bc[3] |= (tmp >> 7) & 0x1;
        Mc[3]  = (tmp >> 5) & 0x3;
        xmaxc[3]  = (tmp++ & 0x1F) << 1;
        xmaxc[3] |= (tmp >> 7) & 0x1;
        xmc[39]  = (tmp >> 4) & 0x7;
        xmc[40]  = (tmp >> 1) & 0x7;
        xmc[41]  = (tmp++ & 0x1) << 2;
        xmc[41] |= (tmp >> 6) & 0x3;
        xmc[42]  = (tmp >> 3) & 0x7;
        xmc[43]  = tmp++ & 0x7;			/* 30  */
        xmc[44]  = (tmp >> 5) & 0x7;
        xmc[45]  = (tmp >> 2) & 0x7;
        xmc[46]  = (tmp++ & 0x3) << 1;
        xmc[46] |= (tmp >> 7) & 0x1;
        xmc[47]  = (tmp >> 4) & 0x7;
        xmc[48]  = (tmp >> 1) & 0x7;
        xmc[49]  = (tmp++ & 0x1) << 2;
        xmc[49] |= (tmp >> 6) & 0x3;
        xmc[50]  = (tmp >> 3) & 0x7;
        xmc[51]  = tmp & 0x7;			/* 33 */
    }

    *c = tmp; 

	Gsm_Decoder(s, LARc, Nc, bc, Mc, xmaxc, xmc, target);

	return 0;
}
