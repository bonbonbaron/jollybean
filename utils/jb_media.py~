from py_defs import *
from db import *

def create_media_file():
  f = open(MEDIA_FP, "w")
  f.close()

def clean_media_file():
  create_media_file()

def delete(offset, length):
  f = open(MEDIA_FP, "rb")
  before_data = f.read(offset)
  f.seek(length)
  after_data = f.read()
  f.close()
  f = open(MEDIA_FB, "wb")
  f.write(before_data)
  f.write(after_data)
  f.close()

def insert(data):
  f = open(MEDIA_FP, "ab")
  f.write(data)
  f.close()

def update(offset, length, new_data):
  f = open(MEDIA_FP, "rb")
  before_data = f.read(offset)
  f.seek(length)
  after_data = f.read()
  f.close()
  f = open(MEDIA_FB, "wb")
  f.write(before_data)
  f.write(new_data)
  f.write(after_data)
  f.close()
