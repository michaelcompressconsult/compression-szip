/*  bitmodel.h     headerfile for bit indexed trees probability model
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
*
* based on: Peter Fenwick: A New Data Structure for Cumulative Probability Tables
* Technical Report 88, Dep. of Computer Science, University of Auckland, NZ
*
*
* Bitmodel implements bit indexed trees for frequency storage described
* by Peter Fenwick: A New Data Structure for Cumulative Probability Tables
* Technical Report 88, Dep. of Computer Science, University of Auckland, NZ.
* It features a fast method for cumulative frequency storage and updating.
* The difference to the fenwick paper is the way the table is recalculated
* after rescaling; the method here is faster.
*
* There is a compiletime switch; if EXCLUDEONUPDATE is defined symbols
* are excluded on update; to be able to use them again you have to call
* the include function for that symbol.
*
* The module provides functions for creation, reset, deletion, query for
* probabilities, queries for symbols, reenabling symbols and model updating.
*/
#ifndef BITMODEL_H
#define BITMODEL_H


#include "port.h"

#define EXCLUDEONUPDATE

typedef struct {
    int n,             /* number of symbols */
        totalfreq,     /* total frequency count (without excluded symbols) */
        max_totf,      /* maximum allowed total frequency count */
        incr,          /* increment per update */
        mask;          /* initial bitmask used for search */
    uint2 *f,          /* frequency for the symbol; first bit set if excluded */
        *cf;           /* array of cumulative frequencies */
} bitmodel;

/* initialisation of bitmodel                          */
/* m   bitmodel to be initialized                      */
/* n   number of symbols in that model                 */
/* max_totf  maximum allowed total frequency count     */
/* rescale  desired rescaling interval, must be <max_totf/2 */
/* init  array of int's to be used for initialisation (NULL ok) */
void initbitmodel( bitmodel *m, int n, int max_totf, int rescale,
   int *init );

/* reinitialisation of bitmodel                        */
/* m   bitmodel to be initialized                      */
/* init  array of int's to be used for initialisation (NULL ok) */
void resetbitmodel( bitmodel *m, int *init);


/* deletion of bitmodel m                              */
void deletebitmodel( bitmodel *m );


/* retrieval of estimated frequencies for a symbol     */
/* m   bitmodel to be questioned                       */
/* sym  symbol for which data is desired; must be <n   */
/* sy_f frequency of that symbol                       */
/* lt_f frequency of all smaller symbols together      */
/* the total frequency can be obtained with bit_totf   */
void bitgetfreq( bitmodel *m, int sym, int *sy_f, int *lt_f);

/* find out total frequency for a bitmodel             */
/* m   bitmodel to be questioned                       */
#define bittotf(m) ((m)->totalfreq)

/* find out symbol for a given cumulative frequency    */
/* m   bitmodel to be questioned                       */
/* lt_f  cumulative frequency                          */
int bitgetsym( bitmodel *m, int lt_f );


/* update model                                        */
/* m   bitmodel to be updated                          */
/* sym  symbol that occurred (must be <n from init)    */
void bitupdate( bitmodel *m, int sym );


#ifdef EXCLUDEONUPDATE
/* update model and exclude symbol                     */
/* m   bitmodel to be updated                          */
/* sym  symbol that occurred (must be <n from init)    */
void bitupdate_ex( bitmodel *m, int sym );


/* deactivate symbol                                   */
/* m   bitmodel to be updated                          */
/* sym  symbol to be deactivated                       */
void bitdeactivate( bitmodel *m, int sym );

/* reactivate symbol                                   */
/* m   bitmodel to be updated                          */
/* sym  symbol to be reactivated                       */
void bitreactivate( bitmodel *m, int sym );
#endif

#endif
