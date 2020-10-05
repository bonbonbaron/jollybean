class LinkedList:
	def __init__(self):
		self.head = None
		self.tail = None

class MotionLerping:
	def __init__(self):
		self.tgt = None
		self.low_const = 0
		self.high_const = 0

class Vector2:
	def __init__(self):
		self.x = 0
		self.y = 0

class Vector3:
	def __init__(self):
		self.x = 0
		self.y = 0
		self.z = 0

class Subscriber:
	def __init__(self):
		self.busy = 0
		self.s = None
		self.activity = 0

class Animation:
	def __init__(self):
		self.curr_frame_num = 0
		self.curr_duration = 0
		self.curr_anim_strip = None
		self.anim_group = None

class Exit:
	def __init__(self):
		self.id = 0
		self.transition_type = 0
		self.tgt_scene_idx = 0

class AnimationStrip:
	def __init__(self):
		self.repeat = 0
		self.max_frame_idx = 0
		self.durations = None
		self.blit_coords = None

class TblReactRow:
	def __init__(self):
		self.ptr = None
		self.num_react_seq_grp_options = 0
		self.react_seq_grp_options = None

class MotionTranslation:
	def __init__(self):
		self.vel = 0
		self.acc = 0
		self.vel_max = 0

class Sprite:
	def __init__(self):
		self.onscreen = 0
		self.reaction_bookmark = 0
		self.id = 0
		self.type = 0
		self.species = 0
		self.surface = None
		self.rect = 0
		self.blit_coords = 0
		self.react_seq_grp = None

class QuadTree:
	def __init__(self):
		self.root = None
		self.max_depth = 0
		self.max_num_elems = 0

class AnimatedSprite:
	def __init__(self):
		self.sprite = None
		self.animation = None

class LerpConsts:
	def __init__(self):
		self.low = 0
		self.high = 0

class ReactSeqGrp:
	def __init__(self):
		self.priority = 0
		self.num_react_seqs = 0
		self.triggers = None
		self.react_seq_ptr = None

class ReactSeqGrpOption:
	def __init__(self):
		self.sprite_type = 0
		self.scene_type = 0
		self.react_seq_grp = None

class TblMobileRow:
	def __init__(self):
		self.num_motions = 0
		self.motions = None

class ListNode:
	def __init__(self):
		self.data = None
		self.next = None

class MobInfo:
	def __init__(self):
		self.mob_translate = 0
		self.mob_lerp = 0

class Motion:
	def __init__(self):
		self.move = 0
		self.mob_info = 0
		self.qn = None

class TblAnimRow:
	def __init__(self):
		self.num_anim_strips = 0
		self.anim_strips = None

class AnimobSprite:
	def __init__(self):
		self.sprite = None
		self.animation = None
		self.velocity = None

class QuadNode:
	def __init__(self):
		self.nodes = None
		self.num_elems = 0
		self.depth = 0
		self.tl_corner = 0
		self.br_corner = 0
		self.elems = None

class Activity:
	def __init__(self):
		self.tgt = None
		self.current_reaction_sequence = None

class SceneTblRow:
	def __init__(self):
		self.num_sprites = 0
		self.num_exits = 0
		self.num_songs = 0
		self.song_ids = 0
		self.num_sounds = 0
		self.sound_ids = 0
		self.sprite_ids = None
		self.media_info = 0
		self.posori = None
		self.exits = None

class CommChannel:
	def __init__(self):
		self.signal_on = 0
		self.subscribers = None

class TblCameraRow:
	def __init__(self):
		self.motion = None
		self.sprite_tgt = None
		self.react_seq_grp = None

class Scene:
	def __init__(self):
		self.sprites = None
		self.num_sprites = 0
		self.type = 0
		self.bg_idx = 0
		self.exits = None
		self.backgrounds = None
		self.signals = 0
		self.camera = 0
		self.coll_quadtree = 0
		self.coll_grid = None

class MediaInfo:
	def __init__(self):
		self.offset = 0
		self.uncompressed_len = 0
		self.compressed_len = 0

class Camera:
	def __init__(self):
		self.rect = 0
		self.tgt = None
		self.tgt_offset = 0
		self.react_seq_grp = None
		self.motion = None

class ReactSeq:
	def __init__(self):
		self.num_reactions = 0
		self.repeat = 0
		self.react_func_ptr = None

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

