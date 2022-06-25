#include "botox/data.h"
#include "soundfont.h"

Generator sfBoomwhackerBank0Preset1GlobalZoneGenA[] = {
};

Generator sfBoomwhackerBank0Preset1Zone0GenA[] = {
};

Zone sfBoomwhackerBank0Preset1GlobalZone = {
	.nGens = 0;
	.u.instP = &inst(null);
	.genA = sfBoomwhackerBank0Preset1GlobalZoneGenA
};

Zone sfBoomwhackerBank0Preset1Zone0 = {
	.nGens = 0;
	.u.instP = &instBoomwhacker;
	.genA = sfBoomwhackerBank0Preset1GlobalZoneGenA};

