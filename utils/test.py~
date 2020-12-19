import db
from jb_objects import Image
import enum

db.create_db("image")

ben = Image()
ben.name = "ben"
ben.tbl_name = "image"
ben.tilemap_p = "&tm_ben"
ben.color_palette_p = "&cp_aqua"
ben.surface_p = "NULL"
db.merge("image", ben)


bill = Image()
bill.name = "bill"
bill.tbl_name = "image"
bill.tilemap_p = "&tm_bill"
bill.color_palette_p = "&cp_hot"
bill.surface_p = "NULL"
db.merge("image", bill)

ben.name = "ben"
ben.tbl_name = "image"
ben.tilemap_p = "&tm_crap"
ben.color_palette_p = "&cp_sexy"
ben.surface_p = "NULL"
db.merge("image", ben)

woman = Image()
woman.name = "woman"
woman.tbl_name = "image"
woman.tilemap_p = "&tm_woman"
woman.color_palette_p = "&cp_ugly"
woman.surface_p = "NULL"
db.merge("image", woman)


db.delete("image", bill)

enum.create_enum_file()
enum.create_enum("Sprite")
enum.insert_enum_elem("Sprite", "bill")
enum.create_enum("Level")
enum.insert_enum_elem("Level", "town1")
enum.insert_enum_elem("Level", "town2")
enum.insert_enum_elem("Level", "cave1")
enum.insert_enum_elem("Level", "cave1")
enum.delete_enum_elem("Level", "town2")
