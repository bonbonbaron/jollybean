import jb_db
import jb_media 
from jb_objects import Image
import jb_enum

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




'''
    When I make a system:

    (emphasizing how out-of-order things can be)

    PRIMITIVE SYSTEMS:
    ******************
    IMAGE SYS:
        jill, bob, glenn, dan
    ANIM SYS: 
        bob, jill, glenn
    MOTION SYS:
        dan, bob, jill
    COLLISION SYS:
        dan, jill, bob
    CONTROL SYS:
        bob
    TIMER SYS:
        dan, jill   <-- these guys have randomly timed animated walks
    TEXT SYS:
        dan, jill   <-- the only sprites you can talk to 
    
    GAME-SPECIFIC SYSTEMS:
    **********************
    BATTLE STAT SYS

    These systems will each tick every clock cycle.
    When they need to communicate, they'll need:
        1) the address of the other system
        2) the type of the other system
        3) the index of the entity in the other system (can be active or inactive if you don't want compliance)
        4) the command to send to that entity in that system

    There's a system architecture, with parent systems controlling child systems. Point of entry high as possible.

    ARCHITECTURES:
    **************
    BATTLE ARCH:
        BATTLE STAT SYS
        ANIM SYS
        MOTION SYS
        COLLISION SYS
        TIMER SYS
        CONTROL SYS
        TEXT SYS
        DIALOGUE SYS
        MENU SYS

    TOWN ARCH:

    If I pause the battle sys, the timers, animations, and motions in that sys cease. That's beautiful.
    In this way I can have multiple systems of the same type. I can cease entire inner loops. BEAUTIFUL.

    ***FIRST LAW: Architectures must be entirely self-contained. One must not depend on another.
    ***SECOND LAW: Nobody controls systems but architectures. Systems don't message each other; they msg the arch. The arch alone messages each system.


'''
