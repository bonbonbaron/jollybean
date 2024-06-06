#ifndef MAIL_H
#define MAIL_H

#include "mem.h"
#include "fray.h"

// Mailboxes
typedef struct {
	Key address;   // e.g. motion system
	Key attn;      // e.g. specific component in motion system 
	Key cmd;       // e.g. move, [de]activate, or do nothing ( 
	Key arg;       // e.g. change component value to MOVE_UP
} Message;  

Message* mailboxNew( U32 maxNMsgs);
void  mailboxDel(Message **mailboxPP);
void mailboxWrite(Message *mailboxF, Key address, Key attn, Key cmd, Key arg);
void mailboxForward(Message *mailboxF, Message *msgP);
typedef void (*inboxRead)(Message *mailboxF);

#endif  // #ifndef MAIL_H
