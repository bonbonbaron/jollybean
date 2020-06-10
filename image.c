#include "ge.h"

#define IMG_DEBUG 0
/* TODO: https://www.libsdl.org/projects/SDL_image/docs/SDL_image_8.html//SEC8 */
/* Current optimization probleM:
    What if sprites of different types share the same sound? 
    
*/
#define FLIP_NONE  (0)
#define FLIP_UD    (1 << 14)
#define FLIP_LR    (2 << 14)
#define FLIP_DIAG  (3 << 14)
/* TODO finish reconstruct_colormap() */
Error reconstruct_colormap(SDL_Surface *tileset_surface, Uint16 *tilemap, SDL_Surface *colormap_surface) {
  Error stat;
  Uint32 j, img_sz;
  Uint8 *p_colormap, *p_tileset, *p_tileset_origin;
  Uint8  row_idx;
  Uint16* p_tilemap_row_start;
  Uint32 num_tiles_per_row;
  Uint8 *p_colormap_end, *p_colormap_row_end;
  Uint16 orientation, *p_tilemap;
  img_sz = colormap_surface->w * colormap_surface->h;
  
  num_tiles_per_row = colormap_surface->w >> 3;
  p_tileset_origin = (Uint8*) tileset_surface->pixels;
  p_colormap       = (Uint8*) colormap_surface->pixels;
  stat = 0;
  p_colormap_end = (Uint8*) colormap_surface->pixels + (colormap_surface->pitch * colormap_surface->h);
#if IMG_DEBUG
  int k;
  Uint32 i
  Uint32 m;
  FILE* f1 = fopen("C:\\Users\\bonbo\\Desktop\\tileset_data.txt", "w");
  fprintf(f1, "************ Tileset **********\n");
  for (k = 0; k < (img_sz >> 6); k++) {
    fprintf(f1, "%8d ", k);
  }
  fprintf(f1, "\n");
  for (k = 0, p_tileset = (Uint8*) tileset_surface->pixels; k < tileset_surface->h * tileset_surface->pitch; k++) {
    if (k && !(k % TILE_W)) {
      fprintf(f1, " ");
    }
    if (k && !(k % tileset_surface->pitch)) {
      fprintf(f1, "\n");
    }
    fprintf(f1, "%d", *p_tileset++);
  }
  fprintf(f1, "\n\n************ Tilemap **********\n");
  for (m = 0, p_tilemap = img_metadata->tilemap; m < (img_sz >> 6); m++) {
    if (m && !(m % ((colormap_surface->w / TILE_W)))) {
      fprintf(f1, "\n");
    }
    fprintf(f1, "%5d ", *p_tilemap++);
  }
  fclose(f1);
#endif
  /* assuming TILE_W = TILE_H = 8, img_sz divided by 64 is the number of tiles. */
  row_idx = 0;
  p_tileset = (Uint8*)tileset_surface->pixels;

  p_colormap_row_end = (Uint8*) colormap_surface->pixels;
  p_tilemap = p_tilemap_row_start = tilemap;
  while (p_colormap < p_colormap_end) {
    p_colormap_row_end += colormap_surface->pitch;
    while (p_colormap < p_colormap_row_end) {
      orientation = *p_tilemap & 0xC000;
      if (orientation == FLIP_NONE) {  // tile's first pixel              // start of current row in unflipped order
        p_tileset = p_tileset_origin + ((*p_tilemap & 0x3FFF) * TILE_W) + ((row_idx & 7) * tileset_surface->pitch);
        for (j = 0; j < TILE_W; j++) {
          *p_colormap++ = *p_tileset++; 
        }
      }
      else if (orientation == FLIP_UD) {  // tile's first pixel           // start of current row in upside-down order
        p_tileset = p_tileset_origin + ((*p_tilemap & 0x3FFF) * TILE_W) + ((7 - (row_idx & 7)) * tileset_surface->pitch);
        for (j = 0; j < TILE_W; j++) {
          *p_colormap++ = *p_tileset++; 
        }
      }
      else if (orientation == FLIP_LR) {  // tile's first pixel           // end of current row
        p_tileset = p_tileset_origin + ((*p_tilemap & 0x3FFF) * TILE_W) + (((row_idx & 7) * tileset_surface->pitch) + 7);
        for (j = 0; j < TILE_W; j++) {
          *p_colormap++ = *p_tileset--; 
        }
      }
      else if (orientation == FLIP_DIAG) { // tile's first pixel          // end of current tile row in upside-down order
        p_tileset = p_tileset_origin + ((*p_tilemap & 0x3FFF) * TILE_W) + (((7 - (row_idx & 7)) * tileset_surface->pitch) + 7);
        for (j = 0; j < TILE_W; j++) {
          *p_colormap++ = *p_tileset--; 
        }
      }
      else {
        printf("[reconstruct_colormap] invalid orientation for img.\n");
        stat = 1;
        break;
      }
      p_tilemap++;
    }
    row_idx++;
    p_tilemap_row_start += (!(row_idx & 7) * num_tiles_per_row);
    p_tilemap = p_tilemap_row_start;
  }
#if IMG_DEBUG
  FILE* f;
  f = fopen("C:\\Users\\bonbo\\Desktop\\colormap_pixels.txt", "w");
  p_colormap = (Uint8*) colormap_surface->pixels;
  for (i = j = 0; i < img_sz; i++, j++) {
    if (j == 8) {
      j = 0;
      fprintf(f, " ");
    }
    if (i && !(i % colormap_surface->pitch)) {
      fprintf(f, "\n");
    }
    fprintf(f, "%d", *p_colormap++);
    //fwrite(" ", 1, 1, f);
  }
  fclose(f);
#endif
  return stat;
}

static Error load_tileset(TblSpriteRow *metadata, SDL_Surface **tileset_surface) {
  unsigned char* compressed_data;
  Error stat = 0;

  stat = read_media(metadata->media_info, &compressed_data);
  if (!stat) {
    *tileset_surface = SDL_CreateRGBSurfaceWithFormat(metadata->media_info->offset, metadata->surface_w, 
      metadata->surface_h, metadata->bpp, SDL_PIXELFORMAT_INDEX8);
    if (tileset_surface != NULL) {
      stat = decompress_media(compressed_data, metadata->media_info->compressed_len, 
        &(*tileset_surface)->pixels, (size_t*)&metadata->media_info->uncompressed_len);
      if (stat) {
        SDL_FreeSurface(*tileset_surface);
      }
    }
    else {
      stat = 1;
    }
  }

  return stat;
}

Error load_surface(TblSpriteRow *metadata, SDL_Surface **colormap) {
  Error stat = 0;
  SDL_Surface *tileset_surface = NULL;

  *colormap = NULL;

  /* Load tileset */
  stat = load_tileset(metadata, &tileset_surface);
  if (!stat && tileset_surface != NULL) {
    /* Piece tiles together to reconstruct colormap */
    *colormap = SDL_CreateRGBSurfaceWithFormat(metadata->media_info->offset, metadata->surface_w, 
      metadata->surface_h, metadata->bpp, SDL_PIXELFORMAT_INDEX8);
    if (*colormap != NULL) {
      stat = reconstruct_colormap(tileset_surface, metadata->tilemap, *colormap);
      if (stat) {
        SDL_FreeSurface(*colormap); 
      }
    }
    else {
      printf("[load_surface] Something went wrong with colormap loading.\n");
      stat = 1;
    }
  }
  else {
    printf("[load_surface] Something went wrong with tileset loading.\n");
    stat = 1;
  }
  /* Set palette */
  if (!stat) {
    stat = SDL_SetPaletteColors((*colormap)->format->palette, metadata->colors, 0, metadata->num_colors);
  }

  if (tileset_surface != NULL) {
    SDL_FreeSurface(tileset_surface);
  }
  if (stat) {
    SDL_FreeSurface(*colormap);
  }

  return stat;
}

void animate(Sprite *s, Animation *anim) {
  /* If frame's duration's been reached, either increment to next frame or loop back around if repeat. */
  if (++anim->curr_duration >= anim->curr_anim_strip->durations[anim->curr_frame_num]) {
    anim->curr_duration = 0;
    if (anim->curr_frame_num >= (anim->curr_anim_strip->max_frame_idx)) {
      if (anim->curr_anim_strip->repeat) {
        anim->curr_frame_num = 0;
      }
      else {
        anim->curr_frame_num++;
      }
    }
    s->blit_coords = anim->curr_anim_strip->blit_coords[anim->curr_frame_num];
  }
}

void move(Sprite *s, Motion *motion) {
  motion->move(s, motion);
}

void accelerate(Vector2 *velocity, Motion *motion) {
  velocity->x += motion->mob_info.mob_translate.acc.x;
  velocity->y += motion->mob_info.mob_translate.acc.y;
}

void limit_speed(Vector2 *velocity, Motion *motion) {
  if (velocity->x < -motion->mob_info.mob_translate.vel_max.x) {
    velocity->x = -motion->mob_info.mob_translate.vel_max.x;
  }
  else if (velocity->x > motion->mob_info.mob_translate.vel_max.x) {
    velocity->x = motion->mob_info.mob_translate.vel_max.x;
  }
  if (motion->mob_info.mob_translate.vel.y < -motion->mob_info.mob_translate.vel_max.y) {
    velocity->y = -motion->mob_info.mob_translate.vel_max.y;
  }
  else if (velocity->y > motion->mob_info.mob_translate.vel_max.y) {
    velocity->y = motion->mob_info.mob_translate.vel_max.y;
  }
}

void translate(Sprite *s, Vector2 *velocity, Motion *motion) {
  accelerate(velocity, motion);
  limit_speed(velocity, motion);
  s->rect.x += velocity->x;
  s->rect.y += velocity->y;
}


/*
  LERPING:
    Lerping is jumping a fixed percentage of the way from the sprite's position in the current frame to its destination:
    ==============
    Lerping @ 50%:
    ==============
    STEP #:             MOTION:
    1                   |*                 |
    2                   |        *         |
    3                   |            *     |
    4                   |               *  |
    5                   |                * |
    6                   |                 *|

    The implmentation I saw in a tutorial looks like this:
      function lerp (start, end, t) {
        return start * (1 - t) + end * t;
      }
    t is the percentage of the way through you want to progress each step.

    However, I'm not allowing floating point arithmetic in GE. I'm taking the integer arithmetic route:
	    def lerp(self, xf, yf):
		    self.x = ((self.lerp_low_const * self.x) + (self.lerp_high_const * xf)) >> 10
		    self.y = ((self.lerp_low_const * self.y) + (self.lerp_high_const * yf)) >> 10


*/
void lerp(Sprite *s, Vector2 *dest, Motion *motion) {
  s->rect.x = ((s->rect.x * motion->mob_info.mob_lerp.low_const) + (dest->x * motion->mob_info.mob_lerp.high_const)) >> 7;
  s->rect.y = ((s->rect.y * motion->mob_info.mob_lerp.low_const) + (dest->y * motion->mob_info.mob_lerp.high_const)) >> 7;
}
