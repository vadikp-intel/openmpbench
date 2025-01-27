/****************************************************************************
*                                                                           *
*             OpenMP MicroBenchmark Suite - Version 3.1                     *
*                                                                           *
*                            produced by                                    *
*                                                                           *
*             Mark Bull, Fiona Reid and Nix Mc Donnell                      *
*                                                                           *
*                                at                                         *
*                                                                           *
*                Edinburgh Parallel Computing Centre                        *
*                                                                           *
*         email: markb@epcc.ed.ac.uk or fiona@epcc.ed.ac.uk                 *
*                                                                           *
*                                                                           *
*      This version copyright (c) The University of Edinburgh, 2015.        *
*                                                                           *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*      http://www.apache.org/licenses/LICENSE-2.0                           *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#include "common.h"
#include "schedbench.h"

int cksz, itersperthr = 8192;
char testName[32];

int main(int argc, char **argv) {

    init(argc, argv);
    // Also print itersperthr
    printf("\t%d iterations per threads\n", itersperthr);
	
	/* GENERATE REFERENCE TIME */
    reference("reference time", &refer);
	
    /* TEST STATIC */
    benchmark("STATIC", &teststatic);

	/* TEST STATIC,n */
    cksz = 1;
    while (cksz <= itersperthr) {
		sprintf(testName, "STATIC %d", cksz);
		benchmark(testName, &teststaticn);
		cksz *= 2;
    }

	/* TEST DYNAMIC,n */
    cksz = 1;
    while (cksz <= itersperthr) {
		sprintf(testName, "DYNAMIC %d", cksz);
		benchmark(testName, &testdynamicn);
		cksz *= 2;
    }

	/* TEST GUIDED,n */
    cksz = 1;
    while (cksz <= itersperthr / nthreads) {
		sprintf(testName, "GUIDED %d", cksz);
		benchmark(testName, &testguidedn);
		cksz *= 2;
    }
	/* GENERATE REFERENCE TIME for sections*/
	reference("sections reference time", &refer_testsections);

	/* TEST SECTIONS,n */
	nthreads = 1;
	while (nthreads <= 10) {
		sprintf(testName, "SECTIONS %d threads", nthreads);
		benchmark(testName, &testsections);
		nthreads++;
	}

	finalise();

    return EXIT_SUCCESS;

}

void refer() {
    int i, j;
    for (j = 0; j < innerreps; j++) {
    for (i = 0; i < itersperthr; i++) {
        delay(delaylength);
    }
    }
}

void teststatic() {

    int i, j;
#pragma omp parallel private(j)
    {
    for (j = 0; j < innerreps; j++) {
#pragma omp for schedule(static)
        for (i = 0; i < itersperthr * nthreads; i++) {
			delay(delaylength);
        }
    }
    }
}

void teststaticn() {
    int i, j;
#pragma omp parallel private(j)
    {
    for (j = 0; j < innerreps; j++) {
#pragma omp for schedule(static,cksz)
        for (i = 0; i < itersperthr * nthreads; i++) {
			delay(delaylength);
        }
    }
    }
}

void testdynamicn() {
    int i, j;
#pragma omp parallel private(j)
    {
    for (j = 0; j < innerreps; j++) {
#pragma omp for schedule(dynamic,cksz)
        for (i = 0; i < itersperthr * nthreads; i++) {
        delay(delaylength);
        }
    }
    }
}

void testguidedn() {
    int i, j;
#pragma omp parallel private(j)
    {
    for (j = 0; j < innerreps; j++) {
#pragma omp for schedule(guided,cksz)
        for (i = 0; i < itersperthr * nthreads; i++) {
        delay(delaylength);
        }
    }
    }
}

void refer_testsections() {
	int i,j;
	{
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr; i++) {
					delay(delaylength);
			}
		}
		}
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr; i++) {
					delay(delaylength * 2);
			}
		}
		}
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr; i++) {
					delay(delaylength * 4);
			}
		}
		}
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr; i++) {
					delay(delaylength * 8);
			}
		}
		}
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr; i++) {
					delay(delaylength * 16);
			}
		}
		}
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr; i++) {
					delay(delaylength * 32);
			}
		}
		}
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr; i++) {
					delay(delaylength * 64);
			}
		}
		}
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr; i++) {
					delay(delaylength * 128);
			}
		}
		}
	}
}

void testsections() {
	int i,j;
#pragma omp parallel sections num_threads(nthreads)
	{
#pragma omp section
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr * nthreads; i++) {
					delay(delaylength);
			}
		}
		}
#pragma omp section
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr * nthreads; i++) {
					delay(delaylength * 2);
			}
		}
		}
#pragma omp section
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr * nthreads; i++) {
					delay(delaylength * 4);
			}
		}
		}
#pragma omp section
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr * nthreads; i++) {
					delay(delaylength * 8);
			}
		}
		}
#pragma omp section
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr * nthreads; i++) {
					delay(delaylength * 16);
			}
		}
		}
#pragma omp section
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr * nthreads; i++) {
					delay(delaylength * 32);
			}
		}
		}
#pragma omp section
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr * nthreads; i++) {
					delay(delaylength * 64);
			}
		}
		}
#pragma omp section
		{
			for (j = 0; j < innerreps; j++) {
				for (i = 0; i < itersperthr * nthreads; i++) {
					delay(delaylength * 128);
			}
		}
		}
	}
}
