#include "ge.h"


Error subscribe(Sprite *s) {
  Error stat = 0;
  Uint8 i, channel;
  ReactSeqGrp *react_seq_grp;

  react_seq_grp = tbl_react[s->type].ptr;

  for (i = 0 ; (i < react_seq_grp->num_react_seqs) && !stat; i++) {
    channel = react_seq_grp->triggers[i];
    if (comm_channels[channel].subscribers == NULL) {
      stat = init_list(&comm_channels[channel].subscribers);
    }
    if (!stat) {
      stat = list_insert_tail(comm_channels[channel].subscribers, s);
    }
  }
    
  return stat;
}

Error unsubscribe(Sprite *s, Uint8 channel) {
  ListNode* node;

  if (comm_channels[channel].subscribers == NULL) {
    printf("No linked list in channel %d to unsubscribe sprite species %d from.\n", channel, s->species);
    return 1;
  }

  /* Find sprite in channel and remove his ass.*/
  node = list_find_node(comm_channels[channel].subscribers, s);
  if (node != NULL) {
    list_remove_node(comm_channels[channel].subscribers, node, NULL, NULL);
  }
  /* Otherwise don't sweat it; do nothing. */

  return 0;
}

void broadcast(Uint8 channel) {
  extern SDL_bool signals_exist;
  signals_exist = SDL_TRUE;
  comm_channels[channel].signal_on = SDL_TRUE;
}

void cancel(Uint8 channel) {
  comm_channels[channel].signal_on = SDL_FALSE;
}

Error trigger_reaction(Subscriber *subscriber, Uint8 signal, LinkedList *active_subscribers) {
  Error stat = 0; 

  if (signal < subscriber->s->react_seq_grp->num_react_seqs) {
    subscriber->busy = SDL_TRUE;
    subscriber->activity.current_reaction_sequence = subscriber->s->react_seq_grp->react_seq_ptr[signal];
    //subscriber->tgt (TODO)
    list_insert_tail(active_subscribers, subscriber);
  }

  return stat;
}