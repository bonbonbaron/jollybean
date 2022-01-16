#ifndef JB_H
#define JB_H
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "SDL.h"
typedef Sint8 Error;
SDL_bool game_over;
 
#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 500

#define TILE_W 8
#define TILE_H TILE_W
 
#define CTRL_U 0x01
#define CTRL_D 0x02
#define CTRL_L 0x04
#define CTRL_R 0x08
#define CTRL_A 0x10
#define CTRL_B 0x20
#define CTRL_X 0x40
#define CTRL_S 0x80
 
#define FPS 30
#define FRAME_DURATION (1000)/(FPS)  /* (1000 ms/s) * (s/N frames) = 1000/N milliseconds/frame */


/* Linked lists */
typedef struct ListNode {
	void *data;
	struct ListNode *next, *prev;
} ListNode;

typedef struct LinkedList {
	ListNode *head;
	ListNode *tail;
} LinkedList;

Error inilist(LinkedList **ll);
void  dellist(LinkedList *ll);
Error list_insert_tail(LinkedList *ll, void *data);
Error list_insert_head(LinkedList *ll, void *data);
void list_remove_node(LinkedList *ll, ListNode *node, ListNode **next, void **data);
ListNode* list_find_node(LinkedList *ll, void *data);

/* QuadTrees */
struct _QuadNode;
typedef struct _QuadNode {
	struct _QuadNode *nodes[4];
	Uint8 n_elems, depth;
	Vector2 tl_corner, br_corner;
	LinkedList *elems;
} QuadNode;

typedef struct QuadTree {
	QuadNode *root;
	Uint8 max_depth, max_n_elems;
} QuadTree;

Error qt_init(QuadTree **qt, Uint8 max_depth, Vector2 *tree_rect_dims, Uint8 max_n_elems);
Error qn_init(QuadNode **qn, Uint8 depth, Vector2 *tl, Vector2 *br);
void qt_delete(QuadTree **qt);
void qn_delete(QuadNode **qn);
void qt_query(QuadTree *qt, Vector2 *tl, Vector2 *br, QuadNode **qn);
void qn_query(QuadTree *qt, Vector2 *tl, Vector2 *br, QuadNode **qn);
Error qt_insert(QuadTree *qt, struct Sprite_t *s);
Error qn_insert(QuadTree *qt, QuadNode *qn, struct Sprite_t *s);
void qn_remove(QuadTree *qt, QuadNode *qn, struct Sprite_t *s);
Error qn_move(QuadTree *qt, QuadNode *qn, struct Sprite_t *s);
Error qn_subdivide(QuadTree *qt, QuadNode *qn);

/* Sorting */
typedef SDL_bool (*cmp)(struct Sprite_t*, struct Sprite_t*);
void insert_sort(struct Sprite_t* arr[], int n_sprites, cmp cmp_func);

/* Game state */
void save_game(void *data);
void load_game(void *data);

/* Media */
typedef struct MediaInfo {
	Uint32 offset, uncompressed_len, compressed_len;
} MediaInfo ;

/* Decompression */
Error decompress_media(const unsigned char *src, const int src_len, void **dst, size_t *dst_len);
/* General */
void print_no_mem(char *func_nm);

/***********/
/* image.c */
/***********/
struct Image_t;
Error load_image(struct Database_t *db_p, Uint16 img_idx, struct Image_t **img_pp);
Error reconstruct_colormap(SDL_Surface* tileset_surface, Uint16* tilemap, SDL_Surface* colormap_surface);

typedef struct Tileset {
	Uint16    n_tiles;
	Uint8     bpp;
	Uint16    mi_idx;     /* index into the media info table */
	SDL_Surface *surface_p;
	Uint32    last_modified_tm;
} Tileset;

typedef struct Tilemap {
	Uint16         ts_idx;      /* index into tileset table */
	Uint16        *tm_arry;
	Vector2        dimensions;  /* in units of tiles */
} Tilemap;

typedef struct ColorPalette {
	Uint8      n_colors;
	SDL_Color *cp_arry;
} ColorPalette;

typedef struct Image_t {
	Uint16        tm_idx;     /* index into tilemap table */
	Uint8         cp_idx;     /* index into color palette table */
	SDL_Surface  *surface_p;  /* ptr to loaded surface; results loaded tileset building img w/ tilemap & colors */
} Image;

typedef struct Position {
	Sint32 x;
	Sint32 y;
} Position;  /* Used by Motion to point at X & Y portion of SDL_Rect */
/* an array of shorts will be used to populate sprites. Sprites don't need references to the index of the image in the image table. */

typedef struct Strip {
	SDL_bool      repeat;
	Uint8         max_frame_idx;  /* awkward var name, because i don't want to subtract by one in animate(). */
	Uint16       *durations;
	SDL_Rect     *blit_coords;
} Strip;

/* instead of HTs, I'm going to enforce same-ordering & same population of everybody belonging to a sprite group. */
typedef struct Animation_t {
	SDL_Rect        blit_coords;
	Uint8           curr_frame_num;
	Uint32          curr_duration;
	Strip          *curr_strip_p;
} Animation;

Error inisprite(Sprite *s, const Vector2 *position);
void inimotion(Sprite *s, Motion *motion);
void inianimation(Sprite *s, Animation *animation);
void inireaction_sequence_group(Sprite *s, struct Scene_t *scene);

/***********/
/* Systems */
/***********/
typedef void (*IniSysFP)(System *sys_p);
typedef Error (*RunSysLoopFP)(void *componentArry, void);

Error sysNew();
void  sysDel;
Error sysReadChildOutbox();
Error sysReadInbox();
Error sysWriteOutbox();
Error sysNewChild();
	
typedef struct System {
	RunSysLoopFP runLoop;
	Message     *inbox;
	Message     *outbox;
	System      *childSysArry;
	void        *componentArry;
	dirEntry    *entityDirectory;  // a system reads inbox and delivers messages to its components. Multiple entities may send message to the same component. 
} System;

#endif

