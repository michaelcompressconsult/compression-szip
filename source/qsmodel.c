/* qsmodel.c     headerfile for quasistatic probability model
*
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
* Qsmodel is a quasistatic probability model that periodically
* (at chooseable intervals) updates probabilities of symbols;
* it also allows to initialize probabilities. Updating is done more
* frequent in the beginning, so it adapts very fast even without
* initialisation.
*
* it provides function for creation, deletion, query for probabilities
* and symbols and model updating.
*
* for usage see example.c
*/

#include "qsmodel.h"
#include <stdio.h>
#include <stdlib.h>

/* default tablesize 1<<TBLSHIFT */
#define TBLSHIFT 7

/* rescale frequency counts */
static void dorescale( qsmodel *m)
{   int i, cf, missing;
    if (m->nextleft)  /* we have some more before actual rescaling */
    {   m->incr++;
        m->left = m->nextleft;
        m->nextleft = 0;
        return;
    }
    if (m->rescale < m->targetrescale)  /* double rescale interval if needed */
    {   m->rescale <<= 1;
        if (m->rescale > m->targetrescale)
            m->rescale = m->targetrescale;
    }
    cf = missing = m->cf[m->n];  /* do actual rescaling */
    for(i=m->n-1; i; i--)
    {   int tmp = m->newf[i];
        cf -= tmp;
        m->cf[i] = cf;
        tmp = tmp>>1 | 1;
        missing -= tmp;
        m->newf[i] = tmp;
    }
    if (cf!=m->newf[0])
    {   fprintf(stderr,"BUG: rescaling left %d total frequency\n",cf);
        deleteqsmodel(m);
        exit(1);
    }
    m->newf[0] = m->newf[0]>>1 | 1;
    missing -= m->newf[0];
    m->incr = missing / m->rescale;
    m->nextleft = missing % m->rescale;
    m->left = m->rescale - m->nextleft;
    if (m->search != NULL)
    {   i=m->n;
        while (i)
        {   int start, end;
            end = (m->cf[i]-1) >> m->searchshift;
            i--;
            start = m->cf[i] >> m->searchshift;
            while (start<=end)
            {   m->search[start] = i;
                start++;
            }
        }
    }
}


/* initialisation of qsmodel                           */
/* m   qsmodel to be initialized                       */
/* n   number of symbols in that model                 */
/* lg_totf  base2 log of total frequency count         */
/* rescale  desired rescaling interval, should be < 1<<(lg_totf+1) */
/* init  array of int's to be used for initialisation (NULL ok) */
/* compress  set to 1 on compression, 0 on decompression */
void initqsmodel( qsmodel *m, int n, int lg_totf, int rescale, int *init, int compress )
{   m->n = n;
    m->targetrescale = rescale;
    m->searchshift = lg_totf - TBLSHIFT;
    if (m->searchshift < 0)
        m->searchshift = 0;
    m->cf = (uint2*) malloc((n+1)*sizeof(uint2));
    m->newf = (uint2*) malloc((n+1)*sizeof(uint2));
    m->cf[n] = 1<<lg_totf;
    m->cf[0] = 0;
    if (compress)
        m->search = NULL;
    else
    {   m->search = (uint2*) malloc(((1<<TBLSHIFT)+1)*sizeof(uint2));
        m->search[1<<TBLSHIFT] = n-1;
    }
    resetqsmodel(m, init);
}


/* reinitialisation of qsmodel                         */
/* m   qsmodel to be initialized                       */
/* init  array of int's to be used for initialisation (NULL ok) */
void resetqsmodel( qsmodel *m, int *init)
{   int i, end, initval;
    m->rescale = m->n>>4 | 2;
    m->nextleft = 0;
    if (init == NULL)
    {   initval = m->cf[m->n] / m->n;
        end = m->cf[m->n] % m->n;
        for (i=0; i<end; i++)
            m->newf[i] = initval+1;
        for (; i<m->n; i++)
            m->newf[i] = initval;
    } else
        for(i=0; i<m->n; i++)
            m->newf[i] = init[i];
    dorescale(m);
}


/* deletion of qsmodel m                               */
void deleteqsmodel( qsmodel *m )
{   free(m->cf);
    free(m->newf);
    if (m->search != NULL)
        free(m->search);
}


/* retrieval of estimated frequencies for a symbol     */
/* m   qsmodel to be questioned                        */
/* sym  symbol for which data is desired; must be <n   */
/* sy_f frequency of that symbol                       */
/* lt_f frequency of all smaller symbols together      */
/* the total frequency is 1<<lg_totf                   */
void qsgetfreq( qsmodel *m, int sym, int *sy_f, int *lt_f )
{   *sy_f = m->cf[sym+1] - (*lt_f = m->cf[sym]);
}	


/* find out symbol for a given cumulative frequency    */
/* m   qsmodel to be questioned                        */
/* lt_f  cumulative frequency                          */
int qsgetsym( qsmodel *m, int lt_f )
{   int lo, hi;
    uint2 *tmp;
    tmp = m->search+(lt_f>>m->searchshift);
    lo = *tmp;
    hi = *(tmp+1) + 1;
    while (lo+1 < hi )
    {   int mid = (lo+hi)>>1;
        if (lt_f < m->cf[mid])
            hi = mid;
        else
            lo = mid;
    }
    return lo;
}


/* update model                                        */
/* m   qsmodel to be updated                           */
/* sym  symbol that occurred (must be <n from init)    */
void qsupdate( qsmodel *m, int sym )
{   if (m->left <= 0)
        dorescale(m);
    m->left--;
    m->newf[sym] += m->incr;
}
