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
struct TblSpriteRow_t;
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
struct _TblSpriteRow;
Error load_surface(struct _TblSpriteRow *tbl_sprite_Row, SDL_Surface **colormap);
Error reconstruct_colormap(SDL_Surface* tileset_surface, Uint16* tilemap, SDL_Surface* colormap_surface);

/***********/
/* sprite.c */
/***********/
#define FLG_SPRITE_ANIMATED (1)
#define FLG_SPRITE_MOBILE   (2)
#define FLG_SPRITE_AUDIBLE  (4)
/* react_func_ptr() returns 1 if it's okay to proceed to the next action and increment bookmark, 0 otherwise. */

/* Indexed by sprite->species */
/* Instead of implementing hash tables, just order all warriors' attacks the same. Then all walk_ups are 0, attack_1s are 4, jumps are 5, etc. */
typedef struct _TblSpriteRow {
  Uint8  bpp, num_colors, type;
  Uint16 rect_w, rect_h, surface_w, surface_h;
  MediaInfo *media_info;
  Uint16 *tilemap;
  SDL_Color *colors;
  SDL_Surface *ptr;
} TblSpriteRow;

typedef struct Sprite_t {
  SDL_bool              onscreen;
  Uint8                 reaction_bookmark;  /* Bc if sprite doesn't do anything, why is it not just a background tile? */
  Uint16                id;       /* e.g. Goomba #1, Goomba #2, Bowser #1, etc. */
  Uint8                 type;     /* e.g. enemy, fire, water, poisFon, spring, etc. */
  Uint16                species;  /* e.g. Goomba, old man in red shirt, Bowser, etc. */
  SDL_Surface          *surface;
  SDL_Rect              rect, blit_coords;
  struct ReactSeqGrp_t *react_seq_grp;
} Sprite;

/* Motion_t points to this func ptr, so having Motion_t parameter is circular. See if that's really correct. */
typedef SDL_bool (*move_func_ptr)(Sprite *s, struct Motion_t *motion);

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
  move_func_ptr move;  //4  /* must come first so it can always be accessed the same way in generic move() function. */
  MobInfo mob_info;
  QuadNode *qn;  /* Only moving sprites get a place in the quad tree. The rest are assigned to the grid. */
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
  Uint8           curr_frame_num;
  Uint32          curr_duration;
  AnimationStrip *curr_anim_strip;
  TblAnimRow     *anim_group;
} Animation;

/* Animated and/or Mobile Sprites */
typedef struct AnimatedSprite {
  Sprite *sprite;
  Animation *animation;
} AnimatedSprite;


typedef struct AnimobSprite {
  Sprite *sprite;
  Animation *animation;
  Vector2 *velocity;
} AnimobSprite;

typedef struct {
	Sprite   *sprite_p;  /* actor sprite */
	void     *tgt_p;     /* acted upon sprite */
	Uint16    val;       /* in case this action requires a value */
	Scene    *scene_p;   /* in case this action requires more information from the scene */
typedef SDL_bool (*react_func_ptr)(Sprite*, Sprite*, struct Scene_t*);  /* acting sprite, tgt (if any), scene (if necessary) */
/* (predefined) A sequence of reactions a sprite calls in response to a trigger */
typedef struct ReactSeq {
  Uint8      priority;     /* To interrupt sequences of lower priorities. 255 = lowest priority. */
  Uint8 num_reactions;
  SDL_bool repeat;
  react_func_ptr  *react_func_ptr; /* (args TBD) */
} ReactSeq;

/* A group of sequences, e.g. reseq_pedestrian would have walk_up/down/etc. based on timers */
/* Seems to me all we have to do is index a staggered array. The triggers member is not needed. */
typedef struct ReactSeqGrp_t {
  Uint8      num_react_seqs;  /* used to prevent illegal access error */
  Uint8     *triggers;     /* needed for subscribing each reaction sequence to a channel */
  ReactSeq **react_seq_ptr; /* Double ptr = array of ptrs to reaction sequences */
} ReactSeqGrp;

/* Points to the reaction sequence group and its seq available to the sprite for this scene type. */
/* Seems to me all we have to do is index a staggered array. The triggers member is not needed. */
typedef struct ReactSeqGrpOption {
  Uint8        sprite_type, scene_type;
  ReactSeqGrp *react_seq_grp;
} ReactSeqGrpOption;

typedef struct TblReactRow {
  ReactSeqGrp        *ptr;
  Uint8               num_react_seq_grp_options;  /* This'll apply for camera too */
  ReactSeqGrpOption  *react_seq_grp_options;  /* array of ReactSeqGrpOptions */
} TblReactRow;
 
Error init_sprite(Sprite *s, const Vector2 *position);
void init_motion(Sprite *s, Motion *motion);
void init_animation(Sprite *s, Animation *animation);
void init_reaction_sequence_group(Sprite *s, struct Scene_t *scene);
extern TblSpriteRow *tbl_sprite;
extern TblMobileRow *tbl_mobile;
extern TblAnimRow   *tbl_anim;  /* table indexed by sprite species */
extern TblReactRow  *tbl_react;

/***********/
/* scene.c */
/***********/

typedef struct {
  SDL_Rect     rect;
  Sprite     *tgt;
  Vector2    tgt_offset;   /* for when tgt sprite is running fast and we want to track a point front of it */
  ReactSeqGrp *react_seq_grp;
  Motion      *motion;
} Camera;


/* Row indexed by Uint8 scene_type */
typedef struct TblCameraRow {
  Motion      *motion;
  Uint8       *sprite_tgt;
  ReactSeqGrp *react_seq_grp;
} TblCameraRow;
extern TblCameraRow *tbl_camera;

void init_camera(struct Scene_t *scene, Vector2 *init_pos);

typedef struct Exit {
  Uint8 id;
  Uint8 transition_type;
  Uint16 tgt_scene_idx;
} Exit;

typedef struct _CommChannel;
#define NUM_COMM_CHANNELS 12
typedef struct Scene_t {
    Sprite      **sprites;  /* If there's a way to make background images sprites too, do iiiiiit!! */
    Uint16        num_sprites;
    Uint8         type;  /* determines the reactions and their mappings that sprites have */
    Uint8         bg_idx; /* bg layer that camera moves along */
    Exit         *exits; /* just point to the array in the table */
    SDL_Surface  *backgrounds;
    LinkedList    messages;
    Camera        camera;
    QuadTree      coll_quadtree;
    Uint8      ***coll_grid; /* Triple pointer: 1st pointer corresponds to Z-layer; 2nd, a 2D logic grid. Indexed by sprite.z, sprite.x >> 3, and sprite.y >> 3. */
		_CommChannel comm_channels[NUM_COMM_CHANNELS];  /* TODO explain why i picked 12... i don't remember now lol */
} Scene;

/* Let's just play around for a second. I want to see if I can come up with a good Scene table scheme. */
typedef struct SceneTblRow {
  Uint8    num_sprites;
  Uint8    num_exits;
  Uint8    num_songs;
  Uint8    song_ids;  /* TODO: implement songs so you can put them in the Scene struct */
  Uint8    num_sounds;
  Uint8    sound_ids;  /* TODO: implement sound so you can put it in the Scene struct */
  Uint16  *sprite_ids;
  MediaInfo media_info;
  /* If file I/O is the slowest part of the loading process, then let's load as much as we can at once: song ID, coll grid, tileset IDs, tilemaps, exits, etc. Some things need to be dynamically populated, like the QT, etc. */
  Exit    *exits; /* This gets indexed by colliding into a tile whose type contains this index. */
} SceneTblRow; 


/* How did I intend on doing scene-loading? I don't remember quite how I was planning on going about that. ID sounds like I was going to index into a table. So then what would this table have? Would everything already be allocated? I remember envisioning a file for each scene; a C file. And I remember seeing (in my head) a series of initializers being called, like michael = new_michael(), stuff like that. But if there were ten Goombas, even if I've already worked out all the media one-time-only loading, I'd still have to write *twenty* new_goomba() calls. So maybe something more repeatable. init_sprite(goomba_species_id, pos1, pos2, pos3...). But that ellipsis prevents from adding in orientation data too. So I see no way around having to write new_sprite(goomba_type, pos_vec2, orientation Uint8) twenty times. Maybe I'll come up with a better design someday, but that's the best I've got for now.
 *
 * */
Error init_scene(Uint16 scene_id, Scene *scene);
void  close_scene(Scene *scene);
Error init_surface(Sprite *s, TblSpriteRow *metadata);
Error init_audios(Scene *scene);
Error init_events(Scene *scene);
extern Scene *scene_zero;
extern SceneTblRow *scene_tbl;

/************/
/* events.c */
/************/

/* Events are for non-collision-based activities. */
typedef struct Message {
	Uint8 to_type;
	Uint8 to_channel;
	Uint8 msg;  /* e.g. 13 = "You are being attacked." */
} Message;

typedef struct _CommChannel {
  SDL_bool       signal_on;
  LinkedList    *subscribers;
} CommChannel;

typedef struct Activity {  /* stack var */
  Sprite *tgt;
  ReactSeq *current_reaction_sequence;
} Activity;

typedef struct {
  SDL_bool busy;
  Sprite *s;
  Activity activity;
} Subscriber;


Error subscribe(Sprite *s);
Error unsubscribe(Sprite *s, Uint8 channel);
Error trigger_reaction(Subscriber *subscriber, Uint8 signal, LinkedList *active_subscribers);
void broadcast(Uint8 signal );
void cancel(Uint8 signal);
 
/********/
/* LUTS */
/********/
 
 
#define IDX_OFFSET 0
#define IDX_LEN    0
extern char *DATA_FILE;
 
 
#endif
