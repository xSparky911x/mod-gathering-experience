#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "GameEventMgr.h"
#include "SkillDiscovery.h"
#include "Config.h"
#include "Chat.h"
#include <iostream> // Include for logging

// Define the maximum level for gathering scaling
const uint32 GATHERING_MAX_LEVEL = 80;
const uint32 MAX_EXPERIENCE_GAIN = 2500;
const uint32 MIN_EXPERIENCE_GAIN_HIGH_LEVEL_ITEM = 10;

class GatheringExperienceModule : public PlayerScript, public WorldScript
{
public:
    GatheringExperienceModule() : PlayerScript("GatheringExperienceModule"), WorldScript("GatheringExperienceModule") { }

    // OnWorldInitialize hook to log that the module is loaded
    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        enabled = sConfigMgr->GetOption<bool>("GatheringExperience.Enable", true);
        if (!enabled)
        {
            LOG_INFO("module", "Gathering Experience Module is disabled by config.");
            return;
        }

        LOG_INFO("module", "Gathering Experience Module Loaded");
    }

    void OnLogin(Player* player) override
    {
        if (!enabled)
        return;

        if (sConfigMgr->GetOption<bool>("GatheringExperience.Announce", true))
        {
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Gathering Experience|r module by Thaxtin.");
        }
    }

    // Function to calculate scaled experience based on player level and item base XP
    uint32 CalculateExperience(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill, uint32 itemId)
    {
        if (!enabled)
            return 0;

        uint32 playerLevel = player->GetLevel();

        // No XP gain for characters at or above the max level
        if (playerLevel >= GATHERING_MAX_LEVEL)
            return 0;

        // Calculate skill difference and apply diminishing returns
        uint32 skillDifference = (currentSkill > requiredSkill) ? (currentSkill - requiredSkill) : 0;
        float skillMultiplier = 1.0f - (skillDifference * 0.01f); // 1% reduction per skill point over
        skillMultiplier = std::max(skillMultiplier, 0.1f); // Ensure the skill multiplier does not go below 0.1

        // Apply level scaling formula
        float levelMultiplier = (requiredSkill <= 150) ? 1.0f : 
                                (0.5f + (0.5f * (1.0f - static_cast<float>(playerLevel) / GATHERING_MAX_LEVEL)));

        // Calculate final XP with scaling and diminishing returns
        uint32 scaledXP = static_cast<uint32>(baseXP * skillMultiplier * levelMultiplier);

        // Debug logging to see values in the console
        LOG_INFO("module", "CalculateExperience - ItemId: {}, BaseXP: {}, SkillDifference: {}, SkillMultiplier: {:.2f}, LevelMultiplier: {:.2f}, ScaledXP: {}", 
                itemId, baseXP, skillDifference, skillMultiplier, levelMultiplier, scaledXP);

        // Ensure the final XP is at least the minimum XP gain
        scaledXP = std::max(scaledXP, MIN_EXPERIENCE_GAIN_HIGH_LEVEL_ITEM);

        // Return the final scaled and capped XP value
        return std::min(scaledXP, MAX_EXPERIENCE_GAIN);
    }

    uint32 GetSkillBasedXP(uint32 baseXP, uint32 requiredSkill, uint32 currentSkill)
    {
        if (!enabled)
        return 0;
        
        // Log the input values
        std::cout << "baseXP: " << baseXP << ", requiredSkill: " << requiredSkill << ", currentSkill: " << currentSkill << std::endl;

        if (currentSkill < requiredSkill)
        {
            // Increase XP for challenging nodes (player's skill is lower than required)
            uint32 bonusXP = (requiredSkill - currentSkill) * 2; // XP bonus scaling
            uint32 totalXP = baseXP + bonusXP;
            std::cout << "Bonus XP: " << bonusXP << ", Total XP: " << totalXP << std::endl;
            return totalXP;
        }
        else
        {
            // Penalize XP if the node is too easy but keep a minimum XP threshold
            uint32 penaltyXP = (currentSkill - requiredSkill) * 1; // XP penalty scaling
            uint32 totalXP = baseXP > penaltyXP ? baseXP - penaltyXP : MIN_EXPERIENCE_GAIN_HIGH_LEVEL_ITEM; // Ensure at least the minimum XP is given
            std::cout << "Penalty XP: " << penaltyXP << ", Total XP: " << totalXP << std::endl;
            return totalXP;
        }
    }


    // Function to get base XP and required skill for different mining, herbalism items, and skinning items
    std::pair<uint32, uint32> GetGatheringBaseXPAndRequiredSkill(uint32 itemId)
    {
        if (!enabled)
        return {0, 0};
        
        // Mining item XP and required skill values
        const std::map<uint32, std::pair<uint32, uint32>> miningItemsXP = {
            { 2770, {50, 1} },    // Copper Ore
            { 2771, {100, 65} },   // Tin Ore
            { 2772, {200, 125} },  // Iron Ore
            { 2775, {150, 75} },   // Silver Ore
            { 2776, {250, 155} },  // Gold Ore
            { 3858, {400, 175} },  // Mithril Ore
            { 7911, {350, 230} },  // Truesilver Ore
            { 10620, {400, 250} }, // Thorium Ore
            { 23424, {425, 275} }, // Fel Iron Ore
            { 23425, {450, 325} }, // Adamantite Ore
            { 23426, {475, 375} }, // Khorium Ore
            { 36909, {500, 350} }, // Cobalt Ore
            { 36910, {525, 450} }, // Titanium Ore
            { 36912, {550, 400} }, // Saronite Ore
            { 18562, {600, 275} }, // Elementium Ore
            { 22202, {575, 275} }, // Small Obsidian Shard
            { 22203, {625, 300} }, // Large Obsidian Shard
            { 12800, {650, 300} }, // Azerothian Diamond
            { 19774, {675, 300} }, // Souldarite
            { 12364, {700, 300} }, // Huge Emerald
            { 12363, {750, 300} }, // Arcane Crystal
            { 12799, {675, 300} }, // Large Opal
            { 12361, {700, 300} }, // Blue Sapphire
            { 7910, {625, 300} },  // Star Ruby
            { 11754, {600, 300} }, // Black Diamond
            { 7909, {650, 300} },  // Aquamarine
            { 11382, {800, 300} }, // Blood of the Mountain
            { 3864, {300, 300} },  // Citrine
            { 1705, {200, 300} },  // Lesser Moonstone
            { 1529, {175, 300} },  // Jade
            { 1210, {100, 300} },  // Shadowgem
            { 1206, {150, 300} },  // Moss Agate
            { 774, {50, 300} },    // Malachite
            { 818, {75, 300} },    // Tigerseye
            { 37701, {300, 300} }, // Crystallized Earth
            { 37702, {325, 300} }, // Crystallized Fire
            { 37703, {350, 300} }, // Crystallized Shadow
            { 37704, {375, 300} }, // Crystallized Life
            { 37705, {400, 300} }, // Crystallized Water
            { 7912, {125, 300} },  // Solid Stone
            { 2838, {75, 300} },   // Heavy Stone
            { 2836, {50, 300} },   // Coarse Stone
            { 12365, {100, 300} }, // Dense Stone
            { 11370, {375, 300} }, // Dark Iron Ore
        };

        // Herbalism item XP and required skill values
        const std::map<uint32, std::pair<uint32, uint32>> herbalismItemsXP = {
            { 765, {360, 1} },     // Silverleaf
            { 2447, {360, 1} },    // Peacebloom
            { 2449, {540, 15} },   // Earthroot
            { 785, {720, 50} },    // Mageroyal
            { 2450, {900, 70} },   // Briarthorn
            { 2452, {1080, 115} }, // Swiftthistle
            { 2453, {1260, 100} }, // Bruiseweed
            { 3820, {1440, 85} },  // Stranglekelp
            { 3355, {1620, 115} }, // Wild Steelbloom
            { 3356, {1800, 125} }, // Kingsblood
            { 3357, {1980, 150} }, // Liferoot
            { 3369, {2160, 120} }, // Grave Moss
            { 3818, {2340, 160} }, // Fadeleaf
            { 3819, {2520, 195} }, // Wintersbite
            { 3821, {2700, 170} }, // Goldthorn
            { 3358, {2880, 185} }, // Khadgar's Whisker
            { 4625, {3240, 205} }, // Firebloom
            { 8831, {3420, 210} }, // Purple Lotus
            { 8836, {3600, 220} }, // Arthas' Tears
            { 8838, {3780, 230} }, // Sungrass
            { 8839, {3960, 235} }, // Blindweed
            { 8845, {4140, 245} }, // Ghost Mushroom
            { 8846, {4320, 250} }, // Gromsblood
            { 13463, {4680, 270} }, // Dreamfoil
            { 13464, {4860, 260} }, // Golden Sansam
            { 13465, {5040, 280} }, // Mountain Silversage
            { 13466, {5220, 285} }, // Plaguebloom
            { 13467, {5400, 300} }, // Black Lotus
            { 22785, {5760, 300} }, // Felweed
            { 22786, {5940, 315} }, // Dreaming Glory
            { 22787, {6120, 325} }, // Ragveil
            { 22789, {6300, 325} }, // Terocone
            { 22790, {6480, 340} }, // Ancient Lichen
            { 22791, {6660, 350} }, // Netherbloom
            { 22792, {6840, 365} }, // Nightmare Vine
            { 22793, {7020, 375} }, // Mana Thistle
            { 36901, {7200, 350} }, // Goldclover
            { 36903, {7380, 400} }, // Adder's Tongue
            { 36904, {7560, 375} }, // Tiger Lily
            { 36905, {7740, 425} }, // Lichbloom
            { 36906, {7920, 435} }, // Icethorn
            { 36907, {8100, 400} }, // Talandra's Rose
        };

        // Skinning item XP and required skill values (Leathers, hides, and scales)
        const std::map<uint32, std::pair<uint32, uint32>> skinningItemsXP = {
            { 2318, {50, 1} },    // Light Leather
            { 2319, {100, 100} },   // Medium Leather
            { 4234, {150, 150} },   // Heavy Leather
            { 4304, {200, 200} },   // Thick Leather
            { 8170, {250, 250} },   // Rugged Leather
            { 15417, {300, 300} },  // Devilsaur Leather
            { 15415, {325, 325} },  // Blue Dragonscale
            { 15416, {325, 325} },  // Black Dragonscale
            { 21887, {350, 350} },  // Knothide Leather
            { 25700, {375, 375} },  // Fel Scales
            { 25707, {400, 400} },  // Fel Hide
            { 33568, {425, 425} },  // Borean Leather
            { 38425, {450, 450} },  // Heavy Borean Leather
            { 44128, {475, 475} },  // Arctic Fur (Rare)
            { 17012, {375, 375} },  // Core Leather (Molten Core, rare)
            { 29539, {400, 400} },  // Cobra Scales
            { 29547, {400, 400} },  // Wind Scales
            { 2934, {25, 25} },    // Ruined Leather Scraps
            { 783, {75, 75} },     // Light Hide
            { 4232, {125, 125} },   // Medium Hide
            { 4235, {175, 175} },   // Heavy Hide
            { 8169, {225, 225} },   // Thick Hide
            { 8171, {275, 275} },   // Rugged Hide
            { 6470, {100, 100} },   // Deviate Scale
            { 6471, {150, 150} },   // Perfect Deviate Scale
            { 5784, {125, 125} },   // Slimy Murloc Scale
            { 7286, {175, 175} },   // Black Whelp Scale
            { 7287, {200, 200} },   // Red Whelp Scale
            { 5785, {225, 225} },   // Thick Murloc Scale
            { 8154, {250, 250} },   // Scorpid Scale
            { 15408, {300, 300} },  // Heavy Scorpid Scale
            { 15414, {350, 350} },  // Red Dragonscale
            { 15412, {375, 375} },  // Green Dragonscale
            { 15417, {400, 400} },  // Blue Dragonscale
            { 15416, {425, 425} },  // Black Dragonscale (Elite)
            { 15417, {450, 450} },  // Devilsaur Leather (Elite)
            { 15419, {475, 475} },  // Pristine Hide of the Beast
            { 15410, {500, 500} },  // Scale of Onyxia
            { 15423, {525, 525} }   // Brilliant Chromatic Scale
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
        return {0, 0}; // Default to 0 XP and 0 required skill for non-gathering items
    }

    // Function to apply rarity-based multipliers for special items
    float GetRarityMultiplier(uint32 itemId)
    {
        if (!enabled)
        return 0;
        
        // Define rarity multipliers for high-end gathering items
        const std::map<uint32, float> rarityMultipliers = {
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
        auto [baseXP, requiredSkill] = GetGatheringBaseXPAndRequiredSkill(itemId);
        return baseXP > 0;
    }

    void OnLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid /*lootguid*/) override
    {
        uint32 itemId = item->GetEntry();  // Get the item ID

        // Ensure the looted item is part of a gathering profession (herbalism/mining/skinning)
        auto [baseXP, requiredSkill] = GetGatheringBaseXPAndRequiredSkill(itemId);
        if (baseXP == 0)
            return; // Skip non-gathering items

        // Get the player's current skill level in the appropriate profession
        uint32 currentSkill = 0;

        // Determine if this is a mining or herbalism node
        if (player->HasSkill(SKILL_MINING))
        {
            currentSkill = player->GetSkillValue(SKILL_MINING);
        }
        else if (player->HasSkill(SKILL_HERBALISM))
        {
            currentSkill = player->GetSkillValue(SKILL_HERBALISM);
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
        
        // Debug logging to see values in the console
        LOG_INFO("module", "OnLootItem - ItemId: {}, BaseXP: {}, RequiredSkill: {}, CurrentSkill: {}, SkillBasedXP: {}, RarityMultiplier: {:.2f}, FinalXP: {}, ScaledXP: {}", 
                itemId, baseXP, requiredSkill, currentSkill, skillBasedXP, rarityMultiplier, finalXP, xp);

        // Give the player the experience with the capped value
        player->GiveXP(xp, nullptr);
    }

    // Hook for Skinning (When the player skins a creature)
    void OnKillCreature(Player* player, Creature* creature)
    {
        if (!enabled)
            return;

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
    private:
    bool enabled;
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