#include "data/map.h"
#include "data/mail.h"
#include "x/x.h"

#define MAX_NUM_KEYS_ ( ( sizeof(Key) << 8 ) - 1 )

static Map* _sharedMemMapOfMapsP = NULL;
static Map* _sharedMemRawPointerMapP = NULL;
static Map* _sharedMemInboxMapP = NULL;
static Map* _sharedSystemMapP = NULL;
// May add more types later

void shareIni( const Key N_SYSTEM_TYPES ) {
  _sharedMemMapOfMapsP = mapNew( MAP_POINTER, sizeof(Map*), MAX_NUM_KEYS_, GENERAL );
  _sharedMemRawPointerMapP = mapNew( NONMAP_POINTER, sizeof(Map*), MAX_NUM_KEYS_, GENERAL );
  _sharedMemInboxMapP = mapNew( NONMAP_POINTER, sizeof(Map*), N_SYSTEM_TYPES, GENERAL );
  _sharedSystemMapP = mapNew( NONMAP_POINTER, sizeof(System*), N_SYSTEM_TYPES, GENERAL );
}

Message* shareNewInbox( const Key SYSTEM_ID, const Key N_SLOTS ) {
  assert( _sharedMemInboxMapP );
  Message* inboxP = mailboxNew( N_SLOTS, GENERAL );
  mapSet( _sharedMemInboxMapP, SYSTEM_ID, (void*) &inboxP );
  return inboxP;
}

Message* shareGetInbox( const Key KEY ) {
#ifndef NDEBUG
  Message** mailboxPP = (Message**) mapGet( _sharedMemInboxMapP, KEY );
  assert( mailboxPP && *mailboxPP );
  return *mailboxPP;
#else
  return *( (Message**) mapGet( _sharedMemInboxMapP, KEY );
#endif
}

Map* shareGetMap( const Key KEY ) {
#ifndef NDEBUG
  Map* nestedMapP = mapGetNestedMapP(_sharedMemMapOfMapsP, KEY);
  assert( nestedMapP );
  return nestedMapP;
#else
  return mapGetNestedMapP(_sharedMemMapOfMapsP, KEY);
#endif
}

Map* shareGetPointer( const Key KEY ) {
#ifndef NDEBUG
  void** rawPP = (void**) mapGet(_sharedMemRawPointerMapP, KEY);
  assert( rawPP );
  assert( *rawPP );
  return *rawPP;
#else
  return mapGet(_sharedMemRawPointerMapP, KEY);
#endif
}

System* shareGetSystem( const Key KEY ) {
#ifndef NDEBUG
  void** sysPP = (void**) mapGet(_sharedMemRawPointerMapP, KEY);
  assert( sysPP );
  assert( *sysPP );
  return *sysPP;
#else
  return mapGetNestedMapP(_sharedMemRawPointerMapP, KEY);
#endif
}

// void shareSetMapOfMapsElem()
