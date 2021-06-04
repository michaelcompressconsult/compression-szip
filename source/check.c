/* check.c - checks stdin for equality with a given file,
* reports filename to logfile if not equal
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
*/

#include <stdio.h>
#include <stdlib.h>
#ifndef unix
#include <io.h>
#include <fcntl.h>
#endif

void usage()
{	fprintf(stderr,"check <otherfile> <logfile>\n");
	exit(2);
}


void bugfound(char *filename, char *logfilename)
{   FILE *logfil;
    logfil = fopen( logfilename, "ab" );
    fprintf(logfil,"%s\n",filename);
    fclose(logfil);
    exit(1);
}

int main( int argc, char *argv[] )
{   FILE *infil;
    int ch;
    if (argc!=3) usage();

    infil = fopen( argv[1], "rb" );
    if (infil==NULL) bugfound(argv[1],argv[2]);


#ifndef unix
    setmode( fileno( infil ), O_BINARY );
    setmode( fileno( stdin ), O_BINARY );
#endif

    while ((ch=getchar()) != EOF)
        if (getc(infil) != ch)
        {   fclose(infil);
            bugfound(argv[1],argv[2]);
        }
    if (getc(infil) != EOF)
        {   fclose(infil);
            bugfound(argv[1],argv[2]);
        }
    return 0;
}
