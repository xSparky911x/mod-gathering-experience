#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "GameEventMgr.h"
#include "SkillDiscovery.h"
#include "Config.h"
#include "Chat.h"

// Define the maximum level for gathering scaling
const uint32 GATHERING_MAX_LEVEL = 80;
const uint32 MAX_EXPERIENCE_GAIN = 300; // Adjusted the XP cap to 300

class GatheringExperienceModule : public PlayerScript, public WorldScript
{
public:
    GatheringExperienceModule() : PlayerScript("GatheringExperienceModule"), WorldScript("GatheringExperienceModule") { }

    // OnWorldInitialize hook to log that the module is loaded
    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        // Check if the GatheringExperience module is enabled
        if (!sConfigMgr->GetOption<bool>("GatheringExperience.Enable", true))
            return; // Exit if the module is disabled
    
        LOG_INFO("module", "Gathering Experience Module Loaded");
    }

    void OnLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("GatheringExperience.Announce", true))
        {
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Gathering Experience|r module by Thaxtin.");
        }
    }

private:
    // Function to check if the module is enabled
    bool IsModuleEnabled() const
    {
        return sConfigMgr->GetOption<bool>("GatheringExperience.Enable", true);
    }

    // Function to calculate scaled experience based on player level and item base XP
    uint32 CalculateExperience(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill, uint32 itemId)
    {
        if (!IsModuleEnabled()) return 0; // Exit if the module is disabled

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
        if (!IsModuleEnabled()) return 0; // Exit if the module is disabled

        if (currentSkill < requiredSkill)
        {
            // Increase XP for challenging nodes (player's skill is lower than required)
            uint32 bonusXP = (requiredSkill - currentSkill) * 2; // XP bonus scaling
            return baseXP + bonusXP;
        }
        else
        {
            // Penalize XP if the node is too easy but keep a minimum XP threshold
            uint32 penaltyXP = (currentSkill - requiredSkill) * 0.5; // Reduced penalty scaling
            return std::max(baseXP - penaltyXP, 10u); // Ensure at least 10 XP is given
        }
    }

    // Function to get base XP for different mining, herbalism, skinning, and fishing items
    uint32 GetGatheringBaseXP(uint32 itemId)
    {
        if (!IsModuleEnabled()) return 0; // Exit if the module is disabled

        // Define a map for mining items with base XP and required skill
        const std::map<uint32, std::pair<uint32, uint32>> miningItems = {
            { 2770, {50, 1} },    // Copper Ore
            { 774, {50, 1} },     // Malachite
            { 2836, {50, 1} },    // Coarse Stone
            { 818, {75, 1} },     // Tigerseye
            { 2838, {75, 1} },    // Heavy Stone
            { 2771, {100, 65} },  // Tin Ore
            { 2776, {150, 65} },  // Incendicite Ore
            { 2775, {150, 75} },  // Silver Ore
            { 1210, {100, 100} }, // Shadowgem
            { 12365, {100, 100} }, // Dense Stone
            { 1206, {150, 100} }, // Moss Agate
            { 1529, {175, 100} }, // Jade
            { 1705, {200, 100} }, // Lesser Moonstone
            { 3864, {300, 100} }, // Citrine
            { 37701, {300, 100} }, // Crystallized Earth
            { 37702, {325, 100} }, // Crystallized Fire
            { 37703, {350, 100} }, // Crystallized Shadow
            { 37704, {375, 100} }, // Crystallized Life
            { 37705, {400, 100} }, // Crystallized Water
            { 7912, {125, 100} }, // Solid Stone
            { 2772, {200, 125} }, // Iron Ore
            { 2776, {250, 155} }, // Gold Ore
            { 3858, {400, 175} }, // Mithril Ore
            { 7911, {350, 205} }, // Truesilver Ore
            { 11754, {600, 200} }, // Black Diamond
            { 19774, {675, 200} }, // Souldarite
            { 12799, {675, 200} }, // Large Opal
            { 7910, {625, 200} }, // Star Ruby
            { 7909, {650, 200} }, // Aquamarine
            { 12361, {700, 250} }, // Blue Sapphire
            { 12364, {700, 250} }, // Huge Emerald
            { 12363, {750, 250} }, // Arcane Crystal
            { 11370, {375, 230} }, // Dark Iron Ore
            { 10620, {400, 230} }, // Thorium Ore
            { 23424, {425, 275} }, // Fel Iron Ore
            { 23425, {450, 325} }, // Adamantite Ore
            { 36909, {500, 350} }, // Cobalt Ore
            { 36909, {500, 375} }, // Rich Cobalt Deposit
            { 23426, {475, 375} }, // Khorium Ore
            { 11382, {800, 375} }, // Blood of the Mountain
            { 36912, {550, 400} }, // Saronite Ore
            { 36912, {550, 425} }, // Rich Saronite Deposit
            { 36910, {525, 450} }, // Titanium Ore (Rare/High-level)
            { 36910, {525, 450} }, // Pure Saronite Deposit
            { 12800, {650, 300} }, // Azerothian Diamond
            { 22202, {575, 305} }, // Small Obsidian Shard
            { 22203, {625, 305} }, // Large Obsidian Shard
            { 18562, {600, 305} }, // Elementium Ore (Classic, rare ore)
        };

        // Define a map for herbalism items with base XP and required skill
        const std::map<uint32, std::pair<uint32, uint32>> herbalismItems = {
            { 765, {50, 1} },     // Silverleaf
            { 2447, {75, 1} },    // Peacebloom
            { 2449, {100, 15} },  // Earthroot
            { 785, {100, 50} },   // Mageroyal
            { 2450, {125, 70} },  // Briarthorn
            { 3820, {200, 85} },  // Stranglekelp
            { 2453, {175, 100} }, // Bruiseweed
            { 3355, {225, 115} }, // Wild Steelbloom
            { 3369, {300, 120} }, // Grave Moss
            { 3356, {250, 125} }, // Kingsblood
            { 3357, {275, 150} }, // Liferoot
            { 3818, {325, 160} }, // Fadeleaf
            { 3821, {375, 170} }, // Goldthorn
            { 3358, {400, 185} }, // Khadgar's Whisker
            { 3819, {350, 195} }, // Wintersbite
            { 4625, {450, 205} }, // Firebloom
            { 8831, {475, 210} }, // Purple Lotus
            { 8836, {500, 220} }, // Arthas' Tears
            { 8838, {525, 230} }, // Sungrass
            { 8839, {550, 235} }, // Blindweed
            { 8845, {575, 245} }, // Ghost Mushroom
            { 8846, {600, 250} }, // Gromsblood
            { 13464, {675, 260} }, // Golden Sansam
            { 13463, {650, 270} }, // Dreamfoil
            { 13465, {700, 280} }, // Mountain Silversage
            { 13466, {725, 285} }, // Plaguebloom
            { 13467, {750, 300} }, // Black Lotus
            { 22785, {800, 300} }, // Felweed
            { 22786, {825, 315} }, // Dreaming Glory
            { 22787, {850, 325} }, // Ragveil
            { 22789, {875, 325} }, // Terocone
            { 22790, {900, 340} }, // Ancient Lichen
            { 36901, {1000, 350} }, // Goldclover
            { 22791, {925, 350} }, // Netherbloom
            { 39970, {1150, 360} }, // Firethorn
            { 22792, {950, 365} }, // Nightmare Vine
            { 22793, {975, 375} }, // Mana Thistle
            { 36903, {1025, 375} }, // Adder's Tongue
            { 36904, {1050, 375} }, // Tiger Lily
            { 36907, {1125, 385} }, // Talandra's Rose
            { 39970, {1175, 400} }, // Frozen Herb
            { 39970, {1200, 415} }, // Frozen Herb
            { 36905, {1075, 425} }, // Lichbloom
            { 36906, {1100, 435} }, // Icethorn
            { 39970, {1225, 450 } } // Frost Lotus
        };

        // Define a map for skinning items with base XP and required skill
        const std::map<uint32, std::pair<uint32, uint32>> skinningItems = {
            { 2934, {1, 1} },     // Ruined Leather Scraps
            { 2318, {1, 1} },     // Light Leather
            { 783, {1, 1} },      // Light Hide
            { 33568, {350, 1} },  // Borean Leather
            { 33567, {350, 1} },  // Borean Leather Scraps
            { 38557, {375, 1} },  // Icy Dragonscale
            { 38558, {375, 1} },  // Nerubian Chitin
            { 38561, {385, 1} },  // Jormungar Scale
            { 44128, {400, 1} },  // Arctic Fur
            { 6470, {10, 10} },   // Deviate Scale
            { 6471, {10, 10} },   // Perfect Deviate Scale
            { 4232, {10, 10} },   // Medium Hide
            { 17057, {20, 20} },  // Shiny Fish Scales
            { 2319, {10, 50} },   // Medium Leather
            { 7286, {70, 70} },   // Black Whelp Scale
            { 4235, {100, 105} }, // Heavy Hide
            { 7287, {115, 115} }, // Red Whelp Scale
            { 4234, {100, 125} }, // Heavy Leather
            { 7392, {170, 170} }, // Green Whelp Scale
            { 4304, {150, 180} }, // Thick Leather
            { 8169, {150, 180} }, // Thick Hide
            { 8154, {185, 185} }, // Scorpid Scale
            { 15412, {285, 205} },// Green Dragonscale
            { 8170, {200, 215} }, // Rugged Leather
            { 8171, {200, 235} }, // Rugged Hide
            { 15415, {285, 250} },// Blue Dragonscale
            { 15416, {285, 250} },// Black Dragonscale
            { 15408, {260, 260} },// Heavy Scorpid Scale
            { 21887, {300, 265} },// Knothide Leather
            { 25649, {300, 265} },// Knothide Leather Scraps
            { 15417, {275, 270} },// Devilsaur Leather
            { 15414, {285, 285} },// Red Dragonscale
            { 25707, {350, 290} },// Fel Hide
            { 25700, {350, 295} }// Fel Scales
        };

        // Define a map for fishing items with base XP and required skill
        const std::map<uint32, std::pair<uint32, uint32>> fishingItems = {
            { 6303, {1, 1} },     // Raw Slitherskin Mackerel
            { 6291, {1, 1} },     // Raw Brilliant Smallfish
            { 6308, {25, 1} },    // Raw Bristle Whisker Catfish
            { 6361, {50, 1} },    // Raw Rainbow Fin Albacore
            { 7974, {50, 1} },    // Zesty Clam Meat
            { 6289, {75, 1} },    // Raw Longjaw Mud Snapper
            { 6317, {75, 1} },    // Raw Loch Frenzy
            { 6358, {100, 1} },   // Oily Blackmouth
            { 4603, {100, 1} },   // Raw Spotted Yellowtail
            { 6362, {125, 1} },   // Raw Rockscale Cod
            { 6359, {150, 1} },   // Firefin Snapper
            { 13754, {150, 1} },  // Raw Glossy Mightfish
            { 13758, {175, 1} },  // Raw Redgill
            { 13889, {175, 1} },  // Raw Whitescale Salmon
            { 13760, {200, 1} },  // Raw Sunscale Salmon
            { 13893, {200, 1} },  // Large Raw Mightfish
            { 13422, {225, 1} },  // Stonescale Eel
            { 13926, {250, 1} },  // Golden Pearl
            { 21071, {275, 1} },  // Raw Sagefish
            { 21153, {300, 1} },  // Raw Greater Sagefish
            { 27422, {300, 1} },  // Barbed Gill Trout
            { 27425, {300, 1} },  // Spotted Feltail
            { 27429, {325, 1} },  // Zangarian Sporefish
            { 27435, {350, 1} },  // Figluster's Mudfish
            { 27437, {350, 1} },  // Icefin Bluefish
            { 27438, {375, 1} },  // Golden Darter
            { 27439, {375, 1} },  // Furious Crawdad
            { 27516, {375, 1} },  // Enormous Barbed Gill Trout
            { 33823, {400, 1} },  // Bloodfin Catfish
            { 33824, {400, 1} },  // Crescent-Tail Skullfish
            { 41800, {400, 1} },  // Deep Sea Monsterbelly
            { 41801, {400, 1} },  // Moonglow Cuttlefish
            { 41802, {425, 1} },  // Imperial Manta Ray
            { 41803, {425, 1} },  // Rockfin Grouper
            { 41805, {450, 1} },  // Borean Man O' War
            { 41806, {450, 1} },  // Musselback Sculpin
            { 41807, {475, 1} },  // Dragonfin Angelfish
            { 41808, {475, 1} },  // Bonescale Snapper
            { 41809, {500, 1} },  // Glacial Salmon
            { 41810, {500, 1} },  // Fangtooth Herring
            { 43572, {425, 1} },  // Nettlefish
            { 45902, {450, 1} },  // Sewer Carp
        };

        // Check if the item is a mining item
        auto miningXP = miningItems.find(itemId);
        if (miningXP != miningItems.end())
            return miningXP->second.first;

        // Check if the item is a herbalism item
        auto herbXP = herbalismItems.find(itemId);
        if (herbXP != herbalismItems.end())
            return herbXP->second.first;

        // Check if the item is a skinning item
        auto skinningXP = skinningItems.find(itemId);
        if (skinningXP != skinningItems.end())
            return skinningXP->second.first;

        // Check if the item is a fishing item
        auto fishingXP = fishingItems.find(itemId);
        if (fishingXP != fishingItems.end())
        {
            LOG_INFO("module", "Fishing Item ID: %u, Base XP: %u", itemId, fishingXP->second.first); // Log fishing item base XP
            return fishingXP->second.first; // Ensure this returns the correct base XP for fishing items
        }

        // Default base XP if the item is not found in the above tables
        return 0; // Default to 0 XP for non-gathering items
    }

    // Function to apply rarity-based multipliers for special items
    float GetRarityMultiplier(uint32 itemId)
    {
        if (!IsModuleEnabled()) return 1.0f; // Default multiplier if the module is disabled

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

    // Check if the item is related to gathering (mining, herbalism, skinning, or fishing)
    bool IsGatheringItem(uint32 itemId)
    {
        if (!IsModuleEnabled()) return false; // Exit if the module is disabled

        // Check if the item exists in mining, herbalism, skinning, or fishing XP maps
        return GetGatheringBaseXP(itemId) > 0;
    }

    // Hook for Mining and Herbalism (Looting a resource node)
    void OnLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid /*lootguid*/) override
    {
        uint32 itemId = item->GetEntry();  // Get the item ID
        LOG_INFO("module", "OnLootItem called for Item ID: %u", itemId); // Confirm function call

        // Ensure the looted item is part of a gathering profession (herbalism/mining/skinning/fishing)
        uint32 baseXP = GetGatheringBaseXP(itemId);
        LOG_INFO("module", "Item ID: %u, Base XP: %u", itemId, baseXP); // Debug log for item ID and base XP
        if (baseXP == 0)
        {
            LOG_INFO("module", "Item ID: %u is not a gathering item.", itemId); // Log if item is not a gathering item
            return; // Skip non-gathering items
        }

        // Get the player's current skill level in the appropriate profession
        uint32 currentSkill = 0;
        uint32 requiredSkill = 0;

        // Determine if this is a mining, herbalism, or fishing node
        if (player->HasSkill(SKILL_MINING))
        {
            currentSkill = player->GetSkillValue(SKILL_MINING);
            requiredSkill = (itemId == 2770) ? 1 : (itemId == 10620) ? 200 : 50;
            LOG_INFO("module", "Mining: Current Skill: %u, Required Skill: %u", currentSkill, requiredSkill);
        }
        else if (player->HasSkill(SKILL_HERBALISM))
        {
            currentSkill = player->GetSkillValue(SKILL_HERBALISM);
            requiredSkill = (itemId == 765) ? 1 : (itemId == 13463) ? 150 : 50;
            LOG_INFO("module", "Herbalism: Current Skill: %u, Required Skill: %u", currentSkill, requiredSkill);
        }
        else if (player->HasSkill(SKILL_FISHING))
        {
            currentSkill = player->GetSkillValue(SKILL_FISHING);
            requiredSkill = 1; // Fishing does not typically have different skill requirements for specific fish
            LOG_INFO("module", "Fishing: Current Skill: %u, Required Skill: %u", currentSkill, requiredSkill);
        }
        else
        {
            LOG_INFO("module", "No gathering skill found for player.");
            return; // No gathering skill, exit
        }

        // Apply the skill-based XP bonus/penalty
        uint32 skillBasedXP = GetSkillBasedXP(baseXP, requiredSkill, currentSkill);
        LOG_INFO("module", "Skill Based XP: %u", skillBasedXP); // Debug log for skill-based XP

        // Apply rarity multiplier
        float rarityMultiplier = GetRarityMultiplier(itemId);
        uint32 finalXP = static_cast<uint32>(skillBasedXP * rarityMultiplier);
        LOG_INFO("module", "Final XP before scaling: %u, Rarity Multiplier: %.2f", finalXP, rarityMultiplier); // Debug log for final XP

        // Calculate Skill and Level Multipliers
        uint32 playerLevel = player->GetLevel();
        uint32 skillDifference = (currentSkill > requiredSkill) ? (currentSkill - requiredSkill) : 0;
        float skillMultiplier = 1.0f - (skillDifference * 0.02f); // Slight penalty: 2% reduction per skill point over
        float levelMultiplier = 0.5f + (0.5f * (1.0f - static_cast<float>(playerLevel) / GATHERING_MAX_LEVEL));

        // Calculate scaled experience
        uint32 scaledXP = static_cast<uint32>(finalXP * skillMultiplier * levelMultiplier);
        LOG_INFO("module", "ItemId: %u, BaseXP: %u, SkillMultiplier: %.2f, LevelMultiplier: %.2f, ScaledXP: %u", itemId, baseXP, skillMultiplier, levelMultiplier, scaledXP); // Debug log for scaled XP

        // Calculate final XP to give
        uint32 xp = std::min(scaledXP, MAX_EXPERIENCE_GAIN); // Cap the XP if necessary
        LOG_INFO("module", "Calculated XP to give: %u", xp); // Debug log for calculated XP

        // Give the player the experience with the capped value
        player->GiveXP(xp, nullptr);
    }

    // Hook for Skinning (When the player skins a creature)
    void OnKillCreature(Player* player, Creature* creature)
    {
        if (!IsModuleEnabled()) return; // Exit if the module is disabled

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
