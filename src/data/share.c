#include "data/map.h"
#include "data/mail.h"

#define MAX_NUM_KEYS_ ( ( sizeof(Key) << 8 ) - 1 )

static Map* _sharedMemMapOfMapsP = NULL;
static Map* _sharedMemRawPointerMapP = NULL;
static Map* _sharedMemInboxMapP = NULL;
// May add more types later

void shareIni( const Key N_SYSTEM_TYPES ) {
  _sharedMemMapOfMapsP = mapNew( MAP_POINTER, sizeof(Map*), MAX_NUM_KEYS_, GENERAL );
  _sharedMemRawPointerMapP = mapNew( NONMAP_POINTER, sizeof(Map*), MAX_NUM_KEYS_, GENERAL );
  _sharedMemInboxMapP = mapNew( NONMAP_POINTER, sizeof(Map*), N_SYSTEM_TYPES, GENERAL );
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

// void shareSetMapOfMapsElem()
