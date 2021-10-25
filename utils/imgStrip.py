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
 
class Inflatable:
    def __init__(self, name, cmpLen, decompLen, cmpData):
        self.cmpLen = cmpLen
        self.decompLen = decompLen
        self.cmpData = cmpData
        self.name = name
    
    def writeInflationData(self, fp, tsDims):
        f = open(fp, "w")
        f.write("#include \"data.h\"\n\n")
        f.write("Inflatable inflatable%s = {\n"%(self.name.title()))
        f.write("\t%d,\n\t%d,\n\tNULL,\n\t"%(self.cmpLen, self.decompLen))
        f.write("{" + ", ".join([str(hex(e)) for e in self.cmpData]) + "}\n};")
        f.write("\n\nImage img%s = {%d, %d, %d, %d, NULL};"%(self.name.title(), tsDims.w, tsDims.h, tsDims.pitch, tsDims.bpp))

class StripMap:
    def __init__(self, nBytes, pitch, bpp, idxFlipA, idxA):
        self.name = name
        self.nBytes = nBytes
        self.pitch = pitch
        self.bpp = bpp
        self.idxFlipA = idxFlipA
        self.idxA = idxA

    #TODO worry about writing to *.c later.
    def write(self, fp):
        f = open(fp, "w")
        f.write("U32 %sStripIdxA[] = {"%(self.name) + ", ".join([str(hex(e)) for e in self.idxA]) + "}\n};")
        f.write("U32 %sIdxFlipA[] = {"%(self.name) + ", ".join([str(hex(e)) for e in self.idxA]) + "}\n};")
        f.write("#include \"data.h\"\n\n")
        f.write("StripMap sm%s = {\n"%(self.name.title()))
        f.write("\t%d,\n\t%d,\n\t"%(self.nBytes, self.pitch, self.bpp))


# Checks for existence of multi-element sub-array in container array
###########################################
def contains(container, elem):
    for i in container:
        if elems_equal(i, elem):
            return True
    return False 

###########################################
STRP_N_PIXELS = 64  # images will use 8x8 s to ensure pixel count is multiple of STRP_N_PIXELS
def mapStrips(img):
    flat = img.flatten()
    numStrips = flat.shape[0] // STRP_N_PIXELS
     
    stripLabel = 0
    stripsLabelled = np.zeros((numStrips))
    stripSet = []
    flipList = []
    stripMap = np.zeros((numStrips), np.uint16)
    
    # Find distinct s and write an index-mapping to them.
    print("Analyzing viability of breaking image into strips...")
    for i in range(numStrips):
        if not stripsLabelled[i]:
            currStrip = flat[(i * STRP_N_PIXELS) : ((i + 1) * STRP_N_PIXELS)]
            stripSet.append(currStrip.astype(np.uint8))
            stripsLabelled[i] = True
            stripMap[i] = stripLabel
            # Look for unlabelled strips identical as-is or post-flip.
            for ii in range(numStrips):
                if (ii != i) and not stripsLabelled[ii]:
                    nextStrip = flat[(ii * STRP_N_PIXELS) : ((ii + 1) * STRP_N_PIXELS)]
                    if np.array_equal(currStrip, nextStrip):
                        stripMap[ii] = stripLabel  # flipped strip gets same label
                        stripsLabelled[ii] = True
                    # Not identical as-is. Is it identical with flipping? (Ignore homogeneous strips.)
                    elif (np.unique(nextStrip).shape[0] > 1):
                        nextStripFlipped = np.flip(nextStrip)
                        if np.array_equal(currStrip, nextStripFlipped):
                            print("gonna flip idx %d"%(ii))
                            flipList.append(ii)
                            stripMap[ii] = stripLabel  # flipped strip gets same label
                            stripsLabelled[ii] = True
            stripLabel += 1
    # Slight compression isn't worth going through the trouble to reconstruct.
    if numStrips - max(stripMap) < 5:  # arbitrary number; maybe decide more scientific number later
        return flat, None, None
    else:
        return stripSet, stripMap, flipList
     
 
###########################################
def  get_keymap_psn(filename):
    underscore_position = 0
    for i in range(len(filename)):
        if filename[i] == "_":
            underscore_position = i + 1
            break
    return underscore_position
 
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
def iniColorMap(img):
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
def packBytes(stripList, bpp):
    np_stripSet = np.zeros((len(stripList) * STRP_N_PIXELS)).astype("uint8")
    for i in range(len(stripList)):
        np_stripSet[(i * STRP_N_PIXELS) : ((i + 1) * STRP_N_PIXELS)] = stripList[i]
    flat = np_stripSet.flatten().astype("uint8")
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
def unpackBytes(packedBytes, bpp):
    np_stripSet = np.zeros(len(packedBytes)).astype("uint8")
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
    elif bpp == 8:
        pass

    return colormap

###########################################
def compress_img(img_name):
    if "_" in img_name:
        split_nm = img_name.split("_")  # Naming convention goes "ben_aqua" or [sprite name]_[color palette name]
        img_nm   = split_nm[0]
        cp_nm    = split_nm[1]
    else:
        img_nm = img_name
    # FOR DEBUGGING
    img_fp   = "/home/bonbonbaron/hack/jollybean/utils/%s.png"%(img_name)
    origFileSz = os.path.getsize(img_fp)

    img = cv2.imread(img_fp)
     
    print("image shape: %d x %d"%(img.shape[0], img.shape[1]))
    assert img.flatten().shape[0] % STRP_N_PIXELS == 0

    color_palette = get_color_palette(img)
     
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    #~~~~~~~~~~~~~~~ Tilemap ~~~~~~~~~~~~~~~~#
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    numColors = len(color_palette)
    if numColors <= 2:
        bpp = 1
    elif numColors <= 4:
        bpp = 2
    elif numColors <= 16:
        bpp = 4
    else:
        bpp = 8
    # Whatever pixel format I settle on for Jollybean, the above sizes are for storage only.
     
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    #~~~~~ Colormap StripSet & StripSet ~~~~~~~#
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
    colormap, colorPalette = iniColorMap(img)
    stripSet, stripMap, flipList = mapStrips(colormap)
    stripSetPacked = bytes(packBytes(stripSet, bpp).tolist())

    # I was RIGHT!! The PNG writer function sucks ass by corrupting data! Raw zlib compression it is then.

    print("pre-compression length: %d"%(len(stripSetPacked)))
    from zlib import compress, decompress
    c = compress(stripSetPacked)
    d = decompress(c)
    print("compressed length: %d"%(len(c)))
    print("inflated length: %d"%(len(d)))
    
    # Tell game engine how to deflate colormap stripSet
    if stripMap is None:
        print("Image was not worth breaking into strips. Compressing unmapped, grayed-out image.")
        print("number of strips: %d"%(img.flatten().shape[0] // STRP_N_PIXELS))
    else:
        print("Image was worth mapping strips for. Compressing strip set.")
        print("number of strips: %d"%max(stripMap))

    inflatable = Inflatable(img_name, len(c), len(d), c)
    #inflatable.writeInflationData("./test.c", expectedStripSetDims)

    # Give game engine the stripMap for this image if it applies.
    #class StripMap:
    #    def __init__(self, nBytes, pitch, bpp, idxFlipA, idxA):
    #if stripMap is not None:  # pixel strips were worth whittling down further for zlib compressor
        #sm = StripMap(img_name, STRP_N_PIXELS * len(stripMap), img.shape[1] * bpp // 8, bpp, flipList, stripMap)
    #else:
        #sm = StripMap(img_name, STRP_N_PIXELS * len(stripMap), img.shape[1] * bpp // 8, bpp, flipList, stripMap)
        
    #sm.write("./sm.c")

    # Make sure the inflated image matches the input!       
    stripSet = unpackBytes(stripSetPacked, bpp)
    reconImg = reconstructImage(stripSet, stripMap, flipList, colorPalette, img.shape, bpp)
    print("recon shape: %s... input shape: %s"%(str(reconImg.shape), str(img.shape)))
    if ((reconImg == img).all()):
        print("Reconstructed image is identical to input.")
    else:
        print("Reconstructed image is different from input.")

def proc_img(img_name):
    img_obj = compress_img(img_name)
    
def reconstructImage(stripSet, stripMap, flipList, colorPalette, imgShape, bpp):
    # Construct colored-in stripSet
    img = np.zeros((imgShape[0] * imgShape[1], 3)).astype("uint8")
    fullImgColormap = np.zeros((imgShape[0] * imgShape[1])).astype("uint8")
    # First write pixels in.
    if stripMap is not None:
        for i in range(stripMap.shape[0]):
            fullImgColormap[(i * STRP_N_PIXELS) : ((i + 1) * STRP_N_PIXELS)] \
            = stripSet[stripMap[i] * STRP_N_PIXELS : (stripMap[i] + 1) * STRP_N_PIXELS]
        # Then flip whatever needs flipping.
        print(flipList)
        for i in range(len(flipList)):
            fullImgColormap[(i * STRP_N_PIXELS) : ((i + 1) * STRP_N_PIXELS)] \
            = np.flip(fullImgColormap[(i * STRP_N_PIXELS) : ((i + 1) * STRP_N_PIXELS)])

    else:  # input stripSet is a full image, not a strip set
        for i in range(fullImgColormap.shape[0]):
            fullImgColormap = stripSet

    # Now color the pixels in.
    #print(img.shape)  # (12992, 3)
    #print(fullImgColormap.shape) # (16384,)
    #print(len(colorPalette))  # 7
    for i in range(fullImgColormap.shape[0]):
        img[i] = colorPalette[fullImgColormap[i]]

    img = img.reshape((imgShape[0], imgShape[1], imgShape[2]))
    print("output img shape: %s"%(str(img.shape)))
    cv2.imwrite("./reconstructed.png", img)
    return img


proc_img("bigger")