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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#include "common.h"
#include "windows.h"

#define CONF95 1.96

int nthreads = -1;           // Number of OpenMP threads
int delaylength = -1;        // The number of iterations to delay for
int outerreps = -1;          // Outer repetitions
int innerreps = -1;          // Inner repetitions
double delaytime = -1.0;     // Length of time to delay for in microseconds
double targettesttime = 0.0; // The length of time in microseconds that the test
                             // should run for.
double *times;               // Array of doubles storing the benchmark times in microseconds
double referencetime;        // The average reference time in microseconds to perform
                             // outerreps runs
double referencesd;          // The standard deviation in the reference time in
                             // microseconds for outerreps runs.
double testtime;             // The average test time in microseconds for
                             // outerreps runs
double testsd;               // The standard deviation in the test time in
                             // microseconds for outerreps runs.
int verbose = 0;             // verbosity of output
int fixeddelay = 0;          // We fix the delay and inneriterations if set to 1.

void usage(char *argv[]) {
    printf("Usage: %s.x \n"
       "\t--outer-repetitions <outer-repetitions> (default %d)\n"
	   "\t--inner-repetitions <inner-repetitions> (No default. if set, we fix inner repetitions and delaylength as delay time)\n"
       "\t--test-time <target-test-time> (default %0.2f microseconds)\n"
       "\t--delay-time <delay-time> (default %0.4f microseconds)\n"
       "\t--verbose\n",
        argv[0],
       DEFAULT_OUTER_REPS, DEFAULT_TEST_TARGET_TIME, DEFAULT_DELAY_TIME);
}

void parse_args(int argc, char *argv[]) {
    // Parse the parameters
    int arg;
    for (arg = 1; arg < argc; arg++) {
    if (strcmp(argv[arg], "--delay-time") == 0.0) {
        delaytime = atof(argv[++arg]);
        if (delaytime == 0.0) {
        printf("Invalid float:--delay-time: %s\n", argv[arg]);
        usage(argv);
        exit(EXIT_FAILURE);
        }
        
    } else if (strcmp(argv[arg], "--outer-repetitions") == 0) {
        outerreps = atoi(argv[++arg]);
        if (outerreps == 0) {
        printf("Invalid integer:--outer-repetitions: %s\n", argv[arg]);
        usage(argv);
        exit(EXIT_FAILURE);
        }
        
    } else if (strcmp(argv[arg], "--inner-repetitions") == 0) {
        innerreps = atoi(argv[++arg]);
        if (innerreps == 0) {
        printf("Invalid integer:--inner-repetitions: %s\n", argv[arg]);
        usage(argv);
        exit(EXIT_FAILURE);
        }
        fixeddelay = 1;
    } else if (strcmp(argv[arg], "--test-time") == 0) {
        targettesttime = atof(argv[++arg]);
        if (targettesttime == 0) {
        printf("Invalid float:--test-time: %s\n", argv[arg]);
        usage(argv);
        exit(EXIT_FAILURE);
        }
        
    } else if (strcmp(argv[arg], "-h") == 0) {
        usage(argv);
        exit(EXIT_SUCCESS);
        
    } else if (strcmp(argv[arg], "--verbose") == 0) {
        verbose = 1;
    } else {
        printf("Invalid parameters: %s\n", argv[arg]);
        usage(argv);
        exit(EXIT_FAILURE);
    }
    }
}

int getdelaylengthfromtime(double delaytime) {
	if (fixeddelay) {
		// Note: We fix the delay length as delay time.
		// Reduce SD across runs.
		return delaytime;
	}
    int i, reps;
    double lapsedtime, starttime; // seconds

    reps = 1000;
    lapsedtime = 0.0;

    delaytime = delaytime/1.0E6; // convert from microseconds to seconds

    // Note: delaytime is local to this function and thus the conversion
    // does not propagate to the main code. 

    // Here we want to use the delaytime in microseconds to find the 
    // delaylength in iterations. We start with delaylength=0 and 
    // increase until we get a large enough delaytime, return delaylength 
    // in iterations. 

    delaylength = 0;
    delay(delaylength);

    while (lapsedtime < delaytime) {
        delaylength = delaylength * 1.1 + 1;
        starttime = getclock();
        for (i = 0; i < reps; i++) {
            delay(delaylength);
        }
        lapsedtime = (getclock() - starttime) / (double) reps;
    }
    return delaylength;

}

int getinnerreps(void (*test)(void)) {
	if(fixeddelay) {
		// Note: fixed inner repetitions input by user.
		// Reduce SD across runs.
	    return innerreps;
	}
	innerreps = 1L;  // some initial value
    double time = 0.0;

    double start = getclock();
    test();
    time = (getclock() - start) * 1.0e6;

    while (time < targettesttime) {
        start  = getclock();
        test();
        time = (getclock() - start) * 1.0e6;
        innerreps *=2;

        // Test to stop code if compiler is optimising reference time expressions away
        if (innerreps > (targettesttime*1.0e15)) {
            printf("Compiler has optimised reference loop away, STOP! \n");
            printf("Try recompiling with lower optimisation level \n");
            exit(1);
        }
    }
    return innerreps;
}

void printheader(char *name) {
    if (!verbose)
        return;
    printf("\n");
    printf("--------------------------------------------------------\n");
    printf("Computing %s time using %d reps\n", name, innerreps);
}

void stats(double *mtp, double *sdp) {

    double meantime, totaltime, sumsq, mintime, maxtime, sd, cutoff;

    int i, nr;

    mintime = 1.0e10;
    maxtime = 0.;
    totaltime = 0.;

    for (i = 1; i <= outerreps; i++) {
        mintime = min (mintime, times[i]);
        maxtime = max (maxtime, times[i]);
        totaltime += times[i];
    }

    meantime = totaltime / outerreps;
    sumsq = 0;

    for (i = 1; i <= outerreps; i++) {
        sumsq += (times[i] - meantime) * (times[i] - meantime);
    }
    sd = sqrt(sumsq / (outerreps - 1));

    cutoff = 3.0 * sd;

    nr = 0;

    for (i = 1; i <= outerreps; i++) {
    if (fabs(times[i] - meantime) > cutoff)
        nr++;
    }

    if (verbose) {
        printf("\n");
        printf("Sample_size       Average     Min         Max          S.D.          Outliers\n");
        printf(" %d                %f   %f   %f    %f      %d\n",
            outerreps, meantime, mintime, maxtime, sd, nr);
        printf("\n");
    }

    *mtp = meantime;
    *sdp = sd;

}

void printfooter(char *name, double testtime, double testsd,
        double referencetime, double refsd) {
    printf("%s ", name);
    if (verbose)
        printf("time     = %f microseconds +/- %f\n",
          testtime, CONF95*testsd);
    else
        printf("%f\n", testtime);
    if (verbose)
    printf("%s overhead = %f microseconds +/- %f\n",
        name, testtime-referencetime, CONF95*(testsd+referencesd));

}

void printreferencefooter(char *name, double referencetime, double referencesd) {
    printf("%s ", name);
    if (verbose)
        printf("time     = %f microseconds +/- %f\n",
            referencetime, CONF95 * referencesd);
    else
        printf("%f\n",referencetime);
}

void init(int argc, char **argv)
{
#pragma omp parallel
    {
#pragma omp master
    {
        nthreads = omp_get_num_threads();
    }

    }

    parse_args(argc, argv);

    if (outerreps == -1) {
    outerreps = DEFAULT_OUTER_REPS;
    }
    if (targettesttime == 0.0) {
    targettesttime = DEFAULT_TEST_TARGET_TIME;
    }
    if (delaytime == -1.0) {
    delaytime = DEFAULT_DELAY_TIME; 
    }
    // Always need to compute delaylength in iterations 
    delaylength = getdelaylengthfromtime(delaytime);
    
    times = malloc((outerreps+1) * sizeof(double));

    printf("Running OpenMP benchmark version 3.0\n"
        "\t%d thread(s)\n"
        "\t%d outer repetitions\n"
        "\t%d inner repetitions\n"
        "\t%0.2f test time (microseconds)\n"
        "\t%d delay length (iterations) \n"
        "\t%f delay time (microseconds)\n",
        nthreads,
        outerreps, innerreps, targettesttime,
        delaylength, delaytime);
}

void finalise(void) {
    free(times);

}

void initreference(char *name) {
    printheader(name);

}

/* Calculate the reference time. */
void reference(char *name, void (*refer)(void)) {
    int k;
    double start;

    // Calculate the required number of innerreps
    innerreps = getinnerreps(refer);

    initreference(name);

    for (k = 0; k <= outerreps; k++) {
        start = getclock();
        refer();
        times[k] = (getclock() - start) * 1.0e6 / (double) innerreps;
    }

    finalisereference(name);

}

void finalisereference(char *name) {
    stats(&referencetime, &referencesd);
    printreferencefooter(name, referencetime, referencesd);

}

void intitest(char *name) {
    printheader(name);

}

void finalisetest(char *name) {
    stats(&testtime, &testsd);
    printfooter(name, testtime, testsd, referencetime, referencesd);

}

/* Function to run a microbenchmark test*/
void benchmark(char *name, void (*test)(void))
{
    int k;
    double start;
    const int iter = 4; // use 4 iterations for calculating innerreps

    // Calculate the required number of innerreps
    for (k=0; k<4; k++) {
        innerreps = max(innerreps, getinnerreps(test));
    }
    intitest(name);

    for (k=0; k<=outerreps; k++) {
        start = getclock();
        test();
        times[k] = (getclock() - start) * 1.0e6 / (double) innerreps;
    }

    finalisetest(name);

}

// For the Cray compiler on HECToR we need to turn off optimisation 
// for the delay and array_delay functions. Other compilers should
// not be afffected. 
void delay(int delaylength) {

    int i;
    float a = 0.;

    for (i = 0; i < delaylength; i++)
    a += i;
    if (a < 0)
    printf("%f \n", a);

}

void array_delay(int delaylength, double a[1]) {

    int i;
    a[0] = 1.0;
    for (i = 0; i < delaylength; i++)
    a[0] += i;
    if (a[0] < 0)
    printf("%f \n", a[0]);

}

#ifdef _MSC_VER 
// Linux artifacts implementation for Windows
#define CLOCK_MONOTONIC 0
int clock_gettime(int X, struct timespec *spec)
{
    __int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
    wintime -= 116444736000000000i64;                //1jan1601 to 1jan1970
    spec->tv_sec = wintime / 10000000i64;           //seconds
    spec->tv_nsec = wintime % 10000000i64 * 100;    //nano-seconds
    return 0;
}
#endif

// Linux-specific, but much more precise
double getclock() {
    struct timespec nowtime;
    clock_gettime(CLOCK_MONOTONIC, &nowtime);
    return nowtime.tv_sec + 1.0e-9 * nowtime.tv_nsec;
}

int returnfalse() {
    return 0;

}

