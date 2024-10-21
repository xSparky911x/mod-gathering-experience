#include "Define.h"
#include "Player.h"
#include "Creature.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "ObjectMgr.h"
#include "Log.h"

#ifndef GATHERINGEXPERIENCEXPVALUES_H
#define GATHERINGEXPERIENCEXPVALUES_H

const uint32 MAX_EXPERIENCE_GAIN = 300;  // Define MAX_EXPERIENCE_GAIN
const uint32 GATHERING_MAX_LEVEL = 80;   // Maximum gathering level for scaling

// Function to get base XP for different mining, herbalism, skinning, and fishing items
uint32 GetGatheringBaseXP(uint32 itemId);

#endif // GATHERINGEXPERIENCEXPVALUES_H
