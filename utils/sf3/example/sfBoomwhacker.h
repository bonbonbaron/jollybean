#include "botox/data.h"
#include "../soundfont.h"

Zone sfBoomwhackerBank0Preset0ZoneA[] = {
	{
		.nGens = 0;
		.u.instP = &instBoomwhacker;
		.genA = NULL;
	}
};

Preset sfBoomwhackerBank0PresetA[] = {
	{
		.nZones = 0;
		.globalZoneP =  = NULL;
		.zoneA = sfBoomwhackerBank0Preset0ZoneA;
	}
};

Bank sfBoomwhackerBankA[] = {
	{
		.nPresets = 1;
		.presetA  = sfBoomwhackerBank0PresetA;
	}
};

Soundfont BoomwhackerSoundfont = {
	.nBanks = 1;
	.bankA  = sfBoomwhackerBankA;
};

