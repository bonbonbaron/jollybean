#include "xMain.h"
//***************************************************
//** System systems *********************************
//***************************************************
static Error xMainRunXSystems(Focus *fP);
static Error xMainRunTrees(Focus *fP);
X_(Main, 1, Focus_(0, xMainRunXSystems), Focus_(1, xMainRunTrees));

typedef struct {
	Key size;
	Key count; 
	Key geneType;
	Key geneClass;
} XHistoElem;

typedef struct {
  XHistoElem *xheA;
  Key nDistinctShareds;
  Key nDistinctBbs;
} GeneHisto;

// =====================================================================
// Loop through each genome's genes and histogram their component types.
// =====================================================================
static Error _histoGeneTypes(GeneHisto *geneHistoP, Biome *biomeP) {
	if (!geneHistoP || !geneHistoP->xheA || !biomeP)
		return E_BAD_ARGS;

  XHistoElem *histoA = geneHistoP->xheA;
  XHistoElem *elemP;
	Seed **seedPP = biomeP->seedPA;
	Seed **seedEndPP = seedPP + biomeP->nEntities;   // pointer to the end of the above array
	Genome *genomeP;
	GeneHeader **ghPP, **ghEndPP;   // pointers to an array of gene header pointers and its end
	GeneHeader gh;  // faster access to double pointer'd gene header
	// Loop through genomes
	for (; seedPP < seedEndPP; seedPP++) {
		genomeP = (*seedPP)->genomeP;
		ghPP = &genomeP->genePA[0];
		ghEndPP = ghPP + genomeP->nGenes;
		// Histo genome's genes that aren't blackboard items.
		for (; ghPP < ghEndPP; ghPP++) {
			gh = **ghPP;
			if (gh.geneClass != BLACKBOARD) {
        elemP = &histoA[gh.type];
        if (!elemP->count && gh.geneClass == ECS_SHARED)
          geneHistoP->nDistinctShareds++;
				elemP->count++;
				elemP->size = gh.size;
				elemP->geneClass = gh.geneClass;
			}
      //if (gh.geneClass == BLACKBOARD)
        //geneHistoP->nBbs++;
		}
	}
	return SUCCESS;
}

// =====================================================================
// Distribute all genes to their appropriate subsystems.
// =====================================================================
static Error _distributeGenes(XMain *xMainSysP, Key nXSystemsMax) {
	Seed **seedPP = xMainSysP->biomeP->seedPA;
	Seed **seedEndPP = seedPP + xMainSysP->biomeP->nEntities;   // pointer to the end of the above array
	Genome *genomeP;
	GeneHeader **ghPP, **ghEndPP;   // pointers to an array of gene header pointers and its end
	GeneHeader gh;  // faster access to double pointer'd gene header
	XHeader xh = {0};
	Key componentType;
	System *sP = NULL;
  Map *innerMapP;
	Error e = SUCCESS;
	
	// Loop through each entity's seed
	for (Entity entityCounter = nXSystemsMax; !e && seedPP < seedEndPP; seedPP++, entityCounter++) {
		genomeP = (*seedPP)->genomeP;
		ghPP = (*seedPP)->genomeP->genePA;
		ghEndPP = ghPP + genomeP->nGenes;
		// Loop through current genome's genes
		for (; !e && ghPP < ghEndPP; ghPP++) {  // ghPP is a pointer to a pointer to a global singleton of a component
			gh = **ghPP;
      if (gh.geneClass == ECS_COMPONENT) {
        componentType = gh.type;
        sP = (System*) xGetComp(sP, componentType);  // We don't set the owner of the gene pool.
        if (sP) {
          xh.type = componentType;
          e = xAddComp(sP, entityCounter, &xh);
        }
      }
      else if (gh.geneClass == ECS_SHARED) {
        // Outer map is a map of maps. The key to it is the type of shared object.
        innerMapP = (Map*) mapGet(xMainSysP->sharedMMP, gh.type);  // Inner map knows how big gene's header's container is.
        if (innerMapP)
          // Inner map is a map of components. Multiple systems' components point to them. 
          e = mapSet(innerMapP, entityCounter, (const void*) *ghPP);  // Map knows how big gene's header's container is.
      }
		}
  }
  if (!e) {
    // Give each child system's components their shared members, if any.
    System *subSysP = xMainSysP->system.compDirectoryP->mapA;
    System *subSysEndP = subSysP + arrayGetNElems((void*)xMainSysP->system.compDirectoryP->mapA);
    for (; subSysP < subSysEndP; subSysP++)   
      (*subSysP->sGetShareFP)(sP, xMainSysP->sharedMMP);
  }
	return e;
}


  /* typedef struct {
    Entity entity;
    Personality *personalityP;
    Blackboard *bbP;  // Why array of pointers and not BBs themselves? Because btRun() takes Blackboard*.
  } XGoIniSeed; 

  typedef struct {
    Key nSeeds;
    Key nDistinctIndividualQuirks;
    Key nDistinctHivemindQuirks;
    XGoIniSeed *seedA;
  } XGoIniSeedPkg; */
static Error _prepXGoIniSeedPkg(XGoIniSeedPkg **pkgPP, XMainIniSysPrms *mainIniPrmsP) {
  if (!pkgPP)
    return E_BAD_ARGS;

  // alloate packge
  Error e = jbAlloc((void**) pkgPP, sizeof(XGoIniSeedPkg), 1);
  // allocate package's seed array
  XGoIniSeedPkg *pkgP = NULL;
  if (!e) {
    pkgP = *pkgPP;
    e = arrayNew((void**) &pkgP->seedA, sizeof(XGoIniSeed), mainIniPrmsP->biomeP->nEntities);
  }
  // populate each seed
  if (!e && pkgP)
    for (U32 i = 0; i < mainIniPrmsP->biomeP->nEntities; i++) {
      XGoIniSeed *goSeedP = &pkgP->seedA[i];
      Seed *seedP = mainIniPrmsP->biomeP->seedPA[i];
      if (seedP && goSeedP) {
        goSeedP->entity       = i;
        goSeedP->personalityP = seedP->personalityP;
        goSeedP->bbSeedP      = seedP->bbSeedP;
      }
      else {
        e = E_BAD_INDEX;
        break;
      }
    }
  return e;
}
// =====================================================================
// Initialize the Main system.
// =====================================================================
Error xMainIniSys(System *sP, void *sParamsP) {
  GeneHisto geneHisto;
	XMain *xMainSysP = (XMain*) sP;

	if (!sP || !sParamsP)
		return E_BAD_ARGS;

	XMainIniSysPrms *xMainIniSysPrmsP = (XMainIniSysPrms*) sParamsP;

	if (!xMainIniSysPrmsP->biomeP || !xMainIniSysPrmsP->xSysPA)
		return E_BAD_ARGS;
	
  // Histo gene types.
  Error e = arrayNew((void**) &geneHisto.xheA, sizeof(Key), xMainIniSysPrmsP->nXSystems);
  if (!e) { 
    xMainSysP->biomeP = xMainIniSysPrmsP->biomeP;
    e = _histoGeneTypes(&geneHisto, xMainSysP->biomeP);
  }
	
	/*********************************************************
	************* Put subsystems into main system  ***********
	*********************************************************/
	// Add subsystems as components to Main system.
	if (!e && xMainIniSysPrmsP) {
		for (U32 i = 0; !e && i < xMainIniSysPrmsP->nXSystems; i++) 
			if (xMainIniSysPrmsP->xSysPA[i])
				e = xAddComp(sP, xMainIniSysPrmsP->xSysPA[i]->id, &xMainIniSysPrmsP->xSysPA[i]->xHeader);
  }

	/***********************************************
	************* Set up Behavior System ***********
	***********************************************/
  System *sGoP = NULL;
  XGoIniSeedPkg *xGoSeedPkgP = NULL;
  // Add Go System to Main's components.
  if (!e && xMainIniSysPrmsP->behaviorSysP) {
    sGoP = xMainIniSysPrmsP->behaviorSysP;
    Entity goSysId = xMainIniSysPrmsP->behaviorSysP->id;
    e = xAddComp(sP, goSysId, &xMainIniSysPrmsP->behaviorSysP->xHeader);
  }
  // Prepare initialization package for Go System.
  if (!e)
    _prepXGoIniSeedPkg(&xGoSeedPkgP, xMainIniSysPrmsP);
  // Initialize Go System.
  if (!e && sGoP && xGoSeedPkgP)
    xIniSys(sGoP, xMainIniSysPrmsP->biomeP->nEntities, xGoSeedPkgP);
  arrayDel((void**) &xGoSeedPkgP->seedA);
  jbFree((void**) &xGoSeedPkgP);

	// Initialize subsystems before we put components in them.
#define SYS_TYPE_GO (8)  /* TODO move to enum */
  XHistoElem *xheP, *xheEndP;
	if (!e) {
		xheP = &geneHisto.xheA[0];
		xheEndP = xheP + xMainIniSysPrmsP->nXSystemsMax;
		for (Entity sID = 0; !e && xheP < xheEndP; xheP++, sID++) 
			if (xheP->geneClass == ECS_COMPONENT && xheP->count)  {  // i.e. if any entities exist having components for this system...
				System *_sP = (System*) xGetComp(sP, sID);
				if (_sP)
					e = xIniSys(_sP, xheP->count, _sP->sIniSParamsP);  // makes subsystem's component directory and activities
			}
	}

	/******************************************
	************* Allocate space for Shared Components ***********
	******************************************/
	// Allocate map of sub-maps of shared components before we put those in them.
	if (!e && geneHisto.nDistinctShareds)
		e = mapNew(&xMainSysP->sharedMMP, sizeof(Map), geneHisto.nDistinctShareds);
	if (!e && xMainSysP->sharedMMP) {
		xheP = &geneHisto.xheA[0];
		xheEndP = xheP + xMainIniSysPrmsP->nXSystemsMax;
		Map *mapP = NULL;
		for (; !e && xheP < xheEndP; xheP++)
			if (xheP->geneClass == ECS_SHARED && xheP->count) {
				e = mapNew(&mapP, xheP->size, xheP->count);
				if (!e)
					e = mapSet(xMainSysP->sharedMMP, xheP->geneType, (void*) mapP);
			}
	}

	// ===========================================
	// Distribute the genes to the proper systems.
	// ===========================================
	if (!e) 
		e = _distributeGenes(xMainSysP, xMainIniSysPrmsP->nXSystemsMax);

  if (!e)
    e = xStartFocus(&xMainSysP->system, xMainIniSysPrmsP->behaviorSysP->id, 1);

  #define START (0)  /* TODO move to enum */
  if (!e) {
    System *xGoSysP = (System*) xGetComp(sP, xMainIniSysPrmsP->behaviorSysP->id);
    if (xGoSysP)
      e = mailboxWrite(xGoSysP->inboxP, 0, 0, 0, START);
  }

  arrayDel((void**) &geneHisto.xheA);

	return e;
}

// ====================================================================================
// Placeholder for component-initialization; this has to be handled in xSystemIniS().
// ====================================================================================
Error xMainIniComp(System *sP, XHeader *xhP) {
	unused_(xhP);
	unused_(sP);
	return SUCCESS;
}

XGetShareFuncDefUnused_(Main);

Error xMainProcessMessage(System *sP, Message *msgP) {
  XMainComp *subsystemP = (XMainComp*) xGetComp(sP, msgP->topic);
  if (!subsystemP)
    return E_MAILBOX_BAD_RECIPIENT;
  return mailboxForward(subsystemP->inboxP, msgP);
}

static Error xMainRunTrees(Focus *fP) {
  assert(fP);
  Error e = SUCCESS;
  System *sGoP = (System*) fP->compA;
  // There's only ever going to be one component in this array.
  if (fP->firstInactiveIdx)
    e = xRun((System*) fP->compA);
  // Put idle systems to sleep.
  if (!e && sGoP->firstInactiveActIdx == 0)
    e = xDeactivateComp(fP->ownerP, sGoP->id);
  return e;
}

// This focus runs System's subsystems.
static Error xMainRunXSystems(Focus *fP) {
	assert(fP);
	XMainComp *cP, *cEndP;
	Error e = SUCCESS;
	arrayIniPtrs(fP->compA, (void**) &cP, (void**) &cEndP, fP->firstInactiveIdx);
	for (cP = fP->compA; !e && cP < cEndP; cP++) {
		e = xRun((System*) cP);
		// Put idle systems to sleep.
		if (!e && cP->firstInactiveActIdx == 0)
			e = xDeactivateComp(fP->ownerP, cP->id);
	}
	return e;
}

// make interface happy for now
Error xMainClr(System *sP) {
  unused_(sP);
  return SUCCESS;
}

/* xIni() initializes the parent system as well as its children. */
Error xMainIni(XMain **xMainSysPP, System **sPA, U16 nXSystems, Key nXSystemsMax, Biome *biomeP) {
	if (!sPA || nXSystems < 1 || !biomeP)
		return E_BAD_ARGS;

	XMainIniSysPrms xMainSysIniPrms = {
		.nXSystemsMax = nXSystemsMax,
		.nXSystems = nXSystems,
		.biomeP = biomeP,
		.xSysPA = sPA
  };

	Error e = jbAlloc((void**) xMainSysPP, sizeof(XMain), 1);

	if (!e)
		e = xIniSys(&(*xMainSysPP)->system, nXSystems, (void*) &xMainSysIniPrms);

	return e;
}
