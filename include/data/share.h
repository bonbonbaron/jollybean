#ifndef SHARE_H
#define SHARE_H
#include "data/mail.h"
#include "x/x.h"
#include "jb.h"

void     shareIni();

void     shareAddMap( const Key KEY, MapElemType mapElemType, const U32 elemSz, const Key nElems, const PoolId poolId );
Map*     shareGetMapVal( const Key mapId, const Key valKey );  // maybe make a setter later if needed
                                        //
Map*     shareNewPointer( const Key KEY );  // TODO correct the parameters
Map*     shareGetPointer( const Key KEY );  // maybe make a setter later if needed
Map*     shareGetMap( const Key KEY );

#endif // SHARE_H
