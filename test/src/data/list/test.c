#include "tau.h"
#include "data/list.h"

typedef struct Elem {
  ListNodeHeader hdr;
  int i;
} Elem;

struct Array { 
  Elem *P;
  List list1;
  List list2;
};

static const int N_ELEMS = 100;

TAU_MAIN()

  TEST_F_SETUP(Array) {
    tau->P = arrayNew( sizeof(Elem), N_ELEMS, GENERAL );
    REQUIRE_EQ(arrayGetNElems(tau->P), N_ELEMS);
    REQUIRE_EQ(arrayGetElemSz(tau->P), sizeof(Elem));
    // Populate the array.
    for (U32 i = 0; i < N_ELEMS; ++i) {
      tau->P[i].i = i;
    }
    // Init the lists.
    listIni( &tau->list1, tau->P );
    listIni( &tau->list2, tau->P );
    CHECK_EQ( tau->list1.head, UNSET_ );
    CHECK_EQ( tau->list1.tail, UNSET_ );
    CHECK_EQ( tau->list2.head, UNSET_ );
    CHECK_EQ( tau->list2.tail, UNSET_ );
  }

TEST_F_TEARDOWN(Array) {
  memRst( GENERAL );
}

TEST_F(Array, listAppendWhenEmpty) {
  listAppend( &tau->list1, &tau->P[5].hdr );
  CHECK_EQ( tau->list1.head, 5);
  CHECK_EQ( tau->list1.tail, tau->list1.head );
  CHECK_EQ( tau->P[5].hdr.next, UNSET_);
  CHECK_EQ( tau->P[5].hdr.prev, UNSET_);
}

TEST_F(Array, listAppendWhenNotEmpty) {
  for (size_t i = 0; i < 20; ++i ) {
    listAppend( &tau->list1, &tau->P[i].hdr );
    CHECK_EQ( tau->list1.head, 0);
    CHECK_EQ( tau->list1.tail, i );
    if ( i > 0 ) {
      CHECK_EQ( tau->P[i - 1].hdr.next, i);
    }
    CHECK_EQ( tau->P[i].hdr.prev, i == 0 ? UNSET_ : i - 1);
  }
}

TEST_F(Array, listPrependWhenEmpty) {
  listPrepend( &tau->list1, &tau->P[5].hdr );
  CHECK_EQ( tau->list1.head, 5);
  CHECK_EQ( tau->list1.tail, tau->list1.head );
  CHECK_EQ( tau->P[5].hdr.next, UNSET_);
  CHECK_EQ( tau->P[5].hdr.prev, UNSET_);
}

TEST_F(Array, listPrependWhenNotEmpty) {
  for (size_t i = 0; i < 20; ++i ) {
    listPrepend( &tau->list1, &tau->P[i].hdr );
    CHECK_EQ( tau->list1.head, i);
    CHECK_EQ( tau->list1.tail, 0 );
    if ( i > 0 ) {
      CHECK_EQ( tau->P[i - 1].hdr.prev, i);
    }
    CHECK_EQ( tau->P[i].hdr.next, i == 0 ? UNSET_ : i - 1);
  }
}

TEST_F(Array, listRemoveNodeWhenEmpty) {
  listRemove( &tau->list1, &tau->P[0].hdr );
  CHECK_EQ( tau->list1.head, UNSET_);
  CHECK_EQ( tau->list1.tail, UNSET_);
}

TEST_F(Array, listRemoveTheOnlyNode) {
  listAppend( &tau->list1, &tau->P[0].hdr );
  CHECK_EQ( tau->list1.head, 0);
  CHECK_EQ( tau->list1.tail, 0);
  listRemove( &tau->list1, &tau->P[0].hdr );
  CHECK_EQ( tau->list1.head, UNSET_);
  CHECK_EQ( tau->list1.tail, UNSET_);
}

TEST_F(Array, listRemoveNodeAfterAppends) {
  for (size_t i = 0; i < 20; ++i ) {
    listAppend( &tau->list1, &tau->P[i].hdr );
  }
  listRemove( &tau->list1, &tau->P[7].hdr );
  Elem* elemP = &tau->P[ tau->list1.head ];
  Elem* lastElemP = &tau->P[ tau->list1.tail ];
  int correctVal = 0;
  goto start;
  for ( ; elemP != lastElemP; ++correctVal ) {
    // Hack for letting it get all the way to the last element before quitting.
    elemP = &tau->P[ elemP->hdr.next ];
start:
    if ( correctVal == 7 ) {
      // Double-increment the correctVal to keep up.
      ++correctVal;
    }
    CHECK_EQ( elemP->i, correctVal );
  }
}

TEST_F(Array, listRemoveNodeAfterPrepends) {
  for (size_t i = 0; i < 20; ++i ) {
    listPrepend( &tau->list1, &tau->P[i].hdr );
  }
  listRemove( &tau->list1, &tau->P[7].hdr );
  Elem* elemP = &tau->P[ tau->list1.head ];
  Elem* lastElemP = &tau->P[ tau->list1.tail ];
  int correctVal = 19;
  goto start;
  for ( ; elemP != lastElemP; --correctVal ) {
    // Hack for letting it get all the way to the last element before quitting.
    elemP = &tau->P[ elemP->hdr.next ];
start:
    if ( correctVal == 7 ) {
      // Double-increment the correctVal to keep up.
      --correctVal;
    }
    CHECK_EQ( elemP->i, correctVal );
  }
}

TEST_F(Array, listInsertBeforeHead ) {
  listAppend( &tau->list1, &tau->P[0].hdr );
  CHECK_EQ( tau->list1.head, 0 );
  CHECK_EQ( tau->list1.tail, 0 );
  listInsertBefore( &tau->list1, &tau->P[7].hdr, &tau->P[0].hdr );
  CHECK_EQ( tau->list1.head, 7 );
  CHECK_EQ( tau->list1.tail, 0 );
  CHECK_EQ( tau->P[7].hdr.next, 0 );
  CHECK_EQ( tau->P[0].hdr.prev, 7 );
}

TEST_F(Array, listInsertAfterTail ) {
  listAppend( &tau->list1, &tau->P[0].hdr );
  CHECK_EQ( tau->list1.head, 0 );
  CHECK_EQ( tau->list1.tail, 0 );
  listInsertAfter( &tau->list1, &tau->P[7].hdr, &tau->P[0].hdr );
  CHECK_EQ( tau->list1.head, 0 );
  CHECK_EQ( tau->list1.tail, 7 );
  CHECK_EQ( tau->P[0].hdr.next, 7 );
  CHECK_EQ( tau->P[7].hdr.prev, 0 );
}

TEST_F(Array, listInsertBeforeWhenNotEmpty ) {
  for (size_t i = 0; i < 20; ++i ) {
    listAppend( &tau->list1, &tau->P[i].hdr );
  }
  // Honestly, I'm trusting devs to not re-insert a member node.
  listInsertBefore( &tau->list1, &tau->P[31].hdr, &tau->P[12].hdr );
  CHECK_EQ( tau->P[11].hdr.next, 31 );
  CHECK_EQ( tau->P[31].hdr.prev, 11 );
  CHECK_EQ( tau->P[31].hdr.next, 12 );
  CHECK_EQ( tau->P[12].hdr.prev, 31 );
  CHECK_EQ( tau->P[12].hdr.next, 13 );
}

TEST_F(Array, listInsertAfterWhenNotEmpty ) {
  for (size_t i = 0; i < 20; ++i ) {
    listAppend( &tau->list1, &tau->P[i].hdr );
  }
  // Honestly, I'm trusting devs to not re-insert a member node.
  listInsertAfter( &tau->list1, &tau->P[31].hdr, &tau->P[11].hdr );
  CHECK_EQ( tau->P[11].hdr.next, 31 );
  CHECK_EQ( tau->P[31].hdr.prev, 11 );
  CHECK_EQ( tau->P[31].hdr.next, 12 );
  CHECK_EQ( tau->P[12].hdr.prev, 31 );
  CHECK_EQ( tau->P[12].hdr.next, 13 );
}

TEST_F(Array, appendMultipleListsIntoOneArray ) {
  for (size_t i = 0; i < 20; ++i ) {
    if ( i & 1 ) {
      listAppend( &tau->list1, &tau->P[i].hdr );
    }
    else {
      listAppend( &tau->list2, &tau->P[i].hdr );
    }
  }
  // CHECK LIST 1
  Elem* elemP = &tau->P[ tau->list1.head ];
  Elem* lastElemP = &tau->P[ tau->list1.tail ];
  int correctVal = 1;
  goto start1;
  for ( ; elemP != lastElemP; correctVal += 2 ) {
    // Hack for letting it get all the way to the last element before quitting.
    elemP = &tau->P[ elemP->hdr.next ];
start1:
    CHECK_EQ( elemP->i, correctVal );
  }
  // CHECK LIST 1
  elemP = &tau->P[ tau->list2.head ];
  lastElemP = &tau->P[ tau->list2.tail ];
  correctVal = 0;
  goto start2;
  for ( ; elemP != lastElemP; correctVal += 2 ) {
    // Hack for letting it get all the way to the last element before quitting.
    elemP = &tau->P[ elemP->hdr.next ];
start2:
    CHECK_EQ( elemP->i, correctVal );
  }
}

TEST_F(Array, prependMultipleListsIntoOneArray ) {
  for (size_t i = 0; i < 20; ++i ) {
    if ( i & 1 ) {
      listPrepend( &tau->list1, &tau->P[i].hdr );
    }
    else {
      listPrepend( &tau->list2, &tau->P[i].hdr );
    }
  }
  // CHECK LIST 1
  Elem* elemP = &tau->P[ tau->list1.head ];
  Elem* lastElemP = &tau->P[ tau->list1.tail ];
  int correctVal = 19;
  goto start1;
  for ( ; elemP != lastElemP; correctVal -= 2 ) {
    // Hack for letting it get all the way to the last element before quitting.
    elemP = &tau->P[ elemP->hdr.next ];
start1:
    CHECK_EQ( elemP->i, correctVal );
  }
  // CHECK LIST 1
  elemP = &tau->P[ tau->list2.head ];
  lastElemP = &tau->P[ tau->list2.tail ];
  correctVal = 18;
  goto start2;
  for ( ; elemP != lastElemP; correctVal -= 2 ) {
    // Hack for letting it get all the way to the last element before quitting.
    elemP = &tau->P[ elemP->hdr.next ];
start2:
    CHECK_EQ( elemP->i, correctVal );
  }
}

TEST_F( Array, listMerge ) {
  for (size_t i = 0; i < 20; ++i ) {
    if ( i < 10 ) {
      listAppend( &tau->list1, &tau->P[i].hdr );
    }
    else {
      listAppend( &tau->list2, &tau->P[i].hdr );
    }
  }
  listMerge( &tau->list2, &tau->list1 );

  // Now let's check it.
  Elem* elemP = &tau->P[ tau->list1.head ];
  Elem* lastElemP = &tau->P[ tau->list1.tail ];
  size_t i = 0;
  goto start3;
  for ( ; i < 20; ++i ) {
    // Hack for letting it get all the way to the last element before quitting.
    elemP = &tau->P[ elemP->hdr.next ];
start3:
    CHECK_EQ( elemP->i, i );
  }
}
