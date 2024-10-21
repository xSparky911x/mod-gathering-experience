#include "Define.h"
#include "Player.h"
#include "Creature.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "ObjectMgr.h"
#include "Log.h"

#ifndef GATHERING_EXPERIENCE_UTILS_H
#define GATHERING_EXPERIENCE_UTILS_H

float GetRarityMultiplier(uint32 itemId);

uint32 CalculateExperience(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill, uint32 itemId);
uint32 GetSkillBasedXP(uint32 baseXP, uint32 requiredSkill, uint32 currentSkill);

#endif // GATHERING_EXPERIENCE_UTILS_H
