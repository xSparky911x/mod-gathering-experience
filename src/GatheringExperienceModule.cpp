/*
 * Copyright (C) 2024+ Thaxtin, released under GNU AGPL v3 license: https://github.com/xSparky911x/mod-gathering-experience/blob/master/LICENSE
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "GameEventMgr.h"
#include "SkillDiscovery.h"

// Define the maximum level in Wrath of the Lich King
const uint32 MAX_LEVEL = 80;

class GatheringExperienceModule : public PlayerScript
{
public:
    GatheringExperienceModule() : PlayerScript("GatheringExperienceModule") { }

    // Function to calculate scaled experience based on player level
    uint32 CalculateExperience(Player* player, uint32 baseXP)
    {
        uint32 playerLevel = player->getLevel();
        
        // Apply the scaling formula
        uint32 scaledXP = static_cast<uint32>(baseXP * (1.0 - static_cast<float>(playerLevel) / MAX_LEVEL));

        // Ensure a minimum XP of 1 to avoid giving 0 XP
        return std::max(scaledXP, 1u);
    }

    // Function to calculate bonus XP based on the player's skill vs the required skill
    uint32 GetSkillBasedXP(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill)
    {
        if (currentSkill < requiredSkill)
        {
            // Increase XP for challenging nodes (player's skill is lower than required)
            uint32 bonusXP = (requiredSkill - currentSkill) * 2; // XP bonus scaling
            return baseXP + bonusXP;
        }
        else
        {
            // Penalize XP if the node is too easy but keep a minimum XP threshold
            uint32 penaltyXP = (currentSkill - requiredSkill) * 1; // XP penalty scaling
            return std::max(baseXP - penaltyXP, 1u); // Ensure at least 1 XP is given
        }
    }

    // Function to get base XP for different mining or herbalism items
    uint32 GetGatheringBaseXP(uint32 itemId)
    {
        // Mining item XP values
        const std::map<uint32, uint32> miningItemsXP = {
            { 2770, 50 },  // Copper Ore
            { 2771, 100 }, // Tin Ore
            { 2772, 200 }, // Iron Ore (Higher level ore)
            { 10620, 500 } // Thorium Ore (Rare/High level)
        };

        // Herbalism item XP values
        const std::map<uint32, uint32> herbalismItemsXP = {
            { 765, 50 },   // Silverleaf
            { 2447, 75 },  // Peacebloom
            { 8836, 200 }, // Arthas' Tears (Rare herb)
            { 13463, 300 } // Dreamfoil (High-level herb)
        };

        // Check if the item is a mining item
        auto miningXP = miningItemsXP.find(itemId);
        if (miningXP != miningItemsXP.end())
            return miningXP->second;

        // Check if the item is a herbalism item
        auto herbXP = herbalismItemsXP.find(itemId);
        if (herbXP != herbalismItemsXP.end())
            return herbXP->second;

        // Default base XP if the item is not found in the above tables
        return 50; // Default to a low base XP
    }

    // Function to apply rarity-based multipliers for special items
    float GetRarityMultiplier(uint32 itemId)
    {
        // Define rarity multipliers for high-end gathering items
        const std::map<uint32, float> rarityMultipliers = {
            { 13463, 1.5f },  // Dreamfoil (example)
            { 10620, 2.0f },  // Thorium Ore (rare)
            { 13467, 3.0f },  // Black Lotus (super rare)
        };

        auto rarity = rarityMultipliers.find(itemId);
        if (rarity != rarityMultipliers.end())
            return rarity->second;

        // Default multiplier
        return 1.0f;
    }

    // Hook for Mining and Herbalism (Looting a resource node)
    void OnLootItem(Player* player, Item* item)
    {
        uint32 itemId = item->GetEntry();

        // Get the player's current skill level in the appropriate profession
        uint32 currentSkill = 0;
        uint32 requiredSkill = 0;

        // Determine if this is a mining or herbalism node
        if (player->HasSkill(SKILL_MINING))
        {
            currentSkill = player->GetSkillValue(SKILL_MINING);
            // Hardcode for now, you can implement a function that fetches correct skill requirements based on itemId
            requiredSkill = (itemId == 2770) ? 1 : (itemId == 10620) ? 200 : 50;
        }
        else if (player->HasSkill(SKILL_HERBALISM))
        {
            currentSkill = player->GetSkillValue(SKILL_HERBALISM);
            // Hardcode for now, you can implement a function that fetches correct skill requirements based on itemId
            requiredSkill = (itemId == 765) ? 1 : (itemId == 13463) ? 150 : 50;
        }

        // Get base XP for the specific item
        uint32 baseXP = GetGatheringBaseXP(itemId);

        // Apply the skill-based XP bonus/penalty
        uint32 skillBasedXP = GetSkillBasedXP(player, baseXP, requiredSkill, currentSkill);

        // Apply rarity multiplier
        float rarityMultiplier = GetRarityMultiplier(itemId);
        uint32 finalXP = static_cast<uint32>(skillBasedXP * rarityMultiplier);

        // Calculate scaled experience based on player's level
        uint32 xp = CalculateExperience(player, finalXP);

        // Give the player the experience
        player->GiveXP(xp, nullptr);

        // Send a message to the player
        player->SendBroadcastMessage("You gained experience from gathering.");
    }

    // Hook for Skinning (When the player skins a creature)
    void OnKillCreature(Player* player, Creature* creature)
    {
        // Check if the player can skin the creature and if it's a beast (skinnable creatures)
        if (player->HasSkill(SKILL_SKINNING) && creature->GetCreatureType() == CREATURE_TYPE_BEAST)
        {
            // Get the player's current skinning skill
            uint32 currentSkill = player->GetSkillValue(SKILL_SKINNING);
            uint32 requiredSkill = creature->GetLevel() * 5; // Simplified, adjust as needed based on actual skinning mechanics

            // Base XP for skinning, adjustable based on creature level
            uint32 baseXP = requiredSkill / 2; // Example logic for skinning XP

            // Apply skill-based XP bonus/penalty
            uint32 skillBasedXP = GetSkillBasedXP(player, baseXP, requiredSkill, currentSkill);

            // Calculate scaled experience based on player's level
            uint32 xp = CalculateExperience(player, skillBasedXP);

            // Give the player the experience
            player->GiveXP(xp, nullptr);

            // Send a message to the player
            player->SendBroadcastMessage("You gained experience from skinning.");
        }
    }
};

// Register the script so AzerothCore knows to use it
void AddGatheringExperienceModuleScripts()
{
    new GatheringExperienceModule();
}

void Addmod_gathering_experience()
{
    AddGatheringExperienceModuleScripts();
}