#include <stdio.h>
#include <string.h>

#define TO_STRING_(x_) #x_

int main() {
  char s[500];
  char d[] = "donkeys";
  char g[] = "gorillas";
  sprintf( s, "I love %s.\n", d );
  printf( "Hello %s. %s", TO_STRING_(world), s );
  sprintf( s, "I love %s.\n", g );
  printf( "But now %s", s );
  return 0;
}
