#include "data/map.h"
#include "data/share.h"

static Map* _sharedMapOfMapsP = NULL;
static Map* _sharedPointerMapP = NULL;
// May add more types later

void shareIni( ) {
  _sharedMapOfMapsP = mapNew( MAP_POINTER, sizeof(Map*), KEY_MAX, GENERAL );
  _sharedPointerMapP = mapNew( NONMAP_POINTER, sizeof(Map*), KEY_MAX, GENERAL );
}

void shareAddMap( const Key KEY, MapElemType mapElemType, const U32 elemSz, const Key nElems, const PoolId poolId ) {
  assert( _sharedMapOfMapsP );
  if ( poolId == PERMANENT && mapHasKey( _sharedMapOfMapsP, KEY ) ) {
    return;
  }
  Map *mP = mapNew( mapElemType, elemSz, nElems, poolId );
  mapSet( _sharedMapOfMapsP, KEY, (void*) &mP );
}

Map* shareGetMap( const Key KEY ) {
  return (Map*) mapGetNestedMapP(_sharedMapOfMapsP, KEY);
}

Map* shareGetPointer( const Key KEY ) {
#ifndef NDEBUG
  void** rawPP = (void**) mapGet(_sharedPointerMapP, KEY);
  assert( rawPP );
  assert( *rawPP );
  return *rawPP;
#else
  return mapGet(_sharedPointerMapP, KEY);
#endif
}

