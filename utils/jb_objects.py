from py_defs import JBObject

class Vector2(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.x = 0
		self.y = 0

class Vector3(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.x = 0
		self.y = 0
		self.z = 0

class ListNode(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.data = None
		self.next = None
		self.prev = None

class LinkedList(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.head = None
		self.tail = None

class QuadNode(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.nodes = None
		self.num_elems = 0
		self.depth = 0
		self.tl_corner = 0
		self.br_corner = 0
		self.elems = None

class QuadTree(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.root = None
		self.max_depth = 0
		self.max_num_elems = 0

class MediaInfo(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.offset = 0
		self.uncompressed_len = 0
		self.compressed_len = 0

class Tileset(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_tiles = 0
		self.bpp = 0
		self.mi_idx = 0
		self.surface_p = None
		self.last_modified_tm = 0

class Tilemap(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.ts_idx = 0
		self.tm_arry = None
		self.dimensions = 0

class ColorPalette(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_colors = 0
		self.cp_arry = None

class Image(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.tm_idx = 0
		self.cp_idx = 0
		self.surface_p = None

class CollisionBounds(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.n = 0
		self.s = 0
		self.e = 0
		self.w = 0

class Sprite(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.rect = 0
		self.image_p = None

class LevelLayer(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.tilemap_p = None
		self.logic_grid_arry = None
		self.tileset_p = None

class SceneTblRow(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.level_layer_arry = None
		self.sprite_id_arry = None
		self.position_arry = None

class Scene(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.type = 0
		self.sprite_arry = 0
		self.bg_arry = 0
		self.coll_grid_ppp = None
		self.coll_quadtree = 0

class MotionTranslation(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.vel = 0
		self.acc = 0
		self.vel_max = 0

class MotionLerping(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.tgt = None
		self.low_const = 0
		self.high_const = 0

class MobInfo(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.mob_translate = 0
		self.mob_lerp = 0

class Motion(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.move_callback = 0
		self.mob_info = 0
		self.qn = None

class LerpConsts(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.low = 0
		self.high = 0

class TblMobileRow(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_motions = 0
		self.motions = None

class AnimationStrip(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.repeat = 0
		self.max_frame_idx = 0
		self.durations = None
		self.blit_coords = None

class TblAnimRow(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_anim_strips = 0
		self.anim_strips = None

class Animation(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.blit_coords = 0
		self.curr_frame_num = 0
		self.curr_duration = 0
		self.curr_anim_strip = None
		self.anim_group = None

class AnimatedSprite(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.sprite = None
		self.animation = None

class AnimobSprite(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.sprite = None
		self.animation = None
		self.velocity = None

class ReactParams(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.sprite_p = None
		self.tgt_p = None
		self.val = 0
		self.scene_p = None

class ReactSeq(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.priority = 0
		self.num_reactions = 0
		self.repeat = 0
		self.react_callback_arry = None

class ReactSeqGrp(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.num_react_seqs = 0
		self.triggers_arry = None
		self.react_seq_p_arry = None

class ReactSeqGrpOption(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.sprite_type = 0
		self.scene_type = 0
		self.react_seq_grp_p = None

class TblReactRow(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.ptr = None
		self.num_react_seq_grp_options = 0
		self.react_seq_grp_options = None

class Exit(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.id = 0
		self.tile_id = 0
		self.transition_type = 0
		self.tgt_scene_idx = 0

class CommChannel(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.signal_on = 0
		self.subscriber_list = None

class Activity(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.tgt_p = None
		self.curr_react_seq_p = None

class Subscriber(JBObject):
	def __init__(self):
		JBObject.__init__(self)
		self.busy = 0
		self.s = None
		self.activity = 0
