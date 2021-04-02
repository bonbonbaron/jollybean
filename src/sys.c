#include "jb.h"


/***********/
/* Systems */
/***********/
typedef struct {
	Uint8 src_entity_id;
	Uint8 src_entity_type;
	Uint8 dst_entity_id;
	Uint8 dst_entity_type;
	Uint8 signal;
	Uint16 val1;
	Uint16 val2;
	Uint16 val3;
	Uint16 val4;
} Signal;


typedef Error (*SysCallback)(Signal *sig_p);


// typedef struct {
// 	SDL_bool			 	 active;
// 	struct _System  *parent_p;
// 	struct _System  *children_1a;
// 	Uint8          **directory_2a;   /* 2D array mapping entities to systems' components. 0xFF means no component. Columns correspond to systems; rows to entities. */
// 	Signal          *signal_1a;      /* active signals are always in front; stop iteration at dead signal */
// 	SysCallback     *callbacks_1a;   /* to clean up code, implement array and 2D array to avoid having so many num_XXXXXX-like members */
// } System;
	
Error new_sys(System **sys_pp) {
	if (sys_p == NULL) {
		return 1;
	}
	*sys_pp = calloc(sizeof(System));
	if (*sys_pp == NULL) {
		print_no_mem("ini_sys");
		return 1;
	}
	return 0;
}

void del_sys(System **sys_p) {
	if (*sys_p !=  NULL) {
		free(*sys_p);
		*sys_p = NULL;
	}
}
