#ifndef SHARE_H
#define SHARE_H
#include "data/mail.h"
#include "x/x.h"
#include "jb.h"

void shareIni( const Key N_SYSTEM_TYPES );

Map*     shareGetMap( const Key KEY );

Map*     shareGetPointer( const Key KEY );

Message* shareGetInbox( const SystemId KEY );
Message* shareSetInbox( const SystemId SYSTEM_ID, const Key N_SLOTS );

#if 0  // TODO determine whether these are even needed
System*  shareGetSystemFromType( const Type TYPE );
void     shareSetSystemFromType( const Type TYPE, const System* sysP );
#endif

System*  shareGetSystem( const SystemId KEY );
void     shareSetSystem( const System* sysP );  // no need to pass key; use its id field

#endif // SHARE_H
