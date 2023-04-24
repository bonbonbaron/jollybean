#include "mail.h"

// There is no corresponding mailboxRead() function because that's specific to each implementer.
Error mailboxWrite(Message *mailboxP, Key address, Key attn, Key cmd, Key arg) {
  Message message = {
    .address = address,
    .attn = attn,
    .cmd = cmd,
    .arg = arg 
  };
  return frayAdd((void*) mailboxP, &message, NULL);
}

Error mailboxForward(Message *mailboxP, Message *msgP) {
  return frayAdd((void*) mailboxP, msgP, NULL);
}
