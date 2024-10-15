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
    {
        uint32 playerLevel = player->GetLevel();

        // Apply diminishing returns if the player's skill is much higher than the required skill
        uint32 skillDifference = currentSkill > requiredSkill ? currentSkill - requiredSkill : 0;
        float skillMultiplier = skillDifference > 50 ? 0.55f : 1.0f - (skillDifference * 0.017f); // Increase diminishing returns slightly

        // Apply a scaling formula to reduce XP based on the player's level
        float levelMultiplier = 0.5f + (0.5f * (1.0f - static_cast<float>(playerLevel) / GATHERING_MAX_LEVEL));

        // Calculate final XP with scaling and diminishing returns
        uint32 scaledXP = static_cast<uint32>(baseXP * skillMultiplier * levelMultiplier);

        // Ensure a minimum XP for high-level items like Dreamfoil
        if (baseXP > 250) // Base XP threshold for high-level items
        {
            scaledXP = std::max(scaledXP, MIN_EXPERIENCE_GAIN_HIGH_LEVEL_ITEM);
        }

        // Cap the XP to avoid overflow or unreasonable values
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
    uint32 GetGatheringBaseXP(uint32 itemId)
    {
        // Mining item XP values
        const std::map<uint32, uint32> miningItemsXP = {
            { 2770, 50 },  // Copper Ore
            { 2771, 100 }, // Tin Ore
            { 2772, 200 }, // Iron Ore (Higher level ore)
            { 10620, 325 } // Thorium Ore (Slight reduction)
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

    // Check if item is related to gathering (mining or herbalism)
    bool IsGatheringItem(uint32 itemId)
    {
        // Mining item XP values
        const std::set<uint32> gatheringItems = {
            2770, // Copper Ore
            2771, // Tin Ore
            2772, // Iron Ore
            10620, // Thorium Ore
            765,   // Silverleaf
            2447,  // Peacebloom
            8836,  // Arthas' Tears
            13463  // Dreamfoil
        };

        return gatheringItems.find(itemId) != gatheringItems.end();
    }

    // Hook for Mining and Herbalism (Looting a resource node)
    void OnLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid) override
    {
        uint32 itemId = item->GetEntry();

        // Ensure the looted item is part of a gathering profession (herbalism/mining)
        if (!IsGatheringItem(itemId))
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