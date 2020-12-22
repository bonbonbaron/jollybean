from py_defs import JBObject

class Vector2(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.x = "Sint32"
		self.y = "Sint32"
class Vector3(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.x = "Sint32"
		self.y = "Sint32"
		self.z = "Sint32"
class ListNode(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.data = "void *"
		self.next = "struct ListNode *"
		self.prprevev = "struct ListNode * *"
class LinkedList(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.head = "ListNode *"
		self.tail = "ListNode *"
class QuadNode(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.nodes = "struct _QuadNode *"
		self.num_elems = "Uint8"
		self.depth = "Uint8"
		self.tl_corner = "Vector2"
		self.br_corner = "Vector2"
		self.elems = "LinkedList *"
class QuadTree(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.root = "QuadNode *"
		self.max_depth = "Uint8"
		self.max_num_elems = "Uint8"
class MediaInfo(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.offset = "Uint32"
		self.uncompressed_len = "Uint32"
		self.compressed_len = "Uint32"
class Tileset(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_tiles = "Uint16"
		self.bpp = "Uint8"
		self.mi_idx = "Uint16"
		self.surface_p = "SDL_Surface *"
		self.last_modified_tm = "Uint32"
class Tilemap(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.ts_idx = "Uint16"
		self.tm_arry = "Uint16 *"
		self.dimensions = "Vector2"
class ColorPalette(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_colors = "Uint8"
		self.cp_arry = "SDL_Color *"
class Image(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.tm_idx = "Uint16"
		self.cp_idx = "Uint8"
		self.surface_p = "SDL_Surface *"
class CollisionBounds(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.n = "SDL_Rect"
		self.s = "SDL_Rect"
		self.e = "SDL_Rect"
		self.w = "SDL_Rect"
class Sprite(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.rect = "SDL_Rect"
		self.image_p = "Image *"
class LevelLayer(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.tilemap_p = "Tilemap *"
		self.logic_grid_arry = "Uint8 *"
		self.tileset_p = "Tileset *"
class SceneTblRow(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.level_layer_arry = "Uint16 *"
		self.sprite_id_arry = "Uint16 *"
		self.position_arry = "Uint16 *"
class Scene(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.type = "Uint8"
		self.sprite_arry = "Sprite"
		self.bg_arry = "SDL_Surface"
		self.coll_grid_ppp = "Uint8 * * *"
		self.coll_quadtree = "QuadTree"
class MotionTranslation(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.vel = "Vector2"
		self.acc = "Vector2"
		self.vel_max = "Vector2"
class MotionLerping(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.tgt = "Sprite *"
		self.low_const = "Uint8"
		self.high_const = "Uint8"
class MobInfo(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.mob_translate = "MotionTranslation"
		self.mob_lerp = "MotionLerping"
class Motion(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.move_callback = "MoveCallback"
		self.mob_info = "MobInfo"
		self.qn = "QuadNode *"
class LerpConsts(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.low = "Uint16"
		self.high = "Uint16"
class TblMobileRow(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_motions = "Uint8"
		self.motions = "Motion *"
class AnimationStrip(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.repeat = "SDL_bool"
		self.max_frame_idx = "Uint8"
		self.durations = "Uint16 *"
		self.blit_coords = "SDL_Rect *"
class TblAnimRow(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_anim_strips = "Uint8"
		self.anim_strips = "AnimationStrip *"
class Animation(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.blit_coords = "SDL_Rect"
		self.curr_frame_num = "Uint8"
		self.curr_duration = "Uint32"
		self.curr_anim_strip = "AnimationStrip *"
		self.anim_group = "TblAnimRow *"
class AnimatedSprite(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.sprite = "Sprite *"
		self.animation = "Animation *"
class AnimobSprite(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.sprite = "Sprite *"
		self.animation = "Animation *"
		self.velocity = "Vector2 *"
class ReactParams(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.sprite_p = "Sprite *"
		self.tgt_p = "void *"
		self.val = "Uint16"
		self.scene_p = "Scene *"
class ReactSeq(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.priority = "Uint8"
		self.num_reactions = "Uint8"
		self.repeat = "SDL_bool"
		self.react_callback_arry = "ReactCallback *"
class ReactSeqGrp(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_react_seqs = "Uint8"
		self.triggers_arry = "Uint8 *"
		self.react_seq_p_arry = "ReactSeq * *"
class ReactSeqGrpOption(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.sprite_type = "Uint8"
		self.scene_type = "Uint8"
		self.react_seq_grp_p = "ReactSeqGrp *"
class TblReactRow(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.ptr = "ReactSeqGrp *"
		self.num_react_seq_grp_options = "Uint8"
		self.react_seq_grp_options = "ReactSeqGrpOption *"
class Exit(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.id = "Uint8"
		self.tile_id = "Uint16"
		self.transition_type = "Uint8"
		self.tgt_scene_idx = "Uint16"
class CommChannel(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.signal_on = "SDL_bool"
		self.subscriber_list = "LinkedList *"
class Activity(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.tgt_p = "Sprite *"
		self.curr_react_seq_p = "ReactSeq *"
class Subscriber(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.busy = "SDL_bool"
		self.s = "Sprite *"
		self.activity = "Activity"
