#include "GatheringExperienceHooks.h"
#include "GatheringExperienceUtils.h"
#include "GatheringExperienceXPValues.h"

#include "Define.h"
#include "Player.h"
#include "Creature.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "ObjectMgr.h"
#include "Log.h"

// Hook for Mining, Herbalism, Skinning, and Fishing (Looting a resource node)
void OnLootItem(Player* player, Item* item)
{
    if (!player || !item)
        return;

    uint32 itemId = item->GetEntry();  // Get the item ID

    // Ensure the looted item is part of a gathering profession (herbalism/mining/skinning/fishing)
    uint32 baseXP = GetGatheringBaseXP(itemId);
    if (baseXP == 0)
        return; // Skip non-gathering items

    // Get the player's current skill level in the appropriate profession
    uint32 currentSkill = 0;
    uint32 requiredSkill = 0;

    // Determine if this is a mining, herbalism, or fishing node
    if (player->HasSkill(SKILL_MINING))
    {
        currentSkill = player->GetSkillValue(SKILL_MINING);
        requiredSkill = (itemId == 2770) ? 1 : (itemId == 10620) ? 200 : 50;
    }
    else if (player->HasSkill(SKILL_HERBALISM))
    {
        currentSkill = player->GetSkillValue(SKILL_HERBALISM);
        requiredSkill = (itemId == 765) ? 1 : (itemId == 13463) ? 150 : 50;
    }
    else if (player->HasSkill(SKILL_FISHING))
    {
        currentSkill = player->GetSkillValue(SKILL_FISHING);
        requiredSkill = 1; // Fishing does not typically have different skill requirements for specific fish
    }
    else
    {
        return; // No gathering skill, exit
    }

    // Apply the skill-based XP bonus/penalty
    uint32 skillBasedXP = GetSkillBasedXP(baseXP, requiredSkill, currentSkill);

    // Apply rarity multiplier
    float rarityMultiplier = GetRarityMultiplier(itemId);
    uint32 finalXP = static_cast<uint32>(skillBasedXP * rarityMultiplier);

    // Calculate scaled experience based on player's level and skill difference, now with itemId
    uint32 xp = CalculateExperience(player, finalXP, requiredSkill, currentSkill, itemId);

    // Give the player the experience with the capped value
    player->GiveXP(xp, nullptr);
}

// Hook for Skinning (When the player skins a creature)
void OnKillCreature(Player* player, Creature* creature)
{
    if (!player || !creature)
        return;

    // Check if the GatheringExperience module is enabled
    if (!sConfigMgr->GetOption<bool>("GatheringExperience.Enable", true))
        return; // Exit if the module is disabled

    // Check if the player can skin the creature and if it's a beast (skinnable creatures)
    if (player->HasSkill(SKILL_SKINNING) && creature->GetCreatureType() == CREATURE_TYPE_BEAST && creature->loot.isLooted())
    {
        // Get the player's current skinning skill
        uint32 currentSkill = player->GetSkillValue(SKILL_SKINNING);
        uint32 requiredSkill = creature->GetLevel() * 5; // Simplified, adjust as needed based on actual skinning mechanics

        // Base XP for skinning, adjustable based on creature level
        uint32 baseXP = requiredSkill / 2; // Example logic for skinning XP

        // Apply skill-based XP bonus/penalty
        uint32 skillBasedXP = GetSkillBasedXP(baseXP, requiredSkill, currentSkill);

        // Apply rarity multiplier for skinning items
        float rarityMultiplier = 1.5f; // Adjust as needed
        uint32 finalXP = static_cast<uint32>(skillBasedXP * rarityMultiplier);

        // Pass 0 as itemId since there's no specific item for skinning
        uint32 xp = CalculateExperience(player, finalXP, requiredSkill, currentSkill, 0);

        // Give the player the experience with the capped value
        player->GiveXP(xp, nullptr);
    }
}
