#include "ge.h"

Error init_surface(Sprite *s, TblSpriteRow *tbl_row) {
  Error stat = 0;
  /* Share surfaces to spare RAM. TODO: use below commneted func to share colormaps w/ different palettes. */
  /* extern DECLSPEC SDL_Surface *SDLCALL SDL_DuplicateSurface(SDL_Surface * surface); */
  if (tbl_row->ptr != NULL) {
    s->surface = tbl_row->ptr;
  }
  else {
    stat = load_surface(tbl_row, &tbl_row->ptr);
    if (!stat) {
      s->surface = tbl_row->ptr;
    }
  }
  return stat;
}

Error init_sprite(Sprite *s, const Vector2 *pos) {
  TblSpriteRow *tbl_row = &tbl_sprite[s->species];
  
  s->id                 = sprite_id++;
  s->onscreen           = SDL_FALSE;
  s->rect.x             = pos->x;
  s->rect.y             = pos->y;
  s->rect.w             = tbl_row->rect_w;
  s->rect.h             = tbl_row->rect_h;
  s->reaction_bookmark  = 0;  
  return init_surface(s, tbl_row);
}

void init_motion(Sprite *s, Motion *mobility) {
  TblMobileRow *tbl_row = &tbl_mobile[s->species];
  memcpy(mobility, tbl_row, sizeof(Motion));  /* memcpy, not point, bc vels change for each individual */
}

void init_animation(Sprite *s, Animation *anim) {
  TblAnimRow *tbl_row = &tbl_anim[s->species];  
  
  anim->curr_frame_num = 0;
  anim->curr_duration = 0;
  anim->curr_anim_strip = NULL;
}

/* I may just need to init the pointer and not store it in the sprite. */
void init_reaction_sequence_group(Sprite *s, Scene *scene) {
  Uint8 i;
  TblReactRow *tbl_row;
  
  tbl_row = &tbl_react[s->type];

  if (tbl_row->ptr != NULL) {
    s->react_seq_grp = tbl_row->ptr;
  }
  else {
    if (tbl_row->num_scene_sprite_options == 1) {
      tbl_row->ptr = tbl_row->react_seq_grp_options[0].react_seq_grp;
    }
    for (i = 0; i < tbl_row->num_scene_sprite_options; i++) {
      if (tbl_row->react_seq_grp_options->sprite_type == s->type && 
          tbl_row->react_seq_grp_options->scene_type == scene->type) {
        tbl_row->ptr = tbl_row->react_seq_grp_options[i].react_seq_grp;
      }
    }
    s->react_seq_grp = tbl_row->ptr;
  }
}

/* TODO: This is waiting on the quadtree and grid to both be completed. These two items in turn are waiting on the gap between the level editor and game engine to be bridged. */
void detect_collisions() {

}
