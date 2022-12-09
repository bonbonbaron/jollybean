import json
from time import time as t
from collections import deque
import os
import numpy as np
import png
from struct import pack, unpack
from os.path import getsize
from os import sep
from glob import glob
import sys
from os import path
from py_defs import *
import cv2

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
 
BYTEORDER = "little"  # since that's the order files are written in
 
FLIP_NONE = 0
FLIP_UD   = 1 << 14
FLIP_LR   = 2 << 14
FLIP_DIAG = 3 << 14
FLIP_MASK = FLIP_DIAG

class Inflatable:
    def __init__(self, name, cmpLen, decompLen, cmpData):
        self.cmpLen = cmpLen
        self.decompLen = decompLen
        self.cmpData = cmpData
        self.name = name
    
    def writeInflationData(self, fp):
        f = open(fp, "w")
        f.write("#include \"data.h\"\n\n")
        f.write("Inflatable inflatable%s = {\n"%(self.name.title()))
        f.write("\t%d,\n\t%d,\n\tNULL,\n\t"%(self.cmpLen, self.decompLen))
        f.write("{" + ", ".join([str(hex(e)) for e in self.cmpData]) + "}\n};")

# Checks for existence of multi-element sub-array in container array
###########################################
def contains(container, elem):
    for i in container:
        if elems_equal(i, elem):
            return True
    return False 

###########################################
class AnimSequence:
    def __init__(self, blit_coords, frame_order, durations, repeat):
        self.blit_coords = blit_coords
        self.frame_order = frame_order
        self.durations   = durations
        self.repeat      = repeat
     
def is_animated(fp):
    f = open(fp)
    a = json.load(f)
    if len(a["frames"]) > 1:
        return True
    return False

###########################################
def flip_none(np_array):
    return np_array
flip_ud = np.flipud
flip_lr = np.fliplr
def flip_diag(np_array):
    return np_array[::-1, ::-1, :]
 
# TODO: make an if-statement about system's endianness that conditionally swaps flip_none and flip_lr
flip_funcs = {FLIP_NONE: flip_lr, FLIP_UD: flip_ud, FLIP_LR: flip_none, FLIP_DIAG: flip_diag}
 
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
 
###########################################
def get_color_palette(img):
    color_palette = []
    for x in range(img.shape[0]):
        for y in range(img.shape[1]):
            pixel = img[x, y]
            if not contains(color_palette, pixel):
                color_palette.append(pixel)
    return color_palette

###########################################
def convertToRGBHex(rgbNPArray):
    r = rgbNPArray[0]
    g = rgbNPArray[1]
    b = rgbNPArray[2]
    return ((b & 0xFF) << 16) | ((g & 0xFF) << 8) | ((r & 0xFF) << 0)
# Gray-out sprite colors for data compression. Enforce SNES standards of 8 max colors/sprite.
# Color brightness formula:
#   https://stackoverflow.com/questions/59628/formula-to-determine-brightness-of-rgb-color
###########################################
def iniColormap(img):
    # Get unique pixel values to make color palette
    # I only want to support up to 8 colors like the SNES.
    color_palette = get_color_palette(img)
    colorDict = {}
    idx = 0

    for color in color_palette[:8]:
        rgbHex = convertToRGBHex(color)
        colorDict[rgbHex] = idx
        idx += 1
    colorDict[255] = len(color_palette)
     
    colormap = (255 * np.ones((img.shape[0], img.shape[1]))).astype(np.uint8)
    # Map each value in color image to its rank
    for x in range(img.shape[0]):
        for y in range(img.shape[1]):
            rgbHex = convertToRGBHex(img[x, y])
            try:
                colormap[x, y] = colorDict[rgbHex]
            except:
                print("trying to access colorDict with key %d"%(rgbHex))
                quit()
             
    return colormap, color_palette
     
###########################################
def calcExpDeflatedNBytes(w, h, bpp):
    return (w * h * bpp) / 8

###########################################
def packBytes(tileList, bpp):
    np_tileset = np.zeros((tileList[0].shape[0], tileList[0].shape[1] * len(tileList))).astype("uint8")
    for i in range(len(tileList)):
        np_tileset[:, i*8: (i+1)*8] = tileList[i]
    flat = np_tileset.flatten().astype("uint8")
    flatNew = np.zeros(len(flat) // (8 // bpp)).astype("uint8")
    if bpp == 1:
        for i in range(len(flatNew)):
            flatNew[i] = ((flat[(i * 8) + 7] & 0x01) << 0) | \
                         ((flat[(i * 8) + 6] & 0x01) << 1) | \
                         ((flat[(i * 8) + 5] & 0x01) << 2) | \
                         ((flat[(i * 8) + 4] & 0x01) << 3) | \
                         ((flat[(i * 8) + 3] & 0x01) << 4) | \
                         ((flat[(i * 8) + 2] & 0x01) << 5) | \
                         ((flat[(i * 8) + 1] & 0x01) << 6) | \
                         ((flat[(i * 8) + 0] & 0x01) << 7).astype("uint8") 
    elif bpp == 2:
        for i in range(len(flatNew)):
            flatNew[i] = ((flat[(i * 4) + 3] & 0x03) << 0) | \
                         ((flat[(i * 4) + 2] & 0x03) << 2) | \
                         ((flat[(i * 4) + 1] & 0x03) << 4) | \
                         ((flat[(i * 4) + 0] & 0x03) << 6).astype("uint8")
    elif bpp == 4:
        for i in range(len(flatNew)):
            flatNew[i] = ((flat[(i * 2) + 1] & 0x0f) << 0) | \
                         ((flat[(i * 2) + 0] & 0x0f) << 4).astype("uint8")

    return flatNew 

###########################################
def unpackBytes(packedBytes, w, h, bpp):
    np_tileset = np.zeros((w, h)).astype("uint8")
    colormap = np.zeros(len(packedBytes) * (8 // bpp)).astype("uint8")
    print("colormap shape: " + str(colormap.shape))
    print("packed bytes len: " + str(len(packedBytes)))
    if bpp == 1:
        for i in range(len(packedBytes)):
            colormap[(i * 8) + 0] = ((packedBytes[i] & 0x01) >> 0) 
            colormap[(i * 8) + 1] = ((packedBytes[i] & 0x02) >> 1)
            colormap[(i * 8) + 2] = ((packedBytes[i] & 0x04) >> 2)
            colormap[(i * 8) + 3] = ((packedBytes[i] & 0x08) >> 3)
            colormap[(i * 8) + 4] = ((packedBytes[i] & 0x10) >> 4)
            colormap[(i * 8) + 5] = ((packedBytes[i] & 0x20) >> 5)
            colormap[(i * 8) + 6] = ((packedBytes[i] & 0x40) >> 6)
            colormap[(i * 8) + 7] = ((packedBytes[i] & 0x80) >> 7)
    elif bpp == 2:
        for i in range(len(packedBytes)):
            colormap[(i * 4) + 0] = ((packedBytes[i] & 0x03) >> 0) 
            colormap[(i * 4) + 1] = ((packedBytes[i] & 0x0c) >> 2)
            colormap[(i * 4) + 2] = ((packedBytes[i] & 0x30) >> 4)
            colormap[(i * 4) + 3] = ((packedBytes[i] & 0xc0) >> 6)
    elif bpp == 4:
        for i in range(len(packedBytes)):
            colormap[(i * 2) + 1] = ((packedBytes[i] & 0x0f) >> 0)
            colormap[(i * 2) + 0] = ((packedBytes[i] & 0xf0) >> 4)

    return colormap.reshape((h, w))

###########################################
def compress_img(img_fp):
    img_tokens = img_fp.split(os.sep)
    img_dir = (os.sep).join(img_tokens[:-1])
    if "_" in img_tokens[-1]:
        img_nm_tokens = img_tokens[-1].split("_")
        img_nm   = img_nm_tokens[0].split(".")[0]
        cp_nm    = img_nm_tokens[1]
    else:
        img_nm = img_tokens[-1].split(".")[0]
    # FOR DEBUGGING
    origFileSz = os.path.getsize(img_fp)

    img = cv2.imread(img_fp)
     
    print("image shape: %d x %d"%(img.shape[0], img.shape[1]))
    assert img.shape[0] % 8 == 0
    assert img.shape[1] % 8 == 0   

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
     
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    #~~~~~ Colormap Tileset & Tileset ~~~~~~~#
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    colormap, colorPalette = iniColormap(img)
    tileList, tilemap = map_tiles(colormap)
    flat_packed_tileset = bytes(packBytes(tileList, bpp).tolist())

    # I was RIGHT!! The PNG writer function sucks ass! Raw zlib compression it is then.

    print("pre-compression length: %d"%(len(flat_packed_tileset)))
    from zlib import compress, decompress
    c = compress(flat_packed_tileset)
    d = decompress(c)
    print("compressed length: %d"%(len(c)))
    print("inflated length: %d"%(len(d)))
    
    # Tell game engine how to deflate colormap tileset
    inflatable = Inflatable(img_nm, len(c), len(d), c)
    inflatable.writeInflationData("%s/%s.c"%(img_dir, img_nm))

    # Make sure the inflated image matches the input!       width                                   height         bpp
    colormapTileset = unpackBytes(flat_packed_tileset, len(tileList) * tileList[0].shape[1], tileList[0].shape[0], bpp)
    reconImg = reconstructImage(colormapTileset, tilemap, colorPalette, img.shape, bpp)
    print("recon shape: %s... input shape: %s"%(str(reconImg.shape), str(img.shape)))
    if ((reconImg == img).all()):
        print("Reconstructed image is identical to input.")
    else:
        print("Reconstructed image is different from input.")

def proc_img(img_name):
    img_obj = compress_img(img_name)
    
def reconstructImage(colormapTileset, tilemap, colorPalette, imgShape, bpp):
    # Construct colored-in tileset
    coloredTileset = np.zeros(colormapTileset.shape + (3,))
    for x in range(colormapTileset.shape[0]):
        for y in range(colormapTileset.shape[1]):
            coloredTileset[x, y] = colorPalette[colormapTileset[x, y] & 0x0f]
    # Reconstruct image 
    img = np.zeros((imgShape))
    for x in range(tilemap.shape[0]):
        for y in range(tilemap.shape[1]):
            tileIdx = tilemap[x, y]
            flip_func = flip_funcs[tileIdx & FLIP_MASK] 
            maskedIdx = tileIdx & 0x3FFF
            tile = flip_func(coloredTileset[:, (maskedIdx * 8) : ((maskedIdx + 1) * 8), :])
            img[(x * 8) : ((x + 1) * 8), (y * 8) : ((y + 1) * 8), :] = tile
    cv2.imwrite("./reconstructed.png", img)
    return img
            

args = sys.argv
if len(args) > 1:
    print("running mkimg for each image in [" + ", ".join(args[1:])+ "].")
    for arg in args[1:]:
        proc_img(arg)
