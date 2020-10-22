#include "jb.h"

/* TODO: Consider quadtree approach to detecting sprites' collisions with camera's grid span. Same for collision detection. */

Error init_scene(Uint16 scene_id, Scene *scene) {
	Error stat = 0;
  Uint16 i, sprite_id = 0;


	return stat;
}

/* TODO: Have another boolean column in game media table that stores whether to keep for next scene. Then fly through and delete the rest. */
void switch_scene(Scene *scene) {

}

/* TODO: Fly through game media table and set everything to NULL that isn't. */
void kill_scene(Scene *scene) {
  Error stat = 0;
  Uint16 i, sprite_id = 0;
}

void bound_camera_in_bg_x(Camera *camera, SDL_Surface *bg_surf) {
  if (camera->rect.x < 0) {
    camera->rect.x = 0;
  }
  else if ((camera->rect.x + camera->rect.w) > bg_surf->w) {
    camera->rect.x = bg_surf->w - camera->rect.w;
  }
}

void bound_camera_in_bg_y(Camera *camera, SDL_Surface *bg_surf) {
  if (camera->rect.y < 0) {
    camera->rect.y = 0;
  }
  else if ((camera->rect.y + camera->rect.h) > bg_surf->h) {
    camera->rect.y = bg_surf->h - camera->rect.h;
  }
}

void bound_camera_in_bg(Camera *camera, SDL_Surface *bg_surf) {
  bound_camera_in_bg_x(camera, bg_surf);
  bound_camera_in_bg_y(camera, bg_surf);
}

void init_camera(Scene *scene, Vector2 *init_pos) {
  TblCameraRow *tbl_row;
  tbl_row = &tbl_camera[scene->type];
  scene->camera.motion = tbl_row->motion;
  scene->camera.react_seq_grp = tbl_row->react_seq_grp;
  /* If camera is bigger than bg, center bg. Otherwise, put cam in target position and keep bounded to BG. */
  if (scene->backgrounds[scene->bg_idx].w <= scene->camera.rect.w) {
    scene->camera.rect.x = (scene->camera.rect.w - scene ->backgrounds[scene->bg_idx].w) >> 1;
  }
  else {
    scene->camera.rect.x = init_pos->x;
    bound_camera_in_bg_x(&scene->camera, &scene->backgrounds[scene->bg_idx]);
  }
  if (scene->backgrounds[scene->bg_idx].h <= scene->camera.rect.h) {
    scene->camera.rect.y = (scene->camera.rect.h - scene ->backgrounds[scene->bg_idx].h) >> 1;
  }
  else {
    scene->camera.rect.y = init_pos->y;
    bound_camera_in_bg_y(&scene->camera, &scene->backgrounds[scene->bg_idx]);
  }
}

/* TODO: implement tracking offset from sprite */
/* motion is movement of sprite being tracked */
void camera_track_tgt(Scene *scene, MotionTranslation *motion) {
  if (motion != NULL) {
    scene->camera.tgt_offset.x = motion->vel.x * 5;   /* TODO: fine-tune this later */
    scene->camera.tgt_offset.y = motion->vel.y * 5;
  }
  scene->camera.motion->move(scene->camera.tgt, scene->camera.motion);
  bound_camera_in_bg(&scene->camera, &scene->backgrounds[scene->bg_idx]);
}

/*
  data types for storing scene objects

    images:           pointer (subscription action will access it directly)
    audio:            pointer (subscription action will access it directly)
    text:             pointer (subscription action will access it directly)
    reaction seqs:    pointer (subscription action will access it directly)
    subscriptions:    hash table (comm channels) of linked lists (sprites)
    sprites:          array... or quadtree
*/
