/*
  Store global game data in a static structs! Since static is so flexible, you can divide up your game data however you like
  for fewer indirections.
*/
#ifndef GE_H
#define GE_H
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "SDL.h"
extern void ctrl_listen();
#define USE_LINKLISTS 0 
/************/
/***Errors***/
/************/
typedef Sint8 Error;
 
/***********/
/* General */
/***********/
SDL_bool game_over;
#define DEBUG 1
 
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
 
#define FPS 20
#define FRAME_DURATION (1000)/(FPS)  /* (1000 ms/s) * (s/N frames) = 1000/N milliseconds/frame */

static Uint16 sprite_id = 0;
struct Sprite_t;
struct Mobility_t;
struct Motion_t;
struct TblImgRow_t;
struct Animation_t;
struct Scene_t;
struct ReactSeqGrp_t;
 
/***********/
/* data.c  */
/***********/

typedef enum {
  MEDIA_IMG, MEDIA_SND, MEDIA_TXT
} media_type;

/* You also need a game perma-data table so you don't have to store metadata in file */

/* Defined in sprite's init() function. DON'T read this from a file! */
 
typedef struct Vector2 {
    Sint32 x, y;
} Vector2;
 
typedef struct Vector3 {
    Sint32 x, y, z;
} Vector3;

typedef struct ListNode {
  void *data;
  struct ListNode *next, *prev;
} ListNode;

/* Linked lists */
typedef struct LinkedList {
  ListNode *head;
  ListNode *tail;
} LinkedList;

Error init_list(LinkedList **ll);
void  delete_list(LinkedList *ll);
Error list_insert_tail(LinkedList *ll, void *data);
Error list_insert_head(LinkedList *ll, void *data);
void list_remove_node(LinkedList *ll, ListNode *node, ListNode **next, void **data);
ListNode* list_find_node(LinkedList *ll, void *data);

/* QuadTrees */
struct _QuadNode;
typedef struct _QuadNode {
  struct _QuadNode *nodes[4];
  Uint8 num_elems, depth;
  Vector2 tl_corner, br_corner;
  LinkedList *elems;
} QuadNode;

typedef struct QuadTree {
  QuadNode *root;
  Uint8 max_depth, max_num_elems;
} QuadTree;

Error qt_init(QuadTree **qt, Uint8 max_depth, Vector2 *tree_rect_dims, Uint8 max_num_elems);
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
void insert_sort(struct Sprite_t* arr[], int num_sprites, cmp cmp_func);

/* Game state */
void save_game(void *data);
void load_game(void *data);

/* Media */
typedef struct MediaInfo {
  Uint32 offset, uncompressed_len, compressed_len;
} MediaInfo ;

/* Decompression */
Error read_media(MediaInfo *media_info, unsigned char **src);
Error decompress_media(const unsigned char *src, const int src_len, void **dst, size_t *dst_len);
/* General */
void print_no_mem(char *func_nm);

/***********/
/* image.c */
/***********/
struct Image_t;
Error load_image(Uint16 img_idx, struct Image_t **img_pp);
Error reconstruct_colormap(SDL_Surface* tileset_surface, Uint16* tilemap, SDL_Surface* colormap_surface);

/***********/
/* sprite.c */
/***********/
/* Indexed by sprite->species */
/* Instead of implementing hash tables, just order all warriors' attacks the same. Then all walk_ups are 0, attack_1s are 4, jumps are 5, etc. */
typedef struct Tileset {
	Uint16    num_tiles;
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
	Uint8      num_colors;
	SDL_Color *cp_arry;
} ColorPalette;

typedef struct Image_t {
	Uint16        tm_idx;     /* index into tilemap table */
	Uint8         cp_idx;     /* index into color palette table */
  SDL_Surface  *surface_p;  /* ptr to loaded surface; results loaded tileset building img w/ tilemap & colors */
} Image;

typedef struct CollisionBounds {
	SDL_Rect n, s, e, w;
} CollisionBounds;

/* an array of shorts will be used to populate sprites. Sprites don't need references to the index of the image in the image table. */
typedef struct Sprite_t {
  SDL_Rect              rect;
  Image                *image_p;
} Sprite;

typedef struct LevelLayer {
	Tilemap              *tilemap_p;           /* For camera to know which part of the image to draw */
	Uint8                *logic_grid_arry;     /* Interaction with sprite */
	Tileset              *tileset_p;           /* Image that gets drawn on again and again based on where "camera" is */
} LevelLayer;

extern MediaInfo *tblMediaInfo;
extern Image *tblImage;
extern Tileset *tblTileset;
extern Tilemap *tblTilemap;
extern ColorPalette *tblColorPalette;

#define NUM_COMM_CHANNELS 12
#define MAX_NUM_SPRITES 10
#define MAX_NUM_BGS 3
#define MAX_NUM_EXITS 5
#define MAX_NUM_ACTIVITIES 20
/* Scene table row. Event-related stuff is absent, because it'll be loaded  */
typedef struct SceneTblRow {
	Uint16  *level_layer_arry;
  Uint16  *sprite_id_arry;
	Uint16  *position_arry;
} SceneTblRow; 

typedef struct Scene_t {
    Uint8         type;  /* determines the reactions and their mappings that sprites have */
    Sprite        sprite_arry[MAX_NUM_SPRITES]; 
    SDL_Surface   bg_arry[MAX_NUM_BGS];
    Uint8      ***coll_grid_ppp; /* Triple pointer: 1st pointer corresponds to Z-layer; 2nd, a 2D logic grid. Indexed by sprite.z, sprite.x >> 3, and sprite.y >> 3. */
    QuadTree      coll_quadtree;
} Scene;

/* Motion_t points to this func ptr, so having Motion_t parameter is circular. See if that's really correct. */
typedef SDL_bool (*MoveCallback)(Sprite *s, struct Motion_t *motion);

typedef struct MobilityTranslation {
  Vector2 vel, acc, vel_max;  //6
} MotionTranslation;

typedef struct MobilityLerping {
  Sprite* tgt;  //4
  Uint8 low_const, high_const; //2 /* LOL Uint8 instead of 16 just to make the union a perfect match in size. I'm obsessive. */
} MotionLerping;

/* It's fine that I use a union here, because I was going to waste bytes anyway not needing vx_max, etc. in lerping. */
typedef union {
  MotionTranslation mob_translate;
  MotionLerping mob_lerp;
} MobInfo;

typedef struct Motion_t {
  MoveCallback  move_callback;  //4  /* must come first so it can always be accessed the same way in generic move() function. */
  MobInfo       mob_info;
  QuadNode     *qn;  /* Only moving sprites get a place in the quad tree. The rest are assigned to the grid. */
} Motion;
  

typedef struct {
  Uint16 low, high;
} LerpConsts;

typedef struct TblMobileRow {
  Uint8 num_motions;
  Motion *motions;
} TblMobileRow;

typedef struct AnimationStrip {
  SDL_bool      repeat;
  Uint8         max_frame_idx;  /* awkward var name, because i don't want to subtract by one in animate(). */
  Uint16       *durations;
  SDL_Rect     *blit_coords;
} AnimationStrip;

/* Used to group together the set of animations for each sprite->type, like walk left, up, down, punch, etc. */
typedef struct TblAnimRow {
  Uint8           num_anim_strips;
  AnimationStrip *anim_strips;  /* Animation strips are indexed identically for all sprites of a certain type. idx in strip fn. */
} TblAnimRow;

/* instead of HTs, I'm going to enforce same-ordering & same population of everybody belonging to a sprite group. */
typedef struct Animation_t {
	SDL_Rect        blit_coords;
  Uint8           curr_frame_num;
  Uint32          curr_duration;
  AnimationStrip *curr_anim_strip;
  TblAnimRow     *anim_group;
} Animation;

Error init_sprite(Sprite *s, const Vector2 *position);
void init_motion(Sprite *s, Motion *motion);
void init_animation(Sprite *s, Animation *animation);
void init_reaction_sequence_group(Sprite *s, struct Scene_t *scene);
extern Image *tblImage;
extern TblMobileRow *tbl_mobile;
extern TblAnimRow   *tbl_anim;  /* table indexed by sprite species */

/***********/
/* scene.c */
/***********/

void init_camera(struct Scene_t *scene, Vector2 *init_pos);

typedef struct Exit {
  Uint8 id;
	Uint16 tile_id;
  Uint8 transition_type;
  Uint16 tgt_scene_idx;
} Exit;

Error init_scene(Uint16 scene_id, Scene *scene);
void  close_scene(Scene *scene);
Error init_surface(Sprite *s, Image *metadata);
Error init_audios(Scene *scene);
Error init_events(Scene *scene);
extern Scene *scene_zero;
extern SceneTblRow *scene_tbl;

 
#define IDX_OFFSET 0
#define IDX_LEN    0
extern char *DATA_FILE;
 
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

struct _System;

typedef struct {
	Uint8    num_children;
	Uint8    num_components;
	Uint8    num_signals;
	Uint8    num_callbacks;
	SDL_bool active;
	struct _System *parent_p;
	struct _System *children_1a;
	Uint8 **directory_2a;   /* 2D array mapping entities to systems' components. 0xFF means no component. Columns correspond to systems; rows to entities. */
	Signal *signal_1a;  /* active signals are always in front; stop iteration at dead signal */
	SysCallback *callbacks_1a;
} System;
	
Error ini_sys(System *sys_p);
Error del_sys(System *sys_p);
Error ini_sys_children(System *sys_p);
Error del_sys_children(System *sys_p);
Error get_entity_component(System *parent_p, Uint8 sys_id, Uint8 entity_id, void **comp_pp);  /* gets address of component; caller can cast it to whatever */




#endif
