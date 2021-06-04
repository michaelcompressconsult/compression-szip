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
* I used this structure it in Pascal in 1987? for random number generation
* for a program for my brother to learn latin plant names
*/

#include "sz_bit.h"
#include <stdio.h>
#include <stdlib.h>

/* returns the culmulative frequency <= the given symbol */
Inline freq getcf(symb s, cumtbl *tbl)
{ freq sum, *cf;
  sum = (cf=tbl->cf)[s+1] - tbl->f[s];
  s++;
  while (s &= s-1)
    sum += cf[s];
  return sum;
}

/* updates the given culmulative frequency */
Inline void updatecumonly(symb s, cumtbl *tbl, int delta)
{ freq *cf;
  symb size;
  tbl->totfreq += delta;
  s++;
  cf = tbl->cf;
  size = tbl->size;
  while (s<=size) {
     cf[s] += delta;
     s = (s | (s-1)) + 1;
  }
}

/* get symbol for this culmulative frequency */
Inline symb getsym(freq f, cumtbl *tbl)
{ symb s, m, x, size;
  freq *cf;
  m = tbl->mask;
  size = tbl->size;
  cf = tbl->cf;
  s = 0;
  do {
    if ((x=s|m) <= size && f >= cf[x]) {
      f -= cf[x];
      s = x;
    }
  } while (m >>= 1);
  return s;
}

#define RECALC(cond) {                   \
  symb i;                                \
  freq *cf;                              \
  cf = tbl->cf;                          \
  tbl->totfreq = 0;                      \
  for (i=1; i<=tbl->size; i<<=1) {       \
    symb j;                              \
    for (j=i; j<=tbl->size; j+= i<<1) {  \
      symb k;                            \
      if (cond)                          \
        cf[j] = 0;                       \
      else                               \
        tbl->totfreq += cf[j] = tbl->f[j-1];\
      for (k=i>>1; k; k>>=1)             \
        cf[j] += cf[j-k];                \
    } /* end for j */                    \
  } /* end for i */                      \
}

/* scales the culmulative frequency tables by 0.5 and keeps nonzero values */
void scalefreq(cumtbl *tbl)
{ freq *f, *endf;
  for (f=tbl->f, endf = f+tbl->size; f<endf; f++)
     *f = (*f + 1)>>1;
  RECALC(0);
}

/* scales the culmulative frequency tables by 0.5 and keeps nonzero values */
void scalefreqcond(cumtbl *tbl, uint *donotuse)
{ freq *f, *endf;
  for (f=tbl->f, endf = f+tbl->size; f<endf; f++)
     *f = (*f + 1)>>1;
  RECALC(donotuse[j-1]!=3);
}

/* allocates memory for the frequency table and initializes it */
int initfreq(cumtbl *tbl, symb tblsize, freq initvalue)
{ symb i;
  if ((tbl->f = (freq*)malloc(2*tblsize*sizeof(freq))) == NULL) return 1;
  tbl->cf = tbl->f + tblsize - 1;
  tbl->size = tblsize;
  tbl->mask = 1;
  while (tblsize>>=1)
     tbl->mask <<=1;
  for (i=0; i<tbl->size; i++)
     tbl->f[i] = initvalue;
  RECALC(0);
  return 0;
}

/* does the obvious thing */
void freefreq(cumtbl *tbl)
{ free(tbl->f);
}
