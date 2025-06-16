#include "tau.h"
#include "data/mail.h"


TAU_MAIN()

typedef struct Tau {
  Message *mailboxF;
  Message *fwdMailboxF;
  U32 maxNMsgs;
  // Put stuff to instantiate here. 
  // This is a particularly good place for pointers since double-poitners are needed to allocate them.
} Tau;

TEST_F_SETUP(Tau) {
  tau->maxNMsgs = 50;
  tau->mailboxF = mailboxNew( tau->maxNMsgs, GENERAL);
  REQUIRE_EQ(frayGetNElems_(tau->mailboxF), tau->maxNMsgs);
  REQUIRE_EQ(frayGetElemSz_(tau->mailboxF), sizeof(Message));
  REQUIRE_EQ(_frayGetNActive(tau->mailboxF), 0);
  tau->fwdMailboxF = mailboxNew(tau->maxNMsgs, GENERAL);
  REQUIRE_EQ(frayGetNElems_(tau->fwdMailboxF), tau->maxNMsgs);
  REQUIRE_EQ(frayGetElemSz_(tau->fwdMailboxF), sizeof(Message));
  REQUIRE_EQ(_frayGetNActive(tau->fwdMailboxF), 0);
}

TEST_F_TEARDOWN(Tau) {
  memRst( GENERAL );
}


TEST_F(Tau, mailboxWrite) {
  mailboxWrite(tau->mailboxF, 1, 2, 3, 4, NULL);
  CHECK_EQ(tau->mailboxF[0].address, 1);
  CHECK_EQ(tau->mailboxF[0].attn, 2);
  CHECK_EQ(tau->mailboxF[0].cmd, 3);
  CHECK_EQ(tau->mailboxF[0].arg, 4);
}

TEST_F(Tau, mailboxWrite_TillFull) {
  for (U32 i = 0; i < frayGetNElems_(tau->mailboxF); ++i) {
    mailboxWrite(tau->mailboxF, 1, 2, 3, 4, NULL);
    U32 lastMsg = *_frayGetFirstEmptyIdxP(tau->mailboxF) - 1;
    CHECK_EQ(tau->mailboxF[lastMsg].address, 1);
    CHECK_EQ(tau->mailboxF[lastMsg].attn, 2);
    CHECK_EQ(tau->mailboxF[lastMsg].cmd, 3);
    CHECK_EQ(tau->mailboxF[lastMsg].arg, 4);
  }
}

TEST_F(Tau, mailboxForward) {
  mailboxWrite(tau->mailboxF, 1, 2, 3, 4, NULL);
  mailboxForward(tau->fwdMailboxF, &tau->mailboxF[0]);
  CHECK_EQ(tau->mailboxF[0].address, tau->fwdMailboxF[0].address);
  CHECK_EQ(tau->mailboxF[0].attn,    tau->fwdMailboxF[0].attn);
  CHECK_EQ(tau->mailboxF[0].cmd,     tau->fwdMailboxF[0].cmd);
  CHECK_EQ(tau->mailboxF[0].arg,     tau->fwdMailboxF[0].arg);
}
