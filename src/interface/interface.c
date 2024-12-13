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

// Generic multithreading function (which gets called in threadIni_ macro)
static void* _mtGenericLoop(ThreadFuncArg *thargP) {
  const size_t ptrIncr = arrayGetElemSz(thargP->array);
  U8 *voidP = (U8*) thargP->array + ptrIncr * thargP->startIdx;
  U8 *voidEndP = voidP + thargP->nElemsToProcess;
  /* ******** NOTE *********
   * Because of the below design,
   * the input to the thargP->funcP *must* be a pointer.
   * That is, every element of _array in multiThread() must be a pointer.
   */
  for (; voidP < voidEndP; voidP += ptrIncr) {
    thargP->funcP((void*) *((size_t*) voidP));  // ugly but... how else to generalize?
  }
  return NULL;
}

// Multithreading entry point
void multiThread( CriticalFunc funcP, void *_array) {
  assert (_array && funcP);

  Thread threadA[N_CORES];
  ThreadFuncArg *thArgA = arrayNew( sizeof(ThreadFuncArg), N_CORES);

  U32 nThreadsNeeded = N_CORES;
  // nThreadsNeeded gets updated to fewer than N_CORES if fewer elements than cores exist.
  _threadFuncArgArrayIni(funcP, thArgA, &nThreadsNeeded, _array);

  for (int i = 0; i < nThreadsNeeded; ++i) {
    threadIni_(&threadA[i], &thArgA[i]);
  }

  for (int i = 0; i < nThreadsNeeded; ++i) {
    threadJoin_(threadA[i]);
  }

  arrayDel((void**) &thArgA);
}

#endif //#ifdef MULTITHREADED_
