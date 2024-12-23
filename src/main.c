#include "data/map.h"

int main(int argc, char **argv) {
  Map *mP = NULL;
  Error e = mapNew(&mP, RAW_DATA, sizeof(int), 5);
  if (!e) {
    for (int i = 1; !e && i <= arrayGetNElems(mP->mapA); ++i) {
      int hey = i * 3;
      e = mapSet(mP, i, &hey);
    }
  }

  //mapDel(&mP);

  return e;
}
