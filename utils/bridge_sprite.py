from img import *
from py_defs import *
from jb_objects import TblSpriteRow

'''
class TblSpriteRow:
	def __init__(self):
		self.bpp = 0
		self.num_colors = 0
		self.type = 0
		self.rect_w = 0
		self.rect_h = 0
		self.surface_w = 0
		self.surface_h = 0
		self.media_info = None
		self.tilemap = None
		self.colors = None
		self.ptr = None
'''
# Enforce a certain directory structure of the game development ecosystem. All images must be saved in the appropriate resources child folder.

'''
============= DIRECTORY STRUCTURE ==============

    [GAME DEVELOPMENT ROOT FOLDER] (can be anywhere on your machine)
        media.bin
        db/
            ...
        resources/
            image/
                sprite/
                    raw PNG images
                background/  # What's the difference between a BG and map? We've determined that a FG or animated BG tile is a sprite. 
                    type grid (for collision grid)-- find a way to make this tiled-based. I think it can automate that.
                    tileset (tiled-friendly)-- can be generated in aseprite
                    tilemap (tiled file)
            audio/
                music/
                sound/
            text/
                menu/
                dialogue/

'''

# Verify that this is a sprite object by looking at the containing folder.

# Check DB for any existing record of this object.

# If it already exists, compare the resource's modified date with the modified date of the database's corresponding sprite child folder.

# If the object exists in the database, the entire media.bin file and all the DB MediaInfo members need to be rewritten.

# Call the appropriate bridge function no matter what.
compress_sprite_imgs(directory, img_name);






# TODO: Use this file as a template for your other bridges to accelerate development. In fact, just make one bridge. The differences are small enough
#       for this to serve as a switch among them.
