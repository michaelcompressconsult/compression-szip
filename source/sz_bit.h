/*
* Copyright 1997,1998,2021 Michael Schindler michael@compressconsult.com
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
* this implements bit indexed trees for culmulative frequency storeage
* described in 
* Peter Fenwick: A New Data Structure for Cumulative Probability Tables
* Technical Report 88, Dep. of Computer Science, University of Auckland, NZ
*
* I used this structure it in Pascal in 1993? for random number generation
* for my brother to learn latin plant names
*/

#ifndef b_i_t_h
#define b_i_t_h

#include "port.h"
#include "rangecod.h"
//typedef uint4 freq;
typedef uint4 symb;

typedef struct {
   symb size, mask;
   freq *cf, *f, totfreq;
} cumtbl;

/* returns the culmulative frequency < the given symbol */
Inline freq getcf(symb s, cumtbl *tbl);

/* updates the given frequency */
#define updatefreq(_s,_tbl,_delta)      \
 { int upd_delta = (_delta);            \
   symb upd_s = (_s);                   \
   (_tbl)->f[upd_s] += upd_delta;       \
   updatecumonly(upd_s,_tbl,upd_delta); }

/* updates the given culmulative frequency */
Inline void updatecumonly(symb s, cumtbl *tbl, int delta);

/* get symbol for this culmulative frequency */
Inline symb getsym(freq f, cumtbl *tbl);

/* scales the culmulative frequency tables by 0.5 and keeps nonzero values */
void scalefreq(cumtbl *tbl);

/* scales the culmulative frequency tables by 0.5 and keeps nonzero values */
void scalefreqcond(cumtbl *tbl, uint *donotuse);

/* allocates memory for the frequency table and initializes it */
int initfreq(cumtbl *tbl, symb tblsize, freq initvalue);

/* does the obvious thing */
void freefreq(cumtbl *tbl);

#endif
