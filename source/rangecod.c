/* rangecod.h     headerfile for range encoding
*
* Copyright 1998,1999,2021 Michael Schindler michael@compressconsult.com
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*     http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*
* Range encoding is based on an article by G.N.N. Martin, submitted
* March 1979 and presented on the Video & Data Recording Conference,
* Southampton, July 24-27, 1979. If anyone can name the original
* copyright holder of that article or locate G.N.N. Martin please
* contact me; this might allow me to make that article available on
* the net for general public.
*
* Range coding is closely related to arithmetic coding, except that
* it does renormalisation in larger units than bits and is thus
* faster. An earlier version of this code was distributed as byte
* oriented arithmetic coding, but then I had no knowledge of Martin's
* paper from seventy-nine.
*
* The input and output is done by the INBYTE and OUTBYTE macros
* defined in the .c file; change them as needed; the first parameter
* passed to them is a pointer to the rangecoder structure; extend that
* structure as needed (and don't forget to initialize the values in
* start_encoding resp. start_decoding). This distribution writes to
* stdout and reads from stdin.
*
* There are no global or static var's, so if the IO is thread save the
* whole rangecoder is - unless GLOBALRANGECODER is defined.
*
* For error recovery the last 3 bytes written contain the total number
* of bytes written since starting the encoder. This can be used to
* locate the beginning of a block if you have only the end.
*
* For some application using a global coder variable may provide a better
* performance. This will allow you to use only one coder at a time and
* will destroy thread savety. To enabble this feature uncomment the
* #define GLOBALRANGECODER line below.
*
*
* There is a supplementary file called renorm95.c available at the
* website (www.compressconsult.com/rangecoder/) that changes the range
* coder to an arithmetic coder (renormalize on bits, not bytes) for speed
* comparisons.
* That file has different copyright and cannot be placed on github.
*
* define RENORM95 if you want the bitwise renormalisation. Requires renorm95.c
* Note that the old version does not write out the bytes since init.
* you should not define GLOBALRANGECODER then. This Flag is provided
* only for speed comparisons between both renormalizations, see my
* data compression conference article 1998 for details.
*/
/* #define RENORM95 */

/*
  define NOWARN if you do not expect more than 2^32 outstanding bytes 
  since I recommend restarting the coder in intervals of less than    
  2^23 symbols for error tolerance this is not expected
*/
#define NOWARN

/*
  define EXTRAFAST for increased speed; you loose compression and
  compatibility in exchange.
*/
#define EXTRAFAST

#include <stdio.h>		/* fprintf(), getchar(), putchar(), NULL */
#include "port.h"
#include "rangecod.h"

/* SIZE OF RANGE ENCODING CODE VALUES. */

#define CODE_BITS 32
#define Top_value ((code_value)1 << (CODE_BITS-1))


/* all IO is done by these macros - change them if you want to */
/* no checking is done - do it here if you want it             */
/* cod is a pointer to the used rangecoder                     */
#define outbyte(cod,x) putchar(x)
#define inbyte(cod)    getchar()


#ifdef RENORM95
#include "renorm95.c"

#else
#define SHIFT_BITS (CODE_BITS - 9)
#define EXTRA_BITS ((CODE_BITS-2) % 8 + 1)
#define Bottom_value (Top_value >> 8)

#ifdef NOWARN
#ifdef GLOBALRANGECODER
char coderversion[]="rangecode 1.1c NOWARN GLOBAL (c) 1997-1999 Michael Schindler";
#else
char coderversion[]="rangecode 1.1c NOWARN (c) 1997-1999 Michael Schindler";
#endif
#else    /*NOWARN*/
#ifdef GLOBALRANGECODER
char coderversion[]="rangecode 1.1c GLOBAL (c) 1997-1999 Michael Schindler";
#else
char coderversion[]="rangecode 1.1c (c) 1997-1999 Michael Schindler";
#endif
#endif   /*NOWARN*/
#endif   /*RENORM95*/


#ifdef GLOBALRANGECODER
/* if this is defined we'll make a global variable rngc and    */
/* make RNGC use that var; we'll also omit unneeded parameters */
static rangecoder rngc;
#define RNGC (rngc)
#define M_outbyte(a) outbyte(&rngc,a)
#define M_inbyte inbyte(&rngc)
#define enc_normalize(rc) M_enc_normalize()
#define dec_normalize(rc) M_dec_normalize()
#else
#define RNGC (*rc)
#define M_outbyte(a) outbyte(rc,a)
#define M_inbyte inbyte(rc)
#endif


/* rc is the range coder to be used                            */
/* c is written as first byte in the datastream                */
/* one could do without c, but then you have an additional if  */
/* per outputbyte.                                             */
void start_encoding( rangecoder *rc, char c, int initlength )
{   RNGC.low = 0;                /* Full code range */
    RNGC.range = Top_value;
    RNGC.buffer = c;
    RNGC.help = 0;               /* No bytes to follow */
    RNGC.bytecount = initlength;
}


#ifndef RENORM95
/* I do the normalization before I need a defined state instead of */
/* after messing it up. This simplifies starting and ending.       */
static Inline void enc_normalize( rangecoder *rc )
{   while(RNGC.range <= Bottom_value)     /* do we need renormalisation?  */
    {   if (RNGC.low < (code_value)0xff<<SHIFT_BITS)  /* no carry possible --> output */
        {   M_outbyte(RNGC.buffer);
            for(; RNGC.help; RNGC.help--)
                M_outbyte(0xff);
            RNGC.buffer = (unsigned char)(RNGC.low >> SHIFT_BITS);
        } else if (RNGC.low & Top_value) /* carry now, no future carry */
        {   M_outbyte(RNGC.buffer+1);
            for(; RNGC.help; RNGC.help--)
                M_outbyte(0);
            RNGC.buffer = (unsigned char)(RNGC.low >> SHIFT_BITS);
        } else                           /* passes on a potential carry */
#ifdef NOWARN
            RNGC.help++;
#else
            if (RNGC.bytestofollow++ == 0xffffffffL)
            {   fprintf(stderr,"Too many bytes outstanding - File too large\n");
                exit(1);
            }
#endif
        RNGC.range <<= 8;
        RNGC.low = (RNGC.low<<8) & (Top_value-1);
        RNGC.bytecount++;
    }
}
#endif


/* Encode a symbol using frequencies                         */
/* rc is the range coder to be used                          */
/* sy_f is the interval length (frequency of the symbol)     */
/* lt_f is the lower end (frequency sum of < symbols)        */
/* tot_f is the total interval length (total frequency sum)  */
/* or (faster): tot_f = (code_value)1<<shift                             */
void encode_freq( rangecoder *rc, freq sy_f, freq lt_f, freq tot_f )
{	code_value r, tmp;
	enc_normalize( rc );
	r = RNGC.range / tot_f;
	tmp = r * lt_f;
	RNGC.low += tmp;
#ifdef EXTRAFAST
    RNGC.range = r * sy_f;
#else
    if (lt_f+sy_f < tot_f)
		RNGC.range = r * sy_f;
    else
		RNGC.range -= tmp;
#endif
}

void encode_shift( rangecoder *rc, freq sy_f, freq lt_f, freq shift )
{	code_value r, tmp;
	enc_normalize( rc );
	r = RNGC.range >> shift;
	tmp = r * lt_f;
	RNGC.low += tmp;
#ifdef EXTRAFAST
	RNGC.range = r * sy_f;
#else
	if ((lt_f+sy_f) >> shift)
		RNGC.range -= tmp;
	else  
		RNGC.range = r * sy_f;
#endif
}


#ifndef RENORM95
/* Finish encoding                                           */
/* rc is the range coder to be used                          */
/* actually not that many bytes need to be output, but who   */
/* cares. I output them because decode will read them :)     */
/* the return value is the number of bytes written           */
uint4 done_encoding( rangecoder *rc )
{   uint tmp;
    enc_normalize(rc);     /* now we have a normalized state */
    RNGC.bytecount += 5;
    if ((RNGC.low & (Bottom_value-1)) < (RNGC.bytecount>>1))
       tmp = RNGC.low >> SHIFT_BITS;
    else
       tmp = (RNGC.low >> SHIFT_BITS) + 1;
    if (tmp > 0xff) /* we have a carry */
    {   M_outbyte(RNGC.buffer+1);
        for(; RNGC.help; RNGC.help--)
            M_outbyte(0);
    } else  /* no carry */
    {   M_outbyte(RNGC.buffer);
        for(; RNGC.help; RNGC.help--)
            M_outbyte(0xff);
    }
    M_outbyte(tmp & 0xff);
    M_outbyte((RNGC.bytecount>>16) & 0xff);
    M_outbyte((RNGC.bytecount>>8) & 0xff);
    M_outbyte(RNGC.bytecount & 0xff);
    return RNGC.bytecount;
}


/* Start the decoder                                         */
/* rc is the range coder to be used                          */
/* returns the char from start_encoding or EOF               */
int start_decoding( rangecoder *rc )
{   int c = M_inbyte;
    if (c==EOF)
        return EOF;
    RNGC.buffer = M_inbyte;
    RNGC.low = RNGC.buffer >> (8-EXTRA_BITS);
    RNGC.range = (code_value)1 << EXTRA_BITS;
    return c;
}


static Inline void dec_normalize( rangecoder *rc )
{   while (RNGC.range <= Bottom_value)
    {   RNGC.low = (RNGC.low<<8) | ((RNGC.buffer<<EXTRA_BITS)&0xff);
        RNGC.buffer = M_inbyte;
        RNGC.low |= RNGC.buffer >> (8-EXTRA_BITS);
        RNGC.range <<= 8;
    }
}
#endif


/* Calculate culmulative frequency for next symbol. Does NO update!*/
/* rc is the range coder to be used                          */
/* tot_f is the total frequency                              */
/* or: totf is (code_value)1<<shift                                      */
/* returns the culmulative frequency                         */
freq decode_culfreq( rangecoder *rc, freq tot_f )
{   freq tmp;
    dec_normalize(rc);
    RNGC.help = RNGC.range/tot_f;
    tmp = RNGC.low/RNGC.help;
#ifdef EXTRAFAST
    return tmp;
#else
    return (tmp>=tot_f ? tot_f-1 : tmp);
#endif
}

freq decode_culshift( rangecoder *rc, freq shift )
{   freq tmp;
    dec_normalize(rc);
    RNGC.help = RNGC.range>>shift;
    tmp = RNGC.low/RNGC.help;
#ifdef EXTRAFAST
    return tmp;
#else
    return (tmp>>shift ? ((code_value)1<<shift)-1 : tmp);
#endif
}


/* Update decoding state                                     */
/* rc is the range coder to be used                          */
/* sy_f is the interval length (frequency of the symbol)     */
/* lt_f is the lower end (frequency sum of < symbols)        */
/* tot_f is the total interval length (total frequency sum)  */
void Inline decode_update( rangecoder *rc, freq sy_f, freq lt_f, freq tot_f)
{   code_value tmp;
    tmp = RNGC.help * lt_f;
    RNGC.low -= tmp;
#ifdef EXTRAFAST
    RNGC.range = RNGC.help * sy_f;
#else
    if (lt_f + sy_f < tot_f)
        RNGC.range = RNGC.help * sy_f;
    else
        RNGC.range -= tmp;
#endif
}


/* Decode a byte/short without modelling                     */
/* rc is the range coder to be used                          */
unsigned char decode_byte(rangecoder *rc)
{   unsigned char tmp = decode_culshift(rc,8);
    decode_update( rc,1,tmp,(freq)1<<8);
    return tmp;
}

unsigned short decode_short(rangecoder *rc)
{   unsigned short tmp = decode_culshift(rc,16);
    decode_update( rc,1,tmp,(freq)1<<16);
    return tmp;
}


/* Finish decoding                                           */
/* rc is the range coder to be used                          */
void done_decoding( rangecoder *rc )
{   dec_normalize(rc);      /* normalize to use up all bytes */
}
