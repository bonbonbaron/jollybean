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

#define CTRL_U (21)
#define CTRL_W (23)
#define BACKSPACE (127)

Error genieListen(ListType listType) {
  int e = SUCCESS;
  char newChar[2] = {0};
  char input[100] = {0};
	for (;;) {
    newChar[0] = _getch();
    switch(newChar[0]) {
      case '\t':
        printf("tab\n");
        if (listType == TEXT_FILE) {
          continue;	// do autocomplete text version here
        }
        else if (listType == DIR_FILE) {
          continue;	// do autocomplete dir version here
        }
        break;
      case '\n':
        goto exit;
        break;
      case BACKSPACE:
        if (strlen(input) > 0) {
          input[strlen(input) - 1] = '\0';
        }
        printf("\33[2K\r");
        printf("%s", input);
        continue;
      case CTRL_W:
        for (int i = strlen(input); i > 0; --i) {
          if (i == 1) {
            input[0] = '\0';
            break;
          }
          else if (input[i] == '_') {
            input[i - 1] = '\0';
            break;
          }
        }
        printf("\33[2K\r");
        printf("%s", input);
        continue;
      case CTRL_U:
        printf("\33[2K\r");
        //printf("ctrl u\n");
        memset(input, 0, sizeof(input));
        continue;
      default:
        if (strlen(input) < sizeof(input) - 1) {
          newChar[0] = _purifyChar(newChar[0]);
          newChar[0] = _upperChar(newChar[0]);
          strcat(input, newChar);
        }
        putchar(newChar[0]);
        //printf("in default\n");
        continue;
    }

    if (!e) {  // 0 if fail
      e = E_FILE_IO;
      break;
    }
  }
exit:
  _purify(input);
  _upper(input);
  printf("\n\nFinal output:\n%s\n", input);
  return e;
}



/*
	 On tab, call an autocomplete function. 
	 That autocomplete will either run through
			* a directory's names or
			* a text file's listings (generated at runtime by a jb sed script).
	 
		
 * */
