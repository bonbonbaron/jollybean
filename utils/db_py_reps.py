'''
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
'''

class Sprite:
    def __init__(self):
        self.onscreen = False
        self.reaction_bookmark = None
        self.id = 0
        self.type = 0
        self.species = 0
        self.surface = None
        self.rect = None
        self.blit_coords = None
        react_seq_grp = None
