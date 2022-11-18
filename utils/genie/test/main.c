#include "genie.h"

int main(int argc, char **argv) {
  Directory *dirP = NULL;
  Error e = dirGet(&dirP, "test", argc, 0);

  if (!e) {
    EntryData emptyData = {0};

    char hoopla[32];
    e = genieListen(DIR_FILE);
  }

  return e;
}
