#include "botox/data.h"
#include "../soundfont.h"

Generator iBoomwhacker_zBoomwhacker_2_genA[] = {
	{
		.genType = GEN_PAN,
		.nMods = 0,
		.amount = 500,
		.modA = NULL
	},
};

Generator iBoomwhacker_zBoomwhacker_1_genA[] = {
	{
		.genType = GEN_PAN,
		.nMods = 0,
		.amount = 0,
		.modA = NULL
	},
};

Zone iBoomwhacker_zoneA[] = {
	{
		.nGens = 1,
		.u.sampleP = &sample_BoomwhackerR,
		.genA = iBoomwhacker_zBoomwhacker_2_genA
	},
	{
		.nGens = 1,
		.u.sampleP = &sample_BoomwhackerL,
		.genA = iBoomwhacker_zBoomwhacker_1_genA
	},
};

Instrument inst_Boomwhacker = {
	.nZones = 2,
	.globalZoneP = NULL,
	.zoneA = iBoomwhacker_zoneA
};

Zone pBoomwhacker_zoneA[] = {
	{
		.nGens = 0,
		.u.instP = &inst_Boomwhacker,
		.genA = NULL
	},
};

Preset preset_33936344 = {
	.nZones = 1,
	.globalZoneP = NULL,
	.zoneA = pBoomwhacker_zoneA
};

