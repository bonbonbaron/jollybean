#ifndef SHARE_H
#define SHARE_H
#include "data/mail.h"

void shareIni( const Key N_SYSTEM_TYPES );
Message* shareNewInbox( const Key SYSTEM_ID, const Key N_SLOTS );
Message* shareGetInbox( const Key KEY );
Map*     shareGetMap( const Key KEY );

#endif // SHARE_H
