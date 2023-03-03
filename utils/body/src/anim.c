#include "anim.h"

// Frames ------------
Error frameNodeNew(FrameNode **nodePP) {
  if (!nodePP)
    return E_BAD_ARGS;
  Error e = jbAlloc((void**) nodePP, sizeof(FrameNode), 1);
  if (!e) 
    memset(*nodePP, 0, sizeof(FrameNode));
  return e;
}

Error frameNodeGrow(FrameNode **nodePP) {
  if (!nodePP)
    return E_BAD_ARGS;
  Error e = SUCCESS;
  if (!*nodePP)
    e = frameNodeNew(nodePP);
  else {
    e = frameNodeNew(&(*nodePP)->nextP);
    if (!e)
      *nodePP = (*nodePP)->nextP;
  }
  return e;
}

void frameNodeDel(FrameNode **nodePP) {
  if (nodePP && *nodePP) {
    jbFree((void**) nodePP);
  }
}

// Tags  ---------------
Error tagNodeNew(TagNode **nodePP) {
  if (!nodePP)
    return E_BAD_ARGS;
  Error e = jbAlloc((void**) nodePP, sizeof(TagNode), 1);
  if (!e) 
    memset(*nodePP, 0, sizeof(TagNode));
  return e;
}

Error tagNodeGrow(TagNode **nodePP) {
  if (!nodePP)
    return E_BAD_ARGS;
  Error e = SUCCESS;
  if (!*nodePP)
    e = tagNodeNew(nodePP);
  else {
    e = tagNodeNew(&(*nodePP)->nextP);
    if (!e)
      *nodePP = (*nodePP)->nextP;
  }
  return e;
}

void tagNodeDel(FrameNode **nodePP) {
  if (nodePP && *nodePP) {
    jbFree((void**) nodePP);
  }
}


// At this point the caller has run into either a named object within the "frames" object, or
// an anonymous object within the "frames" array.
void setFrame(FrameNode *frameP, fjson_object *jsonObjP) {
  struct fjson_object_iterator itr =  fjson_object_iter_begin(jsonObjP);
  struct fjson_object_iterator itrEnd =  fjson_object_iter_end(jsonObjP);
  struct fjson_object_iterator innerItr; //struct fjson_object_iterator innerItrEnd;
  struct fjson_object *objP =  NULL;
  
  // Iterate through the frame's inner members.
  for (; !fjson_object_iter_equal(&itr, &itrEnd); fjson_object_iter_next(&itr)) {
    fjson_object *currObjP = fjson_object_iter_peek_value(&itr);
    const char *name = fjson_object_iter_peek_name(&itr);
    switch(fjson_object_get_type(currObjP)) {
    case fjson_type_object:
      // This name was STUPIDLY confusing. Frame is a member of a frames array element. Stupid stupid stupid!
      if (!strcmp(name, "frame")) {
        innerItr    = fjson_object_iter_begin(currObjP);
        //innerItrEnd =  fjson_object_iter_end(currObjP);
        fjson_object *innerObjP = fjson_object_iter_peek_value(&innerItr);
        // Get x, y, w, and h.
        // TODO if you really want to be safe instead of assuming, do lines like this.
        // const char *frameComponentName = fjson_object_iter_peek_name(&innerItr);
        // assert(!strcmp(frameComponentName, "x"));  
        frameP->x = fjson_object_get_int(innerObjP);
        fjson_object_iter_next(&innerItr);
        innerObjP = fjson_object_iter_peek_value(&innerItr);
        frameP->y = fjson_object_get_int(innerObjP);
        fjson_object_iter_next(&innerItr);
        innerObjP = fjson_object_iter_peek_value(&innerItr);
        frameP->w = fjson_object_get_int(innerObjP);
        fjson_object_iter_next(&innerItr);
        innerObjP = fjson_object_iter_peek_value(&innerItr);
        frameP->h = fjson_object_get_int(innerObjP);
      }  // if this is the "frame" member of the frames array element
      break;
    case fjson_type_int:
      assert(!strcmp(name, "duration"));
      frameP->duration = fjson_object_get_int(currObjP);
      break;
    default:
      break;
    }
  }
}
// At this point the caller has run into the "frames" key. 
// Its value is either going to be an object of objects or an array of objects.
Error setFrames(FrameNode **framePP, fjson_object *jsonObjP, fjson_type framesType, U8 verbose) {
  if (!framePP || !jsonObjP)
    return E_BAD_ARGS;

  Error e = SUCCESS;
  fjson_object *currObjP = NULL;
  FrameNode **currFramePP = framePP;
  FrameNode *firstFrameNodeP = NULL;

  // If we're in "object of objects" frames...
  if (framesType == fjson_type_object) {
    struct fjson_object_iterator itr =  fjson_object_iter_begin(jsonObjP);
    struct fjson_object_iterator itrEnd =  fjson_object_iter_end(jsonObjP);
    // Iterate through the sub-objects within the "frames" object.
    for (; !fjson_object_iter_equal(&itr, &itrEnd); fjson_object_iter_next(&itr)) {
      // Get sub-object name.
      if (verbose) {
        const char *name = fjson_object_iter_peek_name(&itr);
        printf("frames sub-object: %s\n", name);
      }
      // Get sub-object value.
      currObjP = fjson_object_iter_peek_value(&itr);
      // Grow a new frame node either from root or head.
      if (!*currFramePP) {
        e = frameNodeGrow(currFramePP);
        firstFrameNodeP = *currFramePP;
      }
      else {
        e = frameNodeGrow(currFramePP);
      }
      //e = frameNodeGrow(currFramePP);
      // Set the new frame node's values.
      if (!e)
        setFrame(*currFramePP, currObjP); 
    }
  }
  else if (framesType == fjson_type_array) {
    // Allocate new variables.
    int nArrElems = 0;
    fjson_object *arrayObjP = NULL;
    // Get number of array elements.
    struct fjson_object_iterator itr =  fjson_object_iter_begin(jsonObjP);
    //currObjP = fjson_object_iter_peek_value(&itr);
    nArrElems = fjson_object_array_length(jsonObjP);
    // Iterate through frames' array.
    for (int j = 0; j < nArrElems; ++j) {
      // Grow a new frame node either from root or head.
      if (!*currFramePP) {
        e = frameNodeGrow(currFramePP);
        firstFrameNodeP = *currFramePP;
      }
      else {
        e = frameNodeGrow(currFramePP);
      }
      if (!e) {
        arrayObjP = fjson_object_array_get_idx(jsonObjP, j);
        setFrame(*currFramePP, arrayObjP);
      }
    }
  }
  if (!e)
    *framePP = firstFrameNodeP;
  return e;
}

// At this point the caller has run into the "frameTags" key. 
Error setTags(TagNode **tagNodePP, fjson_object *jsonObjP, U8 verbose) {
  if (!tagNodePP || !jsonObjP)
    return E_BAD_ARGS;

  Error e = SUCCESS;
  fjson_object *currObjP = NULL;
  TagNode **currTagPP = tagNodePP;
  TagNode *firstTagNodeP = NULL;
  // Iterate through the tag array...
  //fjson_object *currObjP = fjson_object_iter_peek_value(&itr);
  // Allocate new variables.
  int nArrElems = 0;
  fjson_object *arrayElemObjP = NULL;
  // Get number of array elements.
  nArrElems = fjson_object_array_length(jsonObjP);
  // Iterate through frames' array.
  for (int j = 0; j < nArrElems; ++j) {
    if (verbose)
      printf("Tags array elem #%d...\n", j);
    // Grow a new frame node either from root or head.
    if (!e) {
      // Get frame tag at index j.
      arrayElemObjP = fjson_object_array_get_idx(jsonObjP, j);
      struct fjson_object_iterator arrIter = fjson_object_iter_begin(arrayElemObjP);
      //struct fjson_object_iterator arrIterEnd = fjson_object_iter_end(arrayElemObjP);
      // be sure to store the outcome to come back to it later. I know it's ugly but i'm tired.
      if (!*currTagPP) {
        e = tagNodeGrow(currTagPP);
        firstTagNodeP = *currTagPP;
      }
      else {
        e = tagNodeGrow(currTagPP);
      }

      if (e)
        break;
      // Get name of current object in array.
      // Name
      fjson_object *frameTagInnerObjP = fjson_object_iter_peek_value(&arrIter);
      (*currTagPP)->name = (char*) fjson_object_get_string(frameTagInnerObjP);
      fjson_object_iter_next(&arrIter);
      // From
      frameTagInnerObjP = fjson_object_iter_peek_value(&arrIter);
      (*currTagPP)->from = fjson_object_get_int(frameTagInnerObjP);
      fjson_object_iter_next(&arrIter);
      // To
      frameTagInnerObjP = fjson_object_iter_peek_value(&arrIter);
      (*currTagPP)->to = (U32) fjson_object_get_int(frameTagInnerObjP);
      fjson_object_iter_next(&arrIter);
      // Direction
      frameTagInnerObjP = fjson_object_iter_peek_value(&arrIter);
      (*currTagPP)->direction = (char*) fjson_object_get_string(frameTagInnerObjP);
    }
  }
  if (!e)
    *tagNodePP = firstTagNodeP;
  return e;
}

void printResults(AnimJsonData *animP) {
  for (FrameNode *frameNodeP = animP->frameNodeA; frameNodeP != NULL; frameNodeP = frameNodeP->nextP) {
    printf("Got frame:\n");
    printf("\tx: %d\n", frameNodeP->x);
    printf("\ty: %d\n", frameNodeP->y);
    printf("\tw: %d\n", frameNodeP->w);
    printf("\th: %d\n", frameNodeP->h);
    printf("\tduration: %d\n", frameNodeP->duration);
  }

  for (TagNode *tagNodeP = animP->tagNodeA; tagNodeP != NULL; tagNodeP = tagNodeP->nextP) {
    printf("Got tag:\n");
    printf("\tname: %s\n", tagNodeP->name);
    printf("\tfrom: %d\n", tagNodeP->from);
    printf("\tto: %d\n", tagNodeP->to);
    printf("\tdirection: %s\n", tagNodeP->direction);
  }
}

// JSON tree traversal 
// ====================
Error getJsonData(AnimJsonData *animP, fjson_object *objP, U8 verbose) {
  Error e = SUCCESS;
  struct fjson_object_iterator itr =  fjson_object_iter_begin(objP);
  struct fjson_object_iterator itrEnd =  fjson_object_iter_end(objP);
  // Recurse along the current level of the JSON tree.
  for (; !fjson_object_iter_equal(&itr, &itrEnd); fjson_object_iter_next(&itr)) {
    fjson_object *currObjP = fjson_object_iter_peek_value(&itr);
    const char *name = fjson_object_iter_peek_name(&itr);
    if (!strcmp(name, "frames")) 
      e = setFrames(&animP->frameNodeA, currObjP, fjson_object_get_type(currObjP), verbose);
    else if (!strcmp(name, "meta")) {
      struct fjson_object_iterator metaItr = fjson_object_iter_begin(currObjP);
      struct fjson_object_iterator metaItrEnd =  fjson_object_iter_end(currObjP);
      for (; !fjson_object_iter_equal(&metaItr, &metaItrEnd); fjson_object_iter_next(&metaItr)) {
        const char *name = fjson_object_iter_peek_name(&metaItr);
        if (!strcmp(name, "frameTags")) {
          fjson_object *metaArrayObjP = fjson_object_iter_peek_value(&metaItr);
          e = setTags(&animP->tagNodeA, metaArrayObjP, verbose);
          break;
        }
      }
    }
  }
  return e;
}

TagNode* getTagNode(TagNode *rootP, U32 idx) {
  if (!rootP)
    return NULL;
  TagNode *resultP = rootP;
  for (int i = 0; i < idx; ++i)
    resultP = resultP->nextP;
  return resultP;
}

FrameNode* getFrameNode(FrameNode *rootP, U32 idx) {
  if (!rootP)
    return NULL;
  FrameNode *resultP = rootP;
  for (int i = 0; i < idx; ++i)
    resultP = resultP->nextP;
  return resultP;
}

Error writeAnimJsonData(char *entityName, AnimJsonData *animP, U8 verbose) {
  FILE *fP = getBuildFile("Seed/Genome/Gene/Body/Graybody/Animation/src", entityName, "Anim.c", verbose); 
  if (!fP) {
    return E_FILE_IO;
  }
  Key nKeyValPairs = 0;
  fprintf(fP, "#include \"xAnim.h\"\n");
  fprintf(fP, "#include \"animKeyring.h\"\n\n");
  // Write frame arrays.
  for (TagNode *tagP = animP->tagNodeA; tagP != NULL; tagP = tagP->nextP) {
    // AnimJsonData frame arrays
    fprintf(fP, "AnimFrame *frame_%s_%s_A[] = {\n", entityName, tagP->name);
    FrameNode *frameNodeP = getFrameNode(animP->frameNodeA, tagP->from);
    if (!frameNodeP)
      return E_BAD_ARGS;
    for (int i = tagP->from; frameNodeP && i <= tagP->to; ++i, frameNodeP = frameNodeP->nextP) {
      fprintf(fP, "\t{\n");
      fprintf(fP, "\t\t.rect = {\n");
      fprintf(fP, "\t\t\t.x = %d,\n", frameNodeP->x);
      fprintf(fP, "\t\t\t.y = %d,\n", frameNodeP->y);
      fprintf(fP, "\t\t\t.w = %d,\n", frameNodeP->w);
      fprintf(fP, "\t\t\t.h = %d,\n", frameNodeP->h);
      fprintf(fP, "\t\t},\n");
      fprintf(fP, "\t\t.duration = %d\n", frameNodeP->duration);
      fprintf(fP, "\t},\n");
    }
    fprintf(fP, "};\n\n");
    // AnimJsonData strips
    fprintf(fP, "AnimStrip animStrip_%s_%s = {\n", entityName, tagP->name);
    fprintf(fP, "\t.nFrames = %d,\n", 1 + tagP->to - tagP->from);
    fprintf(fP, "\t.flags = %d,\n", 0);
    // Aseprite left out looping in their tags for some reason, so using the otherwise useless reverse!
    Bln isReverse = !strncasecmp(tagP->direction, "reverse", 4);
    fprintf(fP, "\t.repeat = %d,\n", isReverse);  
    Bln isPingPong = !strncasecmp(tagP->direction, "pingpong", 4);
    fprintf(fP, "\t.pingPong = %d,\n", isPingPong);
    fprintf(fP, "\t.frameA = frame_%s_%s_A\n", entityName, tagP->name);
    fprintf(fP, "};\n\n");
    ++nKeyValPairs;
  }
  // Write key-val pairs between tag names and coll strips
  fprintf(fP, "KeyStripPairArray tagName2AnimStripmap_%s[] = {\n", entityName);
  fprintf(fP, "\t.nKeyStripPairs = %d,\n", nKeyValPairs);
  fprintf(fP, "\t.keyValPairA = {\n");
  for (TagNode *tagP = animP->tagNodeA; tagP != NULL; tagP = tagP->nextP) {
    fprintf(fP, "\t\t{\n");
    fprintf(fP, "\t\t\t.key  = %s,\n", tagP->name);
    fprintf(fP, "\t\t\t.animStripP = &animStrip_%s_%s\n", entityName, tagP->name);
    fprintf(fP, "\t\t},\n");
  }
  fprintf(fP, "\t}\n");
  fprintf(fP, "};\n\n");
  fclose(fP);

  return SUCCESS;
}

/* anim() outputs an AnimJsonData** in case the caller wants to use its 
   frame data to look for collision rectangles. */
Error anim (char *entityNameP, U8 verbose, AnimJsonData **animPP) {
  if (!entityNameP || !animPP) {
    return E_BAD_ARGS;
  }

  FILE *fP = getSrcFile("Body/Graybody/Animation", entityNameP, ".json", verbose); 
  if (!fP) {
    return E_FILE_IO;
  }

  // Get file size
  fseek(fP, 0, SEEK_END);
  int nBytes = ftell(fP);
  if (nBytes <= 0) {
    printf("~/jb/src/Body/GrayBody/Animation/%s.json is empty! Exiting...\n", entityNameP);
    return E_FILE_IO;
  }
  else if (verbose) {
    printf("~/jb/src/Body/GrayBody/Animation/%s.json is %d bytes.\n", entityNameP);
  }
  fseek(fP, 0, SEEK_SET);

  // Init JSON file contents to zeroes.
  char jsonFileContents[nBytes];
  memset(jsonFileContents, 0, nBytes);

  // Read json file
  int nBytesRead = fread(&jsonFileContents[0], 1, nBytes, fP);
  assert(nBytesRead == nBytes);

  // Parse json file
  fjson_object *topLevelObjP = fjson_tokener_parse(jsonFileContents);

  // Turn it into a C struct
  AnimJsonData *animP;
  Error e = jbAlloc((void**) &animP, sizeof(AnimJsonData), 1);

  if (!e) {
    memset(animP, 0, sizeof(AnimJsonData));
    e = getJsonData(animP, topLevelObjP, verbose);
  }
  if (!e && verbose) {
    printResults(animP);
  }
  if (!e) {
    e = writeAnimJsonData(entityNameP, animP, verbose);
  }

  // Return animation if it's good; otherwise free it and return NULL.
  if (!e) {
    *animPP = animP;
  }
  else {
    *animPP = NULL;
    jbFree((void**) &animP);
  }

  return e;
}
