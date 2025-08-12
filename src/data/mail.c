#include "data/mail.h"

// There is no corresponding mailboxRead() function because that's specific to each implementer.
Message* mailboxNew(U32 maxNMsgs, const PoolId poolId) {
  assert (maxNMsgs);
  return (Message*) frayNew( sizeof(Message), maxNMsgs, poolId );
}

void mailboxWrite(Message *mailboxP, Key address, Key attn, Key cmd, Key arg, Generic* attachmentP) {
  U32 newIdx;
  frayAddEmpty( (void*) mailboxP, &newIdx );
  Message *msgP = &mailboxP[ newIdx ];
  msgP->address = address;
  msgP->attn = attn;
  msgP->cmd = cmd;
  msgP->arg = arg;
  if (attachmentP) {
    msgP->attachment = *attachmentP;
  }
  else {
    msgP->attachment.u32 = 0;
  }
}

void mailboxForward(Message *mailboxP, Message *msgP) {
  frayAdd((void*) mailboxP, msgP, NULL);
}
