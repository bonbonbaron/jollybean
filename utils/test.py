import db
import media 
from jb_objects import Image
import enum

db.create_tbl("image")
db.create_tbl("mediaInfo")
db.create_tbl("tileset")
db.create_tbl("tilemap")
db.create_tbl("colorPalette")

db.create_enum("image")
db.create_enum("mediaInfo")
db.create_enum("tileset")
db.create_enum("tilemap")
db.create_enum("colorPalette")

media.create_media_file()

ben = Image()

ben.name = "ben"
ben.tbl_name = "image"
ben.tilemap_p = "&tm_ben"
ben.color_palette_p = "&cp_aqua"
ben.surface_p = "NULL"
db.insert("image", ben)

enum.create_enum_file()
enum.create_enum("Sprite")
enum.insert_enum_elem("Sprite", "bill")
enum.create_enum("Level")
