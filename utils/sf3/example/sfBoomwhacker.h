#include "botox/data.h"
#include "../soundfont.h"

Generator iBoomwhacker_zBoomwhacker/2_genA[] = {
	{
		.genType = GEN_PAN,
		.nMods = 0,
		.amount = 500,
		.modA = NULL
	},
};

Generator iBoomwhacker_zBoomwhacker/1_genA[] = {
	{
		.genType = GEN_PAN,
		.nMods = 0,
		.amount = 0,
		.modA = NULL
	},
};

Zone iBoomwhacker_zoneA[] = {
	{
		.nGens = 1;
		.u.sampleP = &BoomwhackerR;
		.genA = iBoomwhacker_zBoomwhacker/2_genA;
	},
	{
		.nGens = 1;
		.u.sampleP = &BoomwhackerL;
		.genA = iBoomwhacker_zBoomwhacker/1_genA;
	},
};

