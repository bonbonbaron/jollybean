#include "data/mail.h"

// There is no corresponding mailboxRead() function because that's specific to each implementer.
Message* mailboxNew(U32 maxNMsgs, const PoolId poolId) {
  assert (maxNMsgs);
  return (Message*) frayNew( sizeof(Message), maxNMsgs, poolId );
}

void mailboxWrite(Message *mailboxP, Key address, Key attn, Key cmd, Key arg, Generic* attachmentP) {
  Message msg = {
    .address = address,
    .attn = attn,
    .cmd = cmd,
    .arg = arg
  };
  if (attachmentP) {
    msg.attachment = *attachmentP;
  }
  else {
    msg.attachment.u32 = 0;
  }
  frayAdd((void*) mailboxP, &msg, NULL);
}

void mailboxForward(Message *mailboxP, Message *msgP) {
  frayAdd((void*) mailboxP, msgP, NULL);
}
