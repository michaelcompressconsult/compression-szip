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
*/
#ifndef rangecod_h
#define rangecod_h


#define GLOBALRANGECODER


#include "port.h"
#if 0    /* done in port.h */
#include <limits.h>
#if INT_MAX > 0xffff
typedef unsigned int uint4;
typedef unsigned short uint2;
#else
typedef unsigned long uint4;
typedef unsigned int uint2;
#endif
#endif

extern char coderversion[];

typedef uint4 code_value;       /* Type of an rangecode value       */
                                /* must accomodate 32 bits          */
/* it is highly recommended that the total frequency count is less  */
/* than 1 << 19 to minimize rounding effects.                       */
/* the total frequency count MUST be less than 1<<23                */

typedef uint4 freq; 

/* make the following private in the arithcoder object in C++	    */

typedef struct {
    uint4 low,           /* low end of interval */
          range,         /* length of interval */
          help;          /* bytes_to_follow resp. intermediate value */
    unsigned char buffer;/* buffer for input/output */
/* the following is used only when encoding */
    uint4 bytecount;     /* counter for outputed bytes  */
/* insert fields you need for input/output below this line! */
} rangecoder;


/* supply the following as methods of the arithcoder object  */
/* omit the first parameter then (C++)                       */
#ifdef GLOBALRANGECODER
#define start_encoding(rc,a,b) M_start_encoding(a,b)
#define encode_freq(rc,a,b,c) M_encode_freq(a,b,c)
#define encode_shift(rc,a,b,c) M_encode_shift(a,b,c)
#define done_encoding(rc) M_done_encoding()
#define start_decoding(rc) M_start_decoding()
#define decode_culfreq(rc,a) M_decode_culfreq(a)
#define decode_culshift(rc,a) M_decode_culshift(a)
#define decode_update(rc,a,b,c) M_decode_update(a,b,c)
#define decode_byte(rc) M_decode_byte()
#define decode_short(rc) M_decode_short()
#define done_decoding(rc) M_done_decoding()
#endif


/* Start the encoder                                         */
/* rc is the range coder to be used                          */
/* c is written as first byte in the datastream (header,...) */
void start_encoding( rangecoder *rc, char c, int initlength);


/* Encode a symbol using frequencies                         */
/* rc is the range coder to be used                          */
/* sy_f is the interval length (frequency of the symbol)     */
/* lt_f is the lower end (frequency sum of < symbols)        */
/* tot_f is the total interval length (total frequency sum)  */
/* or (a lot faster): tot_f = 1<<shift                       */
void encode_freq( rangecoder *rc, freq sy_f, freq lt_f, freq tot_f );
void encode_shift( rangecoder *rc, freq sy_f, freq lt_f, freq shift );

/* Encode a byte/short without modelling                     */
/* rc is the range coder to be used                          */
/* b,s is the data to be encoded                             */
#define encode_byte(ac,b)  encode_shift(ac,(freq)1,(freq)(b),(freq)8)
#define encode_short(ac,s) encode_shift(ac,(freq)1,(freq)(s),(freq)16)


/* Finish encoding                                           */
/* rc is the range coder to be shut down                     */
/* returns number of bytes written                           */
uint4 done_encoding( rangecoder *rc );



/* Start the decoder                                         */
/* rc is the range coder to be used                          */
/* returns the char from start_encoding or EOF               */
int start_decoding( rangecoder *rc );

/* Calculate culmulative frequency for next symbol. Does NO update!*/
/* rc is the range coder to be used                          */
/* tot_f is the total frequency                              */
/* or: totf is 1<<shift                                      */
/* returns the <= culmulative frequency                      */
freq decode_culfreq( rangecoder *rc, freq tot_f );
freq decode_culshift( rangecoder *ac, freq shift );

/* Update decoding state                                     */
/* rc is the range coder to be used                          */
/* sy_f is the interval length (frequency of the symbol)     */
/* lt_f is the lower end (frequency sum of < symbols)        */
/* tot_f is the total interval length (total frequency sum)  */
void decode_update( rangecoder *rc, freq sy_f, freq lt_f, freq tot_f);
#define decode_update_shift(rc,f1,f2,f3) decode_update((rc),(f1),(f2),(freq)1<<(f3));

/* Decode a byte/short without modelling                     */
/* rc is the range coder to be used                          */
unsigned char decode_byte(rangecoder *rc);
unsigned short decode_short(rangecoder *rc);


/* Finish decoding                                           */
/* rc is the range coder to be used                          */
void done_decoding( rangecoder *rc );

#endif
