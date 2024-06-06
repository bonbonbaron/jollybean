#include "mail.h"

// There is no corresponding mailboxRead() function because that's specific to each implementer.
Message* mailboxNew(U32 maxNMsgs) {
  assert (maxNMsgs);
  return (Message*) frayNew(sizeof(Message), maxNMsgs);
}

void mailboxDel(Message **mailboxPP) {
  if (mailboxPP && *mailboxPP) {
    frayDel((void**) mailboxPP);
  }
}

void mailboxWrite(Message *mailboxP, Key address, Key attn, Key cmd, Key arg) {
  Message message = {
    .address = address,
    .attn = attn,
    .cmd = cmd,
    .arg = arg 
  };
  frayAdd((void*) mailboxP, &message, NULL);
}

void mailboxForward(Message *mailboxP, Message *msgP) {
  frayAdd((void*) mailboxP, msgP, NULL);
}
