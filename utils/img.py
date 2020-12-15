# TODO: change color maps to always-increasing integer order. That way it'll be easier to share colormaps.
from time import time as t
import json
from collections import deque
import cv2, os
import numpy as np
import png
from struct import pack, unpack
from os.path import getsize
from os import sep
from glob import glob
IDX_R                 =  0
IDX_G                 =  1
IDX_B                 =  2
TILE_WIDTH            =  8
TILE_HEIGHT           =  8
NBR_COLOR_CHANNELS    =  3
TILE_SIZE = TILE_WIDTH * TILE_HEIGHT * NBR_COLOR_CHANNELS
PNG_BITDEPTH_LOCATION = 24
PNG_BITDEPTH_DATA_SZ  = 1
PNG_IDATA_LOCATION    = PNG_BITDEPTH_LOCATION + 8
PNG_IEND_LENGTH       = 8
PNG_SIGNATURE         = bytearray([137, 80, 78, 71, 13, 10, 26, 10])
PNG_IHDR_START        = bytearray([0, 0, 0, 13, 73, 72, 68, 82])
PNG_PLTE_CHUNK_TYPE   = bytearray([80, 76, 84, 69])
PNG_IDAT_CHUNK_TYPE   = bytearray([73, 68, 65, 84])  
PNG_IEND              = bytearray([0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130])
PNG_IDAT_START        = bytearray("IDAT".encode())
PNG_IDAT_END          = bytearray("IEND".encode())
IMG_DIR_IDX_NAME      = 0
IMG_DIR_IDX_LOCATION  = 1
IMG_DIR_IDX_NBR_TILES = 2
 
BYTEORDER = "big"  # since that's the order files are written in
 
FLIP_NONE = 0
FLIP_UD   = 1 << 14
FLIP_LR   = 2 << 14
FLIP_DIAG = 3 << 14
 
 
###########################################
class AnimSequence:
    def __init__(self, blit_coords, frame_order, durations, repeat):
        self.blit_coords = blit_coords
        self.frame_order = frame_order
        self.durations = durations
        self.repeat = repeat
###########################################
#def preview(img):
#    cv2.imshow("preview", img)
#    cv2.waitKey(0)
#    cv2.destroyAllWindows()
 
###########################################
def flip_none(np_array):
    return np_array
flip_ud = np.flipud
flip_lr = np.fliplr
def flip_diag(np_array):
    return np_array[::-1, ::-1, :]
 
flip_funcs = {FLIP_NONE: flip_none, FLIP_UD: flip_ud, FLIP_LR: flip_lr, FLIP_DIAG: flip_diag}
 
###########################################
# RLE format: one unsigned byte for count, one signed byte for difference between pixel values
def rle(tile):
    assert len(tile.shape) >= 2 and tile.shape[0] == 8 and tile.shape[1] == 8
    flat_tile = tile.flatten()
    rle_tile = []
    curr_color = flat_tile[0]
    counter = 0
    for i in range(1, 64):
        if flat_tile[i] == curr_color:
            counter += 1
        else:
            rle_tile.append(counter)
            rle_tile.append(curr_color)
            curr_color = flat_tile[i]
            counter = 0
    return rle_tile
 
# Highest 2 bits of tileset element = orientation. Every bit after that (14 bits: 0 - 32768) represents tile ID, which is plenty!
FLIP_NONE = 0
FLIP_UD   = 1 << 14
FLIP_LR   = 2 << 14
FLIP_DIAG = 3 << 14
###########################################
def map_tiles(img):
    NBR_TILES_HIGH = img.shape[0] >> 3
    NBR_TILES_WIDE = img.shape[1] >> 3
     
    tile_label = 0
    tiles_labelled = np.zeros((NBR_TILES_HIGH, NBR_TILES_WIDE))
    tileset = []
    tilemap = np.zeros((NBR_TILES_HIGH, NBR_TILES_WIDE), np.uint16)
    # Find distinct tiles and write an index-mapping to them.
    for x in range(NBR_TILES_HIGH):
        for y in range(NBR_TILES_WIDE):
            if not tiles_labelled[x, y]:
                curr_tile = img[x << 3 : (x + 1) << 3, y << 3 : (y + 1) << 3]
                tileset.append(curr_tile.astype(np.uint8))
                tiles_labelled[x, y] = True
                tilemap[x, y] = tile_label
                # Note: In C I can avoid copies by reading tiles in different directions over the same image. I won't bother in the Python prototype.
                for x2 in range(NBR_TILES_HIGH):
                    for y2 in range(NBR_TILES_WIDE):
                        if (x2 != x or y2 != y) and not tiles_labelled[x2, y2]:
                            next_tile = img[x2 << 3 : (x2 + 1) << 3, y2 << 3 : (y2 + 1) << 3]
                            if np.array_equal(curr_tile, next_tile):
                                tilemap[x2, y2] = tile_label | FLIP_NONE
                                tiles_labelled[x2, y2] = True
                            elif np.array_equal(curr_tile, np.flip(next_tile, 0)):
                                tilemap[x2, y2] = tile_label | FLIP_UD
                                tiles_labelled[x2, y2] = True
                            elif np.array_equal(curr_tile, np.flip(next_tile, 1)):
                                tilemap[x2, y2] = tile_label | FLIP_LR
                                tiles_labelled[x2, y2] = True
                            elif np.array_equal(curr_tile, next_tile[::-1, ::-1]):
                                tilemap[x2, y2] = tile_label | FLIP_DIAG
                                tiles_labelled[x2, y2] = True
                tile_label += 1
    # Images with no repeatable parts are treated as vanilla PNG files without tiling.
    if tile_label == NBR_TILES_HIGH * NBR_TILES_WIDE:
        tilemap = None
        tileset = None
    else:
        tilemap = tilemap.reshape((NBR_TILES_HIGH, NBR_TILES_WIDE))
    # print(tilemap)
    return tileset, tilemap
     
 
###########################################
def  get_keymap_psn(filename):
    underscore_position = 0
    for i in range(len(filename)):
        if filename[i] == "_":
            underscore_position = i + 1
            break
    return underscore_position
     
def throw_underscore_error(fn):
    print("[ae] AE couldn't find two underscores in %s. Fix this file and try again. %s"%(fn, AE_FILENAME_FORMAT))
    quit()
     
def get_key_mapping_str(fn, underscore_psns):
    if len(underscore_psns) != 1:
        throw_underscore_error(fn)
    return fn[underscore_psns[0] + 1 :]
 
# ignores any key mappings not in the below set
def get_key_mapping(fn):
    keymap_psn = get_keymap_psn(fn)
    km_str = fn[keymap_psn:-5].upper()
    KM = {"U":     CTLR_U,
          "D":     CTLR_D,
          "L":     CTLR_L,
          "R":     CTLR_R,
          "A":     CTLR_A,
          "B":     CTLR_B,
          "X":     CTLR_X,
          "S":     CTLR_START}
    b = 0  # byte
    for s in km_str:
        km = KM.get(s)
        if km is not None:
            b |= km
    return b
     
def create_anim_map(img_name):
    from blitcoords import BlitCoords
    anim_sequence = None
    anim_map = {}
    files = glob(DIR_SPRITES + img_name + "*.json")
    if len(files) == 0:
        print("[scene] Couldn't find any JSON files for %s in %s! Is the underscore in PNG name(s) supposed to be there? Quitting...\n"%(img_name, DIR_SPRITES))
        quit()
    # for each sequence of this sprite
    for fp in files:
        j = json.load(open(fp))
        frames = j["frames"]
        if len(frames) < 2:
            print("[scene] found less than 2 frames for %s in %s. Are you sure this sprite is animated?"%(img_name, f))
            quit()
        blit_coords = []
        durations = []
        # for each frame in this sequence
        for frame in frames:
            frame_data = frames[frame]
            json_coords = frame_data["frame"]
            x = json_coords["x"]
            y = json_coords["y"]
            w = json_coords["w"]
            h = json_coords["h"]
            # If no repeat and single-frame, then there you go.
            blit_coords.append([x, y, w, h])
            t = int(FPS * (frame_data["duration"] / 1000))  # in units of frames
            durations.append(t)
        # Get key mapping and frame order
        key_mapping = get_key_mapping(fp)
        frame_order = [i for i in range(len(blit_coords))]
        repeat = key_mapping & CTLR_DIR  # assuming only motion repeats animation
        anim_sequence = AnimSequence(blit_coords, frame_order, durations, repeat)
        # Finally, map some key(s) to an animation sequence!
        anim_map[key_mapping] = anim_sequence
    return anim_map
 
def remove_dupe_frames(anim_map, img):
    from collections import deque
    deletion_indices = deque()
    # For each key-mapped animation sequence...
    for s, k in zip(list(anim_map.values()), list(anim_map.keys())):
        for i in range(len(s.blit_coords) - 1):
            # print(s.blit_coords[i].x)
            # print(s.blit_coords[i].y)
            # print(s.blit_coords[i].w)
            # print(s.blit_coords[i].h)
            for j in range(i + 1, len(s.blit_coords)):
                x1 = s.blit_coords[i][0] #.x
                y1 = s.blit_coords[i][1] #.y
                w1 = s.blit_coords[i][2] #.w
                h1 = s.blit_coords[i][3] #.h
                x2 = s.blit_coords[j][0] #.x
                y2 = s.blit_coords[j][1] #.y
                w2 = s.blit_coords[j][2] #.w
                h2 = s.blit_coords[j][3] #.h
                img1 = img[:, x1 : x1 + w1, :]
                img2 = img[:, x2 : x2 + w2, :]
                # preview(img1)
                if np.equal(img1, img2).all():
                    print("Found dupe frame")
                    deletion_indices.appendleft(j)  # include queue library
                    s.frame_order[j] = i
        # ... Remove duplicate frames in this key-mapped animation sequence.
        bc = s.blit_coords
        for di in deletion_indices:
            print("deleting")
            x = s.blit_coords[di][0].x
            y = s.blit_coords[di][1].y
            w = s.blit_coords[di][2].w
            h = s.blit_coords[di][3].h
            img = np.hstack(img[:, :x, :], img[:, x + w:, :])
            del(s.blit_coords[di])
            del(s.durations[di])
            frame_order
 
        anim_map[k] = s
    # preview(img)
    # quit()
    return img, anim_map
 
###########################################
# returns array of indices
def rank_elems(array):
    L = len(array)
    result = np.zeros(L, np.uint8)
    for i in range(L):
        rank = 0
        for j in range(L):
            if array[j] < array[i]:
                rank += 1  # higher ranks for brighter pixels
        result[i] = rank
    return result
     
def elems_equal(a, b):
    if a[0] == b[0] and a[1] == b[1] and a[2] == b[2]:
        return True
    return False
     
# Checks for existence of multi-element sub-array in container array
###########################################
def contains(container, elem):
    for i in container:
        if elems_equal(i, elem):
            return True
    return False
 
###########################################
def sort_colors(color_palette, brightness_ranks):
    result = []  # assume array is 1D
    # In the case of same-brightness colors, ensure definite order for decompression: R, G, & B.
    for i in range(0, len(brightness_ranks) - 1):
        if brightness_ranks[i] == brightness_ranks[i + 1]:
            for color_channel in range(3):
                if color_palette[i][color_channel] > color_palette[i + 1][color_channel]:
                    brightness_ranks[i] += 1
                    # Bump all the higher ranks up now that this one has increased.
                    for j in range(len(brightness_ranks)):  
                        if j != i:
                            if brightness_ranks[j] >= brightness_ranks[i]:
                                brightness_ranks[j] += 1
                    break 
    # (If all channels are the same then who cares if ranks are the same?)
    # Ensure there are no identical ranks for DIFFERENT colors
    for i in range(len(brightness_ranks)):
        for j in range(len(brightness_ranks)):
            if i != j:
                if brightness_ranks[i] == brightness_ranks[j] and color_palette[i] != color_palette[j]:
                    print("ERROR: Two brightness ranks have the same value: %d."%(brightness_ranks))
                    print("Brightness ranks:")
                    print(brightness_ranks)
                    print("Color palette:")
                    print(color_palette)
                    quit()
    # Finally, sort the colors.
    for i in brightness_ranks:
        result.append(color_palette[brightness_ranks[i]])
    return result
 
###########################################
def get_color_palette(img):
    color_palette = []
    for x in range(img.shape[0]):
        for y in range(img.shape[1]):
            pixel = img[x, y]
            if not contains(color_palette, pixel):
                color_palette.append(pixel)
    return color_palette
# Gray-out sprite colors for data compression. Enforce SNES standards of 8 max colors/sprite.
# Color brightness formula:
#   https://stackoverflow.com/questions/59628/formula-to-determine-brightness-of-rgb-color
###########################################
def gray_out_img(img, color_palette):
    brightness        = []
    nbr_mask_size = 0
     
    NBR_PIXELS_HIGH = img.shape[0]
    NBR_PIXELS_WIDE = img.shape[1]
 
 
    # Get unique pixel values to make color palette
    color_palette = get_color_palette(img)
    for cp in color_palette:
        r = cp[0]
        g = cp[1]
        b = cp[2]
        brightness.append((0.2126*r + 0.7152*g + 0.0722*b))
                 
    # I only want to support up to 8 colors like the SNES.
    # for c in color_palette:
        # print(c)
    # print(len(color_palette))
    color_palette = color_palette[:8]
    brightness = brightness[:8]
    assert len(color_palette) <= 8  
     
    brightness_ranks = rank_elems(brightness)  # multiple colors could share a rank.
    max_brightness = max(brightness_ranks)
    # print(max_brightness)
    sorted_color_palette = sort_colors(color_palette, brightness_ranks)
    # Map each value in color image to its rank
    gray_img = (255 * np.ones((NBR_PIXELS_HIGH, NBR_PIXELS_WIDE))).astype(np.uint8)
    len_color_palette = len(color_palette)
    # print(len_color_palette)
    for x in range(img.shape[0]):
        for y in range(img.shape[1]):
            pixel = img[x, y]
            for i in range(len_color_palette):
                if elems_equal(sorted_color_palette[i], pixel):
                    gray_img[x, y] = i
            if gray_img[x, y] == 255:
                gray_img[x, y] = max_brightness
             
    #cv2.imwrite("result.png", gray_img)
    color_palette = sorted_color_palette
    return gray_img
     
class ImageInfo:
	def __init__(self, length, w, h, bpp, is_tiled, idat):
		self.length = len(idat)
		self.w = final_tileset.shape[1]
		self.h = final_tileset.shape[0]
		self.bpp = bpp
		self.num_tiles = 0
###########################################
# current problem: how to get anim_offset correlated with this
def compress_img_and_anim(img, animated=False):
    compressed_tileset = []
    color_palette      = []
     
 
    assert img.shape[0] % 8 == 0
    assert img.shape[1] % 8 == 0   
    nbr_tiles_high = img.shape[0] >> 3
    nbr_tiles_wide = img.shape[1] >> 3
    color_palette = get_color_palette(img)
     
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    #~~~~~~~~~~~~~~~ Tilemap ~~~~~~~~~~~~~~~~#
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
     
    lcp = len(color_palette)
    if lcp <= 2:
        bpp = 1
    elif lcp <= 4:
        bpp = 2
    else:  # if lcp <= 16
        bpp = 4
     
    _ = []
    if animated:
        print("removing dupe frames")
        anim_map = create_anim_map(sprite.img_name)
        remove_dupe_frames(anim_map, img)
    gray_img = gray_out_img(img, _)
    # RIGHT HERE. Before mapping tiles, look up anim JSON data and cut out parts of image that are identical.
    tileset, tilemap = map_tiles(gray_img)
     
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    #~~~~~~~~~~~~ Tileset ~~~~~~~~~~~~~~~~~~~#
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    # This divides up the tileset into one 2D array and splits it up into rows of pixels. This is how PNG Writer wants to write it.
    max_idx = np.max(tileset)
    if max_idx == 1:
        bytes_per_tile = 8
        bitdepth = 1
    elif max_idx < 4:  # 2 bits/pixel * 64 pixels/tile * 1 byte/8bits = 8
        bytes_per_tile = 16
        bitdepth = 2
    else: # max_idx < 8:  # 4 bits/pixel * 64 pixels/tile * 1 byte/8bits = 32s
        bytes_per_tile = 32
        bitdepth = 4
    final_tileset = np.zeros((8, 8 * len(tileset)), np.uint8)
    # 40 tiles * 64 pixels/tile * 4 bits/pixel * 1 byte/8 bits
    for i in range(len_tileset):
        final_tileset[:, i*8: (i+1)*8] = tileset[i]
     
    w = final_tileset.shape[1]
    h = final_tileset.shape[0]
 
    writer = png.Writer(w, h, bitdepth=bitdepth, palette=color_palette)
 
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    #~~~~~~~~~ Extract PNG's IDAT ~~~~~~~~~~~#
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    f = open("temp.png", "wb")
    writer.write(f, final_tileset)
    f.close()
    f = open("temp.png", "rb")
    png_data = f.read()
    idat_start = png_data.index(PNG_IDAT_START) + 4  # We don't include "IDAT." We're BYTE MISERS!
    idat_end   = png_data.index(PNG_IDAT_END) - 4    # 4-byte length precedes IEND
    idat = png_data[idat_start : idat_end]
     
    image_data = [0, 0, 0, 0, 0, 0]
    image_data[IDX_LEN]      = len(idat)
    image_data[IDX_W]        = final_tileset.shape[1]
    image_data[IDX_H]        = final_tileset.shape[0]
    image_data[IDX_BPP]      = bitdepth
    image_data[IDX_IS_TILED] = nbr_tiles > 1
     
    if tilemap is not None:
        tm_metadata = [0, 0, 0, 0]
        tm_metadata[IDX_LEN] = 2 * tilemap.shape[0] * tilemap.shape[1]  # USHORT data type
        tm_metadata[IDX_W]   = tilemap.shape[1]
        tm_metadata[IDX_H]   = tilemap.shape[0]
        temp = list(tilemap.flatten())
        temp = [int(t).to_bytes(2, BYTEORDER) for t in temp]
        tm_array = bytearray()
        for t in temp:
            tm_array.extend(t)
    else:
        tm_array = None
        tm_metadata  = None
     
    return idat, image_data, tm_array, tm_metadata
 
###########################################
# Updates both image dictionary and global data file where it's stored
def compress_sprite_imgs(directory, img_name):
	# Find all files belonging to sprite
	img_fps = glob("%s/%s*.png"%(directory, img_name))
	# Concatenate if animated
	if len(img_fps) == 0:
		print("No images found for %s in \"%s\"! If there are, they're supposed to be PNG format."%(img_name, directory))
		quit()
	animated = "_" in img_fps[0]
	for img_fp in img_fps:
            fn = img_fp.split(".")[0]
            img = cv2.imread(img_fps[0])
            png_idat, image_data, tm_array, tm_metadata = compress_img_and_anim(img, animated)
            f = open("./%.bin"%(fn), "wb")
            f.write(png_idat)
            f.close()
            if tm_array is not None and tm_metadata is not None:
                f = open("%s_tm_array.bin"%(fn), "wb")
                f.write(tm_array)
                f.close()
                f = open("%s_tm_metadata.bin"%(fn), "wb")
                f.write(tm_metadata)
                f.close()

