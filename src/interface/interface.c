#include "interface.h"
#include "SDL_events.h"

/**********************
 * Rendering
 *********************/

#ifdef MULTITHREADED_

// =======================================
// Multithreading
// =======================================
static void _threadFuncArgArrayIni(CriticalFunc funcP, ThreadFuncArg *argA, U32 *nThreadsNeededP, void *_array) {
  if (argA && _array) {
    U32 nElemsToProcess = arrayGetNElems(_array);
    if (nElemsToProcess < *nThreadsNeededP) {
      *nThreadsNeededP = nElemsToProcess;
    }
    // Tell each thread where to start in array and how many items to process
    const U32 nElemsPerThread = ((nElemsToProcess + (*nThreadsNeededP >> 1)) / *nThreadsNeededP);
    for (U32 i = 0; i < *nThreadsNeededP; ++i) {
      argA[i].startIdx = i * nElemsPerThread;
      argA[i].nElemsToProcess = nElemsPerThread;
      argA[i].funcP = funcP;
      argA[i].array = _array;
    }
    // Last thread may have a different number of elems to process then the rest
    // (This might be faster than modulo.)
    argA[*nThreadsNeededP - 1].nElemsToProcess = nElemsToProcess - (nElemsPerThread * (*nThreadsNeededP - 1));
  }
}

// Generic multithreading function
static void* _mtGenericLoop(ThreadFuncArg *thargP) {
  const U32 ptrIncr = arrayGetElemSz(thargP->array);
  U8 *voidP = (U8*) thargP->array + ptrIncr * thargP->startIdx;
  U8 *voidEndP = voidP + thargP->nElemsToProcess;
  for (Error e = SUCCESS; !e && voidP < voidEndP; voidP += ptrIncr) {
    e = thargP->funcP((void*) *((U32*) voidP));  // ugly but... how else to generalize?
  }
  return NULL;
}

// Multithreading entry point
Error multiThread( CriticalFunc funcP, void *_array) {
  if (!_array || !funcP) {
    return E_BAD_ARGS;
  }

  Thread threadA[N_CORES];
  ThreadFuncArg *thArgA = NULL;
  Error e = arrayNew((void**) &thArgA, sizeof(ThreadFuncArg), N_CORES);

  if (!e) {
    U32 nThreadsNeeded = N_CORES;
    // nThreadsNeeded gets updated to fewer than N_CORES if fewer elements than cores exist.
    _threadFuncArgArrayIni(funcP, thArgA, &nThreadsNeeded, _array);

    for (int i = 0; i < nThreadsNeeded; ++i) {
      threadIni_(&threadA[i], &thArgA[i]);
    }

    for (int i = 0; i < nThreadsNeeded; ++i) {
      threadJoin_(threadA[i]);
    }
  }

  arrayDel((void**) &thArgA);

  return e;
}

#endif //#ifdef MULTITHREADED_
