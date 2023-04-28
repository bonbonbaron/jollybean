#include "tau/tau.h"
#include "mail.h"


TAU_MAIN()

typedef struct Tau {
  Error e;
  Message *mailboxF;
  Message *fwdMailboxF;
  U32 maxNMsgs;
  // Put stuff to instantiate here. 
  // This is a particularly good place for pointers since double-poitners are needed to allocate them.
} Tau;

TEST_F_SETUP(Tau) {
  tau->maxNMsgs = 50;
  tau->e = mailboxNew(&tau->mailboxF, tau->maxNMsgs);
  REQUIRE_EQ(tau->e, SUCCESS);
  REQUIRE_EQ(frayGetNElems_(tau->mailboxF), tau->maxNMsgs);
  REQUIRE_EQ(frayGetElemSz_(tau->mailboxF), sizeof(Message));
  REQUIRE_EQ(_frayGetNActive(tau->mailboxF), 0);
  tau->e = mailboxNew(&tau->fwdMailboxF, tau->maxNMsgs);
  REQUIRE_EQ(tau->e, SUCCESS);
  REQUIRE_EQ(frayGetNElems_(tau->fwdMailboxF), tau->maxNMsgs);
  REQUIRE_EQ(frayGetElemSz_(tau->fwdMailboxF), sizeof(Message));
  REQUIRE_EQ(_frayGetNActive(tau->fwdMailboxF), 0);
}

TEST_F_TEARDOWN(Tau) {
  mailboxDel(&tau->mailboxF);
  REQUIRE_TRUE(tau->mailboxF == NULL);
  mailboxDel(&tau->fwdMailboxF);
  REQUIRE_TRUE(tau->fwdMailboxF == NULL);
}


TEST_F(Tau, mailboxWrite) {
  tau-> e = mailboxWrite(tau->mailboxF, 1, 2, 3, 4);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(tau->mailboxF[0].address, 1);
  CHECK_EQ(tau->mailboxF[0].attn, 2);
  CHECK_EQ(tau->mailboxF[0].cmd, 3);
  CHECK_EQ(tau->mailboxF[0].arg, 4);
}

TEST_F(Tau, mailboxWrite_TillFull) {
  while (!tau->e) {
    tau->e = mailboxWrite(tau->mailboxF, 1, 2, 3, 4);
    if (!tau->e) {
      U32 lastMsg = *_frayGetFirstEmptyIdxP(tau->mailboxF) - 1;
      CHECK_EQ(tau->mailboxF[lastMsg].address, 1);
      CHECK_EQ(tau->mailboxF[lastMsg].attn, 2);
      CHECK_EQ(tau->mailboxF[lastMsg].cmd, 3);
      CHECK_EQ(tau->mailboxF[lastMsg].arg, 4);
    }
  }
  CHECK_EQ(tau->e, E_FRAY_FULL);
}

TEST_F(Tau, mailboxForward) {
  tau->e = mailboxWrite(tau->mailboxF, 1, 2, 3, 4);
  CHECK_EQ(tau->e, SUCCESS);
  tau->e = mailboxForward(tau->fwdMailboxF, &tau->mailboxF[0]);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(tau->mailboxF[0].address, tau->fwdMailboxF[0].address);
  CHECK_EQ(tau->mailboxF[0].attn,    tau->fwdMailboxF[0].attn);
  CHECK_EQ(tau->mailboxF[0].cmd,     tau->fwdMailboxF[0].cmd);
  CHECK_EQ(tau->mailboxF[0].arg,     tau->fwdMailboxF[0].arg);
}

TEST_F(Tau, mailboxNew_BadArgs) {
  tau->e = mailboxNew(NULL, 100);
  CHECK_EQ(tau->e, E_BAD_ARGS);
  Message *mbP = NULL;
  tau->e = mailboxNew(&mbP, 0);
  CHECK_EQ(tau->e, E_BAD_ARGS);
}
