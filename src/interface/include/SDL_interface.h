#ifndef SDL_INTERFACE_H
#define SDL_INTERFACE_H

// ==============
// Rendering
// ==============

#include "SDL.h"
// Video
#define Color_ SDL_Color
#define Rect_ SDL_Rect
#define Texture_ SDL_Texture
#define Window_ SDL_Window
#define WindowTypeSz_ sizeof(SDL_Window)
#define Renderer_ struct SDL_Renderer
#define RendererTypeSz_ sizeof(SDL_Renderer)
#define Surface_ SDL_Surface
#define Palette_ SDL_Palette
#define copy_ SDL_RenderCopy
#define present_ SDL_RenderPresent
// Control
#define Event_ SDL_Event
#define pollEvent_ SDL_PollEvent
#define QUIT_ SDL_QUIT
#define quit_ SDL_Quit
#define KEY_UP_ SDL_KEYUP
#define KEY_DOWN_ SDL_KEYDOWN
#define getKeySymbol_(event_) event_.key.keysym.sym
#define KEY_a_ SDLK_a
#define KEY_b_ SDLK_b
#define KEY_c_ SDLK_c
#define KEY_d_ SDLK_d
#define KEY_e_ SDLK_e
#define KEY_f_ SDLK_f
#define KEY_g_ SDLK_g
#define KEY_h_ SDLK_h
#define KEY_i_ SDLK_i
#define KEY_j_ SDLK_j
#define KEY_k_ SDLK_k
#define KEY_l_ SDLK_l
#define KEY_m_ SDLK_m
#define KEY_n_ SDLK_n
#define KEY_o_ SDLK_o
#define KEY_p_ SDLK_p
#define KEY_q_ SDLK_q
#define KEY_r_ SDLK_r
#define KEY_s_ SDLK_s
#define KEY_t_ SDLK_t
#define KEY_u_ SDLK_u
#define KEY_v_ SDLK_v
#define KEY_w_ SDLK_w
#define KEY_x_ SDLK_x
#define KEY_y_ SDLK_y
#define KEY_z_ SDLK_z
#define KEY_SPACE_ SDLK_SPACE
#define KEY_LSHIFT_ SDLK_LSHIFT
#define KEY_ESCAPE_ SDLK_ESCAPE
// Events
#define EVENT_KEYUP_ SDL_KEYUP
#define EVENT_KEYDOWN_ SDL_KEYDOWN
#define EVENT_QUIT_ SDL_QUIT

#endif  // #ifndef SDL_INTERFACE_H

