#include "../ge.h"

/* START_ROW_DEFS */
Uint32 *town1_pos = {1543, 123, 12, 51, 61643, 23, 15};
Uint8  *town1_ori = {0,      3,  2,  1,     3,  1,  2};
Uint32 *final_pos = {1243, 1521, 1234};
Uint8  *final_ori = {   0,    3,    2};
/* END_ROW_DEFS */

/* Idx #, Position array, Orientation array */
/* START_TBL_DEF */
PosOriTblRow *posori_tbl = {
	/*   0 */ {town1_pos, town1_ori},
	/*   1 */ {town2_pos, town2_ori},
	/* 999 */ {final_pos, final_ori}
/* END_TBL_DEF */
};
