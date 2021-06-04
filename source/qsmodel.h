/* qsmodel.h     headerfile for quasistatic probability model
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
#ifndef QSMODEL_H
#define QSMODEL_H

#include "port.h"

typedef struct {
    int n,             /* number of symbols */
        left,          /* symbols to next rescale */
        nextleft,      /* symbols with other increment */
        rescale,       /* intervals between rescales */
        targetrescale, /* should be interval between rescales */
        incr,          /* increment per update */
        searchshift;   /* shift for lt_freq before using as index */
    uint2 *cf,         /* array of cumulative frequencies */
        *newf,         /* array for collecting ststistics */
        *search;       /* structure for searching on decompression */
} qsmodel;

/* initialisation of qsmodel                           */
/* m   qsmodel to be initialized                       */
/* n   number of symbols in that model                 */
/* lg_totf  base2 log of total frequency count         */
/* rescale  desired rescaling interval, should be < 1<<(lg_totf+1) */
/* init  array of int's to be used for initialisation (NULL ok) */
/* compress  set to 1 on compression, 0 on decompression */
void initqsmodel( qsmodel *m, int n, int lg_totf, int rescale,
   int *init, int compress );

/* reinitialisation of qsmodel                         */
/* m   qsmodel to be initialized                       */
/* init  array of int's to be used for initialisation (NULL ok) */
void resetqsmodel( qsmodel *m, int *init);


/* deletion of qsmodel m                               */
void deleteqsmodel( qsmodel *m );


/* retrieval of estimated frequencies for a symbol     */
/* m   qsmodel to be questioned                        */
/* sym  symbol for which data is desired; must be <n   */
/* sy_f frequency of that symbol                       */
/* lt_f frequency of all smaller symbols together      */
/* the total frequency is 1<<lg_totf                   */
void qsgetfreq( qsmodel *m, int sym, int *sy_f, int *lt_f );


/* find out symbol for a given cumulative frequency    */
/* m   qsmodel to be questioned                        */
/* lt_f  cumulative frequency                          */
int qsgetsym( qsmodel *m, int lt_f );


/* update model                                        */
/* m   qsmodel to be updated                           */
/* sym  symbol that occurred (must be <n from init)    */
void qsupdate( qsmodel *m, int sym );

#endif
