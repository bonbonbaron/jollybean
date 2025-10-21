#include "data/map.h"
#include "data/mail.h"
#include "x/x.h"
#include "jb.h"
#include "share.h"

#define MAX_NUM_KEYS_ ( ( sizeof(Key) << 8 ) - 1 )

static Map* _sharedMapOfMapsP = NULL;
static Map* _sharedRawPointerMapP = NULL;
// Permanent maps
static Map* _sharedSystemMapP = NULL;
static Map* _sharedInboxMapP = NULL;
// May add more types later

void shareIni( const Key N_SYSTEM_TYPES ) {
  _sharedMapOfMapsP = mapNew( MAP_POINTER, sizeof(Map*), MAX_NUM_KEYS_, GENERAL );
  _sharedRawPointerMapP = mapNew( NONMAP_POINTER, sizeof(Map*), MAX_NUM_KEYS_, GENERAL );
  if ( ! _sharedSystemMapP ) {
    _sharedSystemMapP = mapNew( NONMAP_POINTER, sizeof(System*), N_SYSTEM_TYPES, PERMANENT );
  }
  if ( ! _sharedInboxMapP ) {
    _sharedInboxMapP = mapNew( NONMAP_POINTER, sizeof(Map*), N_SYSTEM_TYPES, PERMANENT );
  }
}

Message* shareSetInbox( const SystemId SYSTEM_ID, const Key N_SLOTS ) {
  assert( _sharedInboxMapP );
  Message* inboxP = mailboxNew( N_SLOTS, GENERAL );
  mapSet( _sharedInboxMapP, SYSTEM_ID, (void*) &inboxP );
  return inboxP;
}

Message* shareGetInbox( const SystemId SYSTEM_ID ) {
#ifndef NDEBUG
  Message** mailboxPP = (Message**) mapGet( _sharedInboxMapP, SYSTEM_ID );
  assert( mailboxPP && *mailboxPP );
  return *mailboxPP;
#else
  return *( (Message**) mapGet( _sharedInboxMapP, SYSTEM_ID );
#endif
}

Map* shareGetMap( const Key KEY ) {
#ifndef NDEBUG
  Map* nestedMapP = mapGetNestedMapP(_sharedMapOfMapsP, KEY);
  assert( nestedMapP );
  return nestedMapP;
#else
  return mapGetNestedMapP(_sharedMapOfMapsP, KEY);
#endif
}

Map* shareGetPointer( const Key KEY ) {
#ifndef NDEBUG
  void** rawPP = (void**) mapGet(_sharedRawPointerMapP, KEY);
  assert( rawPP );
  assert( *rawPP );
  return *rawPP;
#else
  return mapGet(_sharedRawPointerMapP, KEY);
#endif
}

void shareSetSystem( const System* sysP ) {
  assert( sysP );
  assert( sysP->id );
  mapSet(_sharedSystemMapP, sysP->id, &sysP );
}

System* shareGetSystem( const SystemId SYSTEM_ID ) {
#ifndef NDEBUG
  void** sysPP = (void**) mapGet(_sharedSystemMapP, SYSTEM_ID);
  assert( sysPP );
  assert( *sysPP );
  return *sysPP;
#else
  return mapGetNestedMapP(_sharedSystemMapP, SYSTEM_ID);
#endif
}

