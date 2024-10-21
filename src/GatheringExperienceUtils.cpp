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

// Function to calculate scaled experience based on player level and item base XP
uint32 CalculateExperience(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill, uint32 itemId)
{
    uint32 playerLevel = player->GetLevel();

    // Apply stronger diminishing returns for being over-skilled
    uint32 skillDifference = (currentSkill > requiredSkill) ? (currentSkill - requiredSkill) : 0;
    float skillMultiplier = 1.0f - (skillDifference * 0.02f); // Slight penalty: 2% reduction per skill point over

    // Apply a scaling formula to reduce XP based on the player's level
    float levelMultiplier = 0.5f + (0.5f * (1.0f - static_cast<float>(playerLevel) / GATHERING_MAX_LEVEL));

    // Calculate final XP with scaling and diminishing returns
    uint32 scaledXP = static_cast<uint32>(baseXP * skillMultiplier * levelMultiplier);

    // Debug logging to see values in the console
    LOG_INFO("module", "ItemId: %u, BaseXP: %u, SkillMultiplier: %.2f, LevelMultiplier: %.2f, ScaledXP: %u", itemId, baseXP, skillMultiplier, levelMultiplier, scaledXP);

    // Cap for high-level items like Thorium, Star Ruby
    if (baseXP > 350)  // High-level items like Thorium, Arcane Crystal, etc.
    {
        scaledXP = std::max(scaledXP, 100u);  // Minimum XP of 100 for high-level items
        scaledXP = std::min(scaledXP, 150u);  // Max XP cap of 150 for high-level items
    }
    // Adjusted cap for medium-level items like Mithril, Truesilver
    else if (baseXP > 200 && baseXP <= 350)  // Mid-level items like Mithril, Truesilver
    {
        scaledXP = std::max(scaledXP, 50u);  // Minimum XP of 50 for mid-level items
        scaledXP = std::min(scaledXP, 100u);  // Max XP cap of 100 for mid-level items
    }
    else if (baseXP <= 200)  // Low-level items like Copper, Tin, etc.
    {
        scaledXP = std::min(scaledXP, 50u);   // Enforce 50 XP cap for low-level items
    }

    // Return the final scaled and capped XP value
    return std::min(scaledXP, MAX_EXPERIENCE_GAIN);
}

// Function to calculate bonus XP based on the player's skill vs the required skill
uint32 GetSkillBasedXP(uint32 baseXP, uint32 requiredSkill, uint32 currentSkill)
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

// Function to apply rarity-based multipliers for special items
float GetRarityMultiplier(uint32 itemId)
{
    // Define rarity multipliers for high-end gathering items
    const std::map<uint32, float> rarityMultipliers = {
        // Uncommon items (multiplier: 1.2)
        { 765, 1.2f },    // Silverleaf
        { 2447, 1.2f },   // Peacebloom
        { 3355, 1.2f },   // Wild Steelbloom
        { 3356, 1.2f },   // Kingsblood
        { 2318, 1.2f },   // Light Leather
        { 2319, 1.2f },   // Medium Leather
        { 4234, 1.2f },   // Heavy Leather
        { 2934, 1.2f },   // Ruined Leather Scraps
        { 15415, 1.2f },  // Blue Dragonscale
        { 29539, 1.2f },  // Cobra Scales

        // Rare items (multiplier: 1.5)
        { 13463, 1.5f },  // Dreamfoil
        { 8836, 1.5f },   // Arthas' Tears
        { 8838, 1.5f },   // Sungrass
        { 15417, 1.5f },  // Devilsaur Leather
        { 15416, 1.5f },  // Black Dragonscale
        { 7911, 1.5f },   // Truesilver Ore
        { 10620, 1.5f },  // Thorium Ore
        { 44128, 1.5f },  // Arctic Fur
        { 15414, 1.5f },  // Red Dragonscale
        { 13754, 1.5f },  // Raw Glossy Mightfish (Fishing)
        { 13758, 1.5f },  // Raw Redgill (Fishing)

        // Super rare items (multiplier: 2.0+)
        { 13467, 3.0f },  // Black Lotus (super rare)
        { 8845, 2.5f },   // Ghost Mushroom (super rare)
        { 11382, 2.0f },  // Blood of the Mountain
        { 7910, 2.0f },   // Star Ruby
        { 12364, 2.0f },  // Huge Emerald
        { 12800, 2.0f },  // Azerothian Diamond
        { 12363, 2.5f },  // Arcane Crystal
        { 22203, 2.0f },  // Large Obsidian Shard
        { 15419, 2.5f },  // Pristine Hide of the Beast
        { 15410, 3.0f },  // Scale of Onyxia
        { 15423, 2.5f },  // Brilliant Chromatic Scale
        { 13926, 3.0f }   // Golden Pearl (Fishing)
    };

    // Check if the item has a rarity multiplier
    auto rarity = rarityMultipliers.find(itemId);
    if (rarity != rarityMultipliers.end())
        return rarity->second;

    // Default multiplier for common items
    return 1.0f;
}
