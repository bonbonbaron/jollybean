#include "data.h"
#include "genie.h"

int main(int argc, char **argv) {
  Directory *dirP = NULL;
  Error e = dirGet(&dirP, "test", 2000, 0);

  if (!e) {
    EntryData emptyData = {0};

    // Populate the directory with dummy data; we're only searching for names anyway.
    for (int i = 0; i < 1000; ++i) {
      char hoopla[32] = {'a'};
      hoopla[31] = '\0';  // null-terminate the name
      hoopla[1] = 'A' + (i & 15);
      memset((void*) &hoopla[2], 'A' + (i & 15), 25);
      dirAddEntry(dirP, hoopla, &emptyData, 0);
    }
    char *response = genieAsk("Which hoopla do you want?", DIR_FILE, (List*) dirP, 0);

    printf("you gave me the hoopla %s\nsupposedly %d chars long.\n", response, arrayGetNElems(response));
    arrayDel((void**) &response);
  }

  return e;
}
