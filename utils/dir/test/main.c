#include "dir.h"

int main(int argc, char **argv) {
  Directory *dirP = NULL;
  Error e = dirGet(&dirP, "test", argc, 1);

  EntryData emptyData = {0};

  char hoopla[32];
  if (!e) {
    for (int i = 0; i < 1000; ++i) {
      sprintf(hoopla,  "hoopla%03d", i);
      dirAddEntry(dirP, hoopla, &emptyData, 1);
    }
  }

  // dirList(dirP);  // broekn, i moved this ove rto genie

  return 0;
}
