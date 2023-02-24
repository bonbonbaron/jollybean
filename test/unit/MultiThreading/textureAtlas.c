#include "xRender.h"
#include "blehColormap.h"
#include "heckColormap.h"
#include "redColormap.h"
#include <pthread.h>

/* Coding Kaizen:
 *  1) Code only a little bit at a time then build to make sure it compiles.
 *  2) Test every individual function you write.
 */

typedef pthread_t Thread;

#define N_CORES (4)
#define threadIni_(threadP_, funcP_, arr_) pthread_create(threadP_, NULL, funcP_, arr_)
#define threadJoin_(threadP_) pthread_join(threadP_, NULL)

typedef void* (*ThreadFunc)(void*);

typedef struct {
  U32 startIdx;
  U32 nElemsToProcess;
} ThreadFuncArg;


void* doIt(void *voidP) {
  printf("HIII\n");
}

/* Here's where im taking the design...
 *
 * 1) functions meant to be passed to threads will have their declarations macroed.
 * 2) single function for multithreading will call (1)
 * 3) if no multithreading, 
 *    a) (2) will be replaced with just (1)
 *    b) (1) will have to handle input without additional startIdx and nElemsToProcess fields
 * 4) abstract away any concept of pthread behind interface.c to allow other threading libraries.
 */

static void _threadFuncArgArrayIni(ThreadFuncArg *argA, U32 *nThreadsNeededP, void *_array) {
  if (argA && _array) {
    U32 nElemsToProcess = arrayGetNElems(_array);
    U32 nThreads = arrayGetNElems(argA);
    if (nElemsToProcess < nThreads) {
      nThreads = nElemsToProcess;
    }

    // Tell each thread where to start in array and how many items to process
    const U32 nElemsPerThread = ((nElemsToProcess + (nThreads >> 1)) / nThreads);
    for (U32 i = 0; i < nThreads; ++i) {
      argA[i].startIdx = i * nElemsPerThread;
      argA[i].nElemsToProcess = nElemsPerThread;
    }
    // Last thread may have a different number of elems to process then the rest
    // (This might be faster than modulo.)
    argA[nThreads - 1].nElemsToProcess = nElemsToProcess - (nElemsPerThread * (nThreads - 1));
  }
}


Error multiThread( ThreadFunc funcP, void *_array) {
  if (!_array || !funcP) {
    return E_BAD_ARGS;
  }

  Thread threadA[N_CORES];
  ThreadFuncArg *thArgA = NULL;
  Error e = arrayNew((void**) &thArgA, sizeof(ThreadFuncArg), N_CORES);

  if (!e) {
    _threadFuncArgArrayIni(thArgA, _array);

    for (int i = 0; i < N_CORES; ++i) {
      threadIni_(&threadA[i], doIt, &thArgA[i]);
    }

    for (int i = 0; i < N_CORES; ++i) {
      threadJoin_(threadA[i]);
    }
  }

  arrayDel((void**) &thArgA);

  return e;
}



int main(int argc, char **argv) {
  Colormap **colormapPA = NULL;

  Error e = arrayNew((void**) &colormapPA, sizeof(void*), 3);

  if (!e) {
    colormapPA[0] = &blehColormap;
    colormapPA[1] = &heckColormap;
    colormapPA[2] = &redColormap;

    e = multiThread(doIt, colormapPA);
  }

  arrayDel((void**) &colormapPA);

  return e;
}
