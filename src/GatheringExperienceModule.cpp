#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "GameEventMgr.h"
#include "SkillDiscovery.h"
#include "Config.h" // For loading configuration values

// Define the maximum level for gathering scaling
const uint32 GATHERING_MAX_LEVEL = 80;
const uint32 MAX_EXPERIENCE_GAIN = 400; // Keeping the XP cap at 400
const uint32 MAX_EXPERIENCE_GAIN = 300; // Adjusted the XP cap to 300
const uint32 MIN_EXPERIENCE_GAIN_HIGH_LEVEL_ITEM = 30; // Keeping the minimum XP for high-level items at 30

class GatheringExperienceModule : public PlayerScript, public WorldScript
{
public:
GatheringExperienceModule() : PlayerScript("GatheringExperienceModule"), WorldScript("GatheringExperienceModule") { }

// OnWorldInitialize hook to log that the module is loaded
void OnBeforeConfigLoad(bool /*reload*/) override
{
LOG_INFO("module", "Gathering Experience Module Loaded");
}

    // Function to calculate scaled experience based on player level and item level
    uint32 CalculateExperience(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill)
    // Function to calculate scaled experience based on player level and item base XP
    uint32 CalculateExperience(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill, uint32 itemId)
{
uint32 playerLevel = player->GetLevel();

        // Apply diminishing returns if the player's skill is much higher than the required skill
        uint32 skillDifference = currentSkill > requiredSkill ? currentSkill - requiredSkill : 0;
        float skillMultiplier = skillDifference > 50 ? 0.55f : 1.0f - (skillDifference * 0.017f); // Increase diminishing returns slightly
        // Apply stronger diminishing returns for being over-skilled
        uint32 skillDifference = (currentSkill > requiredSkill) ? (currentSkill - requiredSkill) : 0;
        float skillMultiplier = 1.0f - (skillDifference * 0.02f); // Slight penalty: 2% reduction per skill point over

// Apply a scaling formula to reduce XP based on the player's level
float levelMultiplier = 0.5f + (0.5f * (1.0f - static_cast<float>(playerLevel) / GATHERING_MAX_LEVEL));

// Calculate final XP with scaling and diminishing returns
uint32 scaledXP = static_cast<uint32>(baseXP * skillMultiplier * levelMultiplier);

        // Ensure a minimum XP for high-level items like Dreamfoil
        if (baseXP > 250) // Base XP threshold for high-level items
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
            scaledXP = std::max(scaledXP, MIN_EXPERIENCE_GAIN_HIGH_LEVEL_ITEM);
            scaledXP = std::max(scaledXP, 50u);  // Minimum XP of 50 for mid-level items
            scaledXP = std::min(scaledXP, 100u);  // Max XP cap of 100 for mid-level items
        }
        else if (baseXP <= 200)  // Low-level items like Copper, Tin, etc.
        {
            scaledXP = std::min(scaledXP, 50u);   // Enforce 50 XP cap for low-level items
}

        // Cap the XP to avoid overflow or unreasonable values
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

    // Function to get base XP for different mining or herbalism items
    // Function to get base XP for different mining ,herbalism items and skinning items
uint32 GetGatheringBaseXP(uint32 itemId)
{
// Mining item XP values
const std::map<uint32, uint32> miningItemsXP = {
{ 2770, 50 },    // Copper Ore
{ 2771, 100 },   // Tin Ore
{ 2772, 200 },   // Iron Ore
{ 2775, 150 },   // Silver Ore
{ 2776, 250 },   // Gold Ore
            { 3858, 300 },   // Mithril Ore
            { 3858, 400 },   // Mithril Ore
{ 7911, 350 },   // Truesilver Ore
{ 10620, 400 },  // Thorium Ore
{ 23424, 425 },  // Fel Iron Ore
{ 23425, 450 },  // Adamantite Ore
{ 23426, 475 },  // Khorium Ore
{ 36909, 500 },  // Cobalt Ore
{ 36910, 525 },  // Titanium Ore (Rare/High-level)
{ 36912, 550 },  // Saronite Ore
{ 18562, 600 },  // Elementium Ore (Classic, rare ore)
{ 22202, 575 },  // Small Obsidian Shard
{ 22203, 625 },  // Large Obsidian Shard
{ 12800, 650 },  // Azerothian Diamond
{ 19774, 675 },  // Souldarite
{ 12364, 700 },  // Huge Emerald
{ 12363, 750 },  // Arcane Crystal
{ 12799, 675 },  // Large Opal
{ 12361, 700 },  // Blue Sapphire
{ 7910, 625 },   // Star Ruby
{ 11754, 600 },  // Black Diamond
{ 7909, 650 },   // Aquamarine (Added)
{ 11382, 800 },  // Blood of the Mountain (Added)
{ 3864, 300 },   // Citrine (Added)
{ 1705, 200 },   // Lesser Moonstone (Added)
{ 1529, 175 },   // Jade (Added)
{ 1210, 100 },   // Shadowgem (Added)
{ 1206, 150 },   // Moss Agate (Added)
{ 774, 50 },     // Malachite (Added)
{ 818, 75 },     // Tigerseye (Added)
{ 37701, 300 },  // Crystallized Earth
{ 37702, 325 },  // Crystallized Fire
{ 37703, 350 },  // Crystallized Shadow
{ 37704, 375 },  // Crystallized Life
{ 37705, 400 },  // Crystallized Water
{ 7912, 125 },   // Solid Stone
{ 2838, 75 },    // Heavy Stone
{ 2836, 50 },    // Coarse Stone
            { 12365, 200 },  // Dense Stone
            { 12365, 100 },  // Dense Stone
{ 11370, 375 },  // Dark Iron Ore
};


// Herbalism item XP values
const std::map<uint32, uint32> herbalismItemsXP = {
{ 765, 50 },     // Silverleaf
{ 2447, 75 },    // Peacebloom
{ 2449, 100 },   // Earthroot
{ 785, 100 },    // Mageroyal
{ 2450, 125 },   // Briarthorn
{ 2452, 150 },   // Swiftthistle
{ 2453, 175 },   // Bruiseweed
{ 3820, 200 },   // Stranglekelp
{ 3355, 225 },   // Wild Steelbloom
{ 3356, 250 },   // Kingsblood
{ 3357, 275 },   // Liferoot
{ 3369, 300 },   // Grave Moss
{ 3818, 325 },   // Fadeleaf
{ 3819, 350 },   // Wintersbite
{ 3821, 375 },   // Goldthorn
{ 3358, 400 },   // Khadgar's Whisker
{ 4625, 450 },   // Firebloom
{ 8831, 475 },   // Purple Lotus
{ 8836, 500 },   // Arthas' Tears (Rare herb)
{ 8838, 525 },   // Sungrass
{ 8839, 550 },   // Blindweed
{ 8845, 575 },   // Ghost Mushroom (Rare herb)
{ 8846, 600 },   // Gromsblood
{ 8153, 625 },   // Wildvine (Added)
{ 13463, 650 },  // Dreamfoil
{ 13464, 675 },  // Golden Sansam
{ 13465, 700 },  // Mountain Silversage
{ 13466, 725 },  // Plaguebloom
{ 13467, 750 },  // Black Lotus (Super rare)
{ 19726, 775 },  // Bloodvine (Added)
{ 22785, 800 },  // Felweed
{ 22786, 825 },  // Dreaming Glory
{ 22787, 850 },  // Ragveil
{ 22789, 875 },  // Terocone
{ 22790, 900 },  // Ancient Lichen
{ 22791, 925 },  // Netherbloom
{ 22792, 950 },  // Nightmare Vine
{ 22793, 975 },  // Mana Thistle
{ 36901, 1000 }, // Goldclover
{ 36903, 1025 }, // Adder's Tongue
{ 36904, 1050 }, // Tiger Lily
{ 36905, 1075 }, // Lichbloom
{ 36906, 1100 }, // Icethorn
{ 36907, 1125 }  // Talandra's Rose
};

        // Skinning item XP values (Leathers and hides)
        // Skinning item XP values (Leathers, hides, and scales)
const std::map<uint32, uint32> skinningItemsXP = {
{ 2318, 50 },    // Light Leather
{ 2319, 100 },   // Medium Leather
{ 4234, 150 },   // Heavy Leather
{ 4304, 200 },   // Thick Leather
{ 8170, 250 },   // Rugged Leather
{ 15417, 300 },  // Devilsaur Leather
{ 15415, 325 },  // Blue Dragonscale
{ 15416, 325 },  // Black Dragonscale
{ 21887, 350 },  // Knothide Leather
{ 25700, 375 },  // Fel Scales
{ 25707, 400 },  // Fel Hide
{ 33568, 425 },  // Borean Leather
{ 38425, 450 },  // Heavy Borean Leather
{ 44128, 475 },  // Arctic Fur (Rare)
{ 17012, 375 },  // Core Leather (Molten Core, rare)
{ 29539, 400 },  // Cobra Scales
{ 29547, 400 },  // Wind Scales
            { 2934, 25 },    // Ruined Leather Scraps
            { 783, 75 },     // Light Hide
            { 4232, 125 },   // Medium Hide
            { 4235, 175 },   // Heavy Hide
            { 8169, 225 },   // Thick Hide
            { 8171, 275 },   // Rugged Hide
            { 6470, 100 },   // Deviate Scale
            { 6471, 150 },   // Perfect Deviate Scale
            { 5784, 125 },   // Slimy Murloc Scale
            { 7286, 175 },   // Black Whelp Scale
            { 7287, 200 },   // Red Whelp Scale
            { 5785, 225 },   // Thick Murloc Scale
            { 8154, 250 },   // Scorpid Scale
            { 15408, 300 },  // Heavy Scorpid Scale
            { 15414, 350 },  // Red Dragonscale
            { 15412, 375 },  // Green Dragonscale
            { 15417, 400 },  // Blue Dragonscale
            { 15416, 425 },  // Black Dragonscale (Elite)
            { 15417, 450 },  // Devilsaur Leather (Elite)
            { 15419, 475 },  // Pristine Hide of the Beast
            { 15410, 500 },  // Scale of Onyxia
            { 15423, 525 }   // Brilliant Chromatic Scale
};


// Check if the item is a mining item
auto miningXP = miningItemsXP.find(itemId);
if (miningXP != miningItemsXP.end())
return miningXP->second;

// Check if the item is a herbalism item
auto herbXP = herbalismItemsXP.find(itemId);
if (herbXP != herbalismItemsXP.end())
return herbXP->second;

// Check if the item is a skinning item
auto skinningXP = skinningItemsXP.find(itemId);
if (skinningXP != skinningItemsXP.end())
return skinningXP->second;

// Default base XP if the item is not found in the above tables
        return 50; // Default to a low base XP
        return 0; // Default to 0 XP for non-gathering items
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
            { 15423, 2.5f }   // Brilliant Chromatic Scale
};

// Check if the item has a rarity multiplier
auto rarity = rarityMultipliers.find(itemId);
if (rarity != rarityMultipliers.end())
return rarity->second;

// Default multiplier for common items
return 1.0f;
}

// Check if the item is related to gathering (mining, herbalism, or skinning)
bool IsGatheringItem(uint32 itemId)
{
// Check if the item exists in mining, herbalism, or skinning XP maps
return GetGatheringBaseXP(itemId) > 0;
}

    // Hook for Mining, Herbalism, and Skinning (Looting a resource node or skinning)
    void OnLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid) override
    // Hook for Mining and Herbalism (Looting a resource node)
    void OnLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid /*lootguid*/) override
{
        uint32 itemId = item->GetEntry();
        uint32 itemId = item->GetEntry();  // Get the item ID

// Ensure the looted item is part of a gathering profession (herbalism/mining/skinning)
        if (!IsGatheringItem(itemId))
        uint32 baseXP = GetGatheringBaseXP(itemId);
        if (baseXP == 0)
return; // Skip non-gathering items

// Get the player's current skill level in the appropriate profession
uint32 currentSkill = 0;
uint32 requiredSkill = 0;

// Determine if this is a mining or herbalism node
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
else
{
return; // No gathering skill, exit
}

        // Get base XP for the specific item
        uint32 baseXP = GetGatheringBaseXP(itemId);

// Apply the skill-based XP bonus/penalty
uint32 skillBasedXP = GetSkillBasedXP(baseXP, requiredSkill, currentSkill);

// Apply rarity multiplier
float rarityMultiplier = GetRarityMultiplier(itemId);
uint32 finalXP = static_cast<uint32>(skillBasedXP * rarityMultiplier);

        // Calculate scaled experience based on player's level and skill difference
        uint32 xp = CalculateExperience(player, finalXP, requiredSkill, currentSkill);
        // Calculate scaled experience based on player's level and skill difference, now with itemId
        uint32 xp = CalculateExperience(player, finalXP, requiredSkill, currentSkill, itemId);

// Give the player the experience with the capped value
player->GiveXP(xp, nullptr);
}

// Hook for Skinning (When the player skins a creature)
void OnKillCreature(Player* player, Creature* creature)
{
// Check if the GatheringExperience module is enabled
if (!sConfigMgr->GetOption<bool>("GatheringExperience.Enable", true))
return; // Exit if the module is disabled

// Check if the player can skin the creature and if it's a beast (skinnable creatures)
        if (player->HasSkill(SKILL_SKINNING) && creature->GetCreatureType() == CREATURE_TYPE_BEAST)
        if (player->HasSkill(SKILL_SKINNING) && creature->GetCreatureType() == CREATURE_TYPE_BEAST && creature->loot.isLooted())
{
// Get the player's current skinning skill
uint32 currentSkill = player->GetSkillValue(SKILL_SKINNING);
uint32 requiredSkill = creature->GetLevel() * 5; // Simplified, adjust as needed based on actual skinning mechanics

// Base XP for skinning, adjustable based on creature level
uint32 baseXP = requiredSkill / 2; // Example logic for skinning XP

// Apply skill-based XP bonus/penalty
uint32 skillBasedXP = GetSkillBasedXP(baseXP, requiredSkill, currentSkill);

            // Calculate scaled experience based on player's level and skill difference
            uint32 xp = CalculateExperience(player, skillBasedXP, requiredSkill, currentSkill);
            // Apply rarity multiplier for skinning items
            float rarityMultiplier = 1.5f; // Adjust as needed
            uint32 finalXP = static_cast<uint32>(skillBasedXP * rarityMultiplier);

            // Pass 0 as itemId since there's no specific item for skinning
            uint32 xp = CalculateExperience(player, finalXP, requiredSkill, currentSkill, 0);

// Give the player the experience with the capped value
player->GiveXP(xp, nullptr);
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