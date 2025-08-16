#ifndef MAIL_H
#define MAIL_H

#include "data/mem.h"
#include "data/fray.h"

// Mailboxes
typedef struct {
	Key address;     // e.g. motion system
	Key attn;        // e.g. specific entity in motion system 
	Key cmd;         // e.g. animate, move, [de]activate, or do nothing 
	Key arg;         // e.g. change animation to WALK_UP animation
  Generic attachment;  // e.g. "translate rectangle to/scale imatge by these XY coordinates"
} Message;  


/* Is it practical for arg to always be a unsigned byte?
 *
 * Giving damage: NO
 * Accelerating by an arbitrary amount: NO
 *
 * ... Okay, so I'm convinced.
 * Sounds like Message should be a little more complicated.
 */
Message* mailboxNew( U32 maxNMsgs, const PoolId poolId);
void  mailboxDel(Message **mailboxPP);
void mailboxWrite(Message *mailboxF, Key address, Key attn, Key cmd, Key arg, Generic* attachment);
typedef void (*inboxRead)(Message *mailboxF);

#endif  // #ifndef MAIL_H
