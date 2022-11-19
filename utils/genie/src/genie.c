#include "genie.h"


inline static char _upperChar(char c) {
  if (c >= 'a' && c <= 'z') {
    c -= 32;
  }
  return c;
}

inline static char _purifyChar(char c) {
  if (!((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9'))) {
    return '_';
  }
  return c;
}

static void _upper(char *str) {
  for (char *cP = str; *cP; ++cP) {
    *cP = _upperChar(*cP);
  }
}

static void _purify(char *str) {
  for (char *cP = str; *cP; ++cP) {
    *cP = _purifyChar(*cP);
  }
}


static char _getch(void) {
	int ch;
	struct termios oldTerminalSettings;
	struct termios newTerminalSettings;

	tcgetattr(STDIN_FILENO, &oldTerminalSettings); /*store old settings */
	newTerminalSettings = oldTerminalSettings; /* copy old settings to new settings */
	newTerminalSettings.c_lflag &= ~(ICANON | ECHO); /* make one change to old settings in new settings */
	tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalSettings); /*apply the new settings immediatly */
	ch = getchar(); /* standard getchar call */
	tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminalSettings); /*reapply the old settings */

	return ch; /*return received char */
}

void genieListDir(Directory *dirP) {
	struct winsize terminalWidth;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminalWidth);

	//ioctl(1, TIOCGWINSZ, &terminalWidth);

	// Get the longest entry's length so we can fit as many in the terminal as possible.
	int maxLen = 0;
	for (int i = 0; i < dirP->nEntries; ++i) {
		int currEntryLen = strlen(dirP->entryA[i].name);
		if (maxLen < currEntryLen) {
			maxLen = currEntryLen;
		}
	}

	if (!maxLen)
		return;	// Either directory has no entries or all its entries are empty!

	maxLen += 4;	 // minimal whitespace padding between entries in terminal

	U32 nEntriesPerLine = terminalWidth.ws_col / maxLen;
	//
	// At last one entry per line even if it won't fit for retardedly small terminals.
	if (!nEntriesPerLine)
		nEntriesPerLine = 1; 

	char leftAlignment[maxLen + 1];	// +1 for '\0'
	sprintf(leftAlignment, "%%-%ds", maxLen);

	for (int i = 0; i < dirP->nEntries; ) {
		for (int j = 0; i < dirP->nEntries && j < nEntriesPerLine; ++j, ++i) {
			// Don't print empty entries, even though we should prevent those.
			if (dirP->entryA[i].name[0]) {
				printf(leftAlignment, dirP->entryA[i].name);
			}
		}
		putchar('\n');
	}
}

static void _genieListSuggestions(Directory *dirP, NameNode *startNodeP, U8 verbose) {
	struct winsize terminalWidth;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminalWidth);

	int maxLen = 0;
	// Get the longest entry's length so we can fit as many in the terminal as possible.
	for (NameNode *nodeP = startNodeP; nodeP != NULL; nodeP = nodeP->nextP) {
		int currEntryLen = strlen(dirP->entryA[nodeP->entryIdx].name);
		if (maxLen < currEntryLen) {
			maxLen = currEntryLen;
		}
	}
  
  if (verbose) {
    printf("max len in matches: %d\n", maxLen);
  }

	if (!maxLen)
		return;	// Either directory has no entries or all its entries are empty!

	maxLen += 4;	 // minimal whitespace padding between entries in terminal

	U32 nEntriesPerLine = terminalWidth.ws_col / maxLen;

	// At last one entry per line even if it won't fit for retardedly small terminals.
	if (!nEntriesPerLine) {
		nEntriesPerLine = 1; 
  }

  const char LEFT_ALIGNED_TBD_BYTES[] = "%%-%ds";
	char leftAlignment[strlen(LEFT_ALIGNED_TBD_BYTES) + 16];	// +1 for '\0'
	sprintf(leftAlignment, LEFT_ALIGNED_TBD_BYTES, maxLen);

	for (NameNode *nodeP = startNodeP; nodeP != NULL;) {
		for (int j = 0; nodeP != NULL && j < nEntriesPerLine; ++j, nodeP = nodeP->nextP) {
			// Don't print empty entries, even though we should prevent those.
			if (dirP->entryA[nodeP->entryIdx].name[0]) {
				printf(leftAlignment, dirP->entryA[nodeP->entryIdx].name);
			}
		}
		putchar('\n');
	}
  // couple newlines before the next input prompt
  putchar('\n');
  putchar('\n');
}

// Although it would be proper to use unsigned chars so I could use the right ASCII codes,
// let's be honest: Who wants to write "unsigned" over and over again?
#define CTRL_U (21)
#define CTRL_W (23)
#define BACKSPACE (127)

#define MAX_REPONSE_SZ_ (NAME_LEN_)
#define CLEAR_INPUT_PROMPT_ "\33[2K\r"
// NOTE: Callers MUST arrayDel returned pointers!
// TODO support cursor movement keys (including ^B, ^F, altB, and altF)
static char* _genieListen(ListType listType, List *listP, U8 verbose) {
  char newChar[2] = {0};
  char *responseP;
  if (arrayNew((void**) &responseP, sizeof(char), MAX_REPONSE_SZ_))
    return NULL;

	for (;;) {
    newChar[0] = _getch();
    switch(newChar[0]) {
      // Tab = Show suggestions (or autocomplete if there's only one)
      case '\t':
        if (strlen(responseP) == 0) {
          continue;
        }
        // List suggests or autocomplete according to appropriate filetype.
        if (listType == TEXT_FILE) {
          continue;	// do autocomplete text version here
        }
        else if (listType == DIR_FILE) {
          NameNode *nameNodeP = dirFindNamesStartingWith(&listP->dir, responseP, verbose);
          if (nameNodeP != NULL) {
            putchar('\n');
            putchar('\n');
            _genieListSuggestions(&listP->dir, nameNodeP, verbose);
            nameNodeDel(&nameNodeP);
          }
        }
        printf(CLEAR_INPUT_PROMPT_);
        printf("%s", responseP);
        continue;
      // Enter = Done
      case '\n':
        goto exit;
        break;
      // Backspace
      case BACKSPACE:
        if (strlen(responseP) > 0) {
          responseP[strlen(responseP) - 1] = '\0';
        }
        printf(CLEAR_INPUT_PROMPT_);
        printf("%s", responseP);
        continue;
      // Delete one word backwards
      case CTRL_W:
        for (int i = strlen(responseP); i > 0; --i) {
          if (i == 1) {
            responseP[0] = '\0';
            break;
          }
          else if (responseP[i] == '_') {
            responseP[i - 1] = '\0';
            break;
          }
        }
        printf(CLEAR_INPUT_PROMPT_);
        printf("%s", responseP);
        continue;
      // Delete whole line
      case CTRL_U:
        printf(CLEAR_INPUT_PROMPT_);
        memset(responseP, 0, arrayGetElemSz(responseP));
        continue;
      default:
        if (strlen(responseP) < sizeof(responseP) - 1) {
          //newChar[0] = _purifyChar(newChar[0]);
          //newChar[0] = _upperChar(newChar[0]);
          strcat(responseP, newChar);
        }
        putchar(newChar[0]);
        continue;
    }
  }

exit:
  _purify(responseP);
  _upper(responseP);

  return responseP;
}

char* genieAsk(char* question, ListType listType, List *listP, U8 verbose) {
  printf("%s\n", question);
  return _genieListen(listType, listP, verbose);
}
