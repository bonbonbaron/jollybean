#include "db.h"

int main(int argc, char **argv) {
  Database *dbP = NULL;
  Error e = dbGet(&dbP, "test", argc, 1);

  EntryData emptyData = {0};

  char hoopla[32];
  if (!e) {
    for (int i = 0; i < 1000; ++i) {
      sprintf(hoopla,  "hoopla%03d", i);
      dbAddEntry(dbP, hoopla, &emptyData, 1);
    }
  }

  return 0;
}
