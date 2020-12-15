from img import *
from py_defs import *
from jb_objects import TblSpriteRow
from glob import glob
from os.path import getmtime

'''
typedef struct TblImgRow_t {
  Uint8  bpp, num_colors, type;
  Uint16 rect_w, rect_h, surface_w, surface_h;
  MediaInfo *media_info;  /* This is a reference to where the tileset is stored in media.bin. */
  Uint16 *tilemap;
  SDL_Color *colors;
  SDL_Surface *ptr;  /* this is a pointer to a loaded surface. */
} TblImgRow;
============= DIRECTORY STRUCTURE ==============

    [DEV_DIR] <---- manualy defined in py_defs.py
        media.bin
        game/
          db/
            sprite/
              warrior/
                warrior.h
                warrior.c
        jb/
          db/
            db_media.c  (table of MediaInfo objects) sprite/
            sprite/
              db_sprite.c  
              db_anim.c
              db_mobile.c
        assets/
          sprite/
            raw PNG images

Length-varying attributes point to a specific C file stored in the game directory's specific subfolder. This means sprite's reaction group points to its type's subfolder. I will not make another table for them.

Therefore, db.py needs to know which class-based attributes are length-varying and which aren't. I'll figure this out later.
'''
# directories for assets and in-game databases
media_bin_fp = "%smedia.bin"%(DEV_DIR)
for asset_type, asset_file_ext in zip(ASSET_TYPES, ASSET_FILETYPES):
    asset_dir = "%sassets%s%s%s"%(ASSETS_DIR, asset_type, SEP)
    # Check DB for existing record.
    for fn in glob("%s*.%s"%(asset_dir, asset_file_ext)):
        asset_name = fn.split(".")[0].split("_")
        idx = query(asset_type,  
    # If a match is found, check the modifiction time.
    # If the mod time is newer than the one in the DB, run the media gen script. It'll process other DBs internally.
    # Rewrite the part of media.bin associated with this media object.
    # 
    

# Check DB for any existing record of this object.

# If it already exists, compare the resource's modified date with the modified date of the database's corresponding sprite child folder.

# If the object exists in the database, the entire media.bin file and all the DB MediaInfo members need to be rewritten.

# Call the appropriate bridge function no matter what.
#compress_sprite_imgs(directory, img_name);

