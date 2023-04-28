#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "json.h"

void recurseInto(fjson_object *jsonObjP) {
  struct fjson_object_iterator itr =  fjson_object_iter_begin(jsonObjP);
  struct fjson_object_iterator itrEnd =  fjson_object_iter_end(jsonObjP);

  struct fjson_object *objP =  NULL;
  
  int _int = 0;
  char *str = NULL;
  int nArrElems = 0;
  array_list *array = NULL;
  for (int i = 0; !fjson_object_iter_equal(&itr, &itrEnd); fjson_object_iter_next(&itr)) {
    fjson_object *currObjP = fjson_object_iter_peek_value(&itr);
    const char *name = fjson_object_iter_peek_name(&itr);
    switch(fjson_object_get_type(currObjP)) {
    case fjson_type_int:
      _int = fjson_object_get_int(currObjP);
      printf("int: %s - value %d\n", name, _int);
      break;
    case fjson_type_object:
      printf("object: %s\n", name);
      recurseInto(currObjP);
      break;
    case fjson_type_string:
      str = (char*) fjson_object_get_string(currObjP);
      printf("string: %s - value %s\n", name, str);
      break;
    case fjson_type_array:
      printf("array: %s.\n", name);
      nArrElems = fjson_object_array_length(currObjP);
      for (int j = 0; j < nArrElems; ++j) {
        objP = fjson_object_array_get_idx(currObjP, j);
        recurseInto(objP);
      }
      break;
    default:
      break;
    }
    //printf("%s: %s\n", fjson_object_iter_peek_name(&itr), fjson_object_get_string(currObjP));
  }
}

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	fjson_object *new_obj;
  /* Which one you choose depends on whether you want to test 
     on the json file's "frames" being an array or object. */
#if 1
  FILE *fP = fopen("./red.json", "r");
#else
  FILE *fP = fopen("./red2.json", "r");
#endif
  if (!fP) {
    printf("failed to open target file\n");
    return 1;
  }
  fseek(fP, 0, SEEK_END);
  int nBytes = ftell(fP);
  char jsonFileContents[nBytes];
  memset(jsonFileContents, 0, nBytes);
  fseek(fP, 0, SEEK_SET);

  int nBytesRead = fread(&jsonFileContents[0], 1, nBytes, fP);
  assert(nBytesRead == nBytes);

  fjson_object *mbObjP = fjson_tokener_parse(jsonFileContents);

  // Go dig up all the stuff.
  recurseInto(mbObjP);
	return 0;
}
