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
const uint32 MAX_EXPERIENCE_GAIN = 25000;
const uint32 MIN_EXPERIENCE_GAIN = 10;

class GatheringExperienceModule : public PlayerScript, public WorldScript
{
private:
    std::map<uint32, std::pair<uint32, uint32>> miningItemsXP;
    std::map<uint32, std::pair<uint32, uint32>> herbalismItemsXP;
    std::map<uint32, std::pair<uint32, uint32>> skinningItemsXP;
    std::map<uint32, std::pair<uint32, uint32>> fishingItemsXP;

    bool enabled;

public:
    GatheringExperienceModule() : PlayerScript("GatheringExperienceModule"), WorldScript("GatheringExperienceModule") 
    {
        // Initialize skinningItemsXP map
        skinningItemsXP = {
            { 2934, {360, 1} },     // Ruined Leather Scraps (starter)
            { 2318, {400, 1} },     // Light Leather
            { 2319, {600, 100} },    // Medium Leather (increased from 450)
            { 4234, {800, 150} },    // Heavy Leather (increased from 500)
            { 4304, {1000, 200} },   // Thick Leather (increased from 600)
            { 8170, {1400, 250} },   // Rugged Leather (increased from 700)
            { 15417, {1800, 300} },  // Devilsaur Leather (increased from 800)
            { 15415, {2000, 325} },  // Blue Dragonscale (increased from 850)
            { 15416, {2000, 325} },  // Black Dragonscale
            { 21887, {2400, 350} },  // Knothide Leather
            { 25700, {2800, 375} },  // Fel Scales
            { 25707, {3000, 400} },  // Fel Hide
            { 33568, {3200, 425} },  // Borean Leather
            { 38425, {3400, 450} },  // Heavy Borean Leather
            { 44128, {3600, 475} },  // Arctic Fur
            // Special/rare items
            { 17012, {2800, 375} },  // Core Leather
            { 15419, {3000, 475} },  // Pristine Hide of the Beast
            // ... other special items ...
        };

        // Initialize other maps similarly
        miningItemsXP = {
            { 2770, {360, 1} },     // Copper Ore (starter)
            { 774, {360, 1} },      // Malachite
            { 818, {400, 1} },      // Tigerseye
            { 2771, {600, 65} },    // Tin Ore (increased from 450)
            { 1705, {700, 65} },    // Lesser Moonstone (increased from 500)
            { 1529, {700, 65} },    // Jade
            { 1210, {650, 65} },    // Shadowgem
            { 1206, {675, 65} },    // Moss Agate
            { 2836, {600, 65} },    // Coarse Stone
            { 2775, {825, 75} },    // Silver Ore (increased from 525)
            { 2772, {1000, 125} },  // Iron Ore (increased from 600)
            { 3864, {1200, 125} },  // Citrine (increased from 700)
            { 2838, {800, 125} },   // Heavy Stone
            { 2776, {1400, 155} },  // Gold Ore (increased from 800)
            { 7909, {1600, 155} },  // Aquamarine (increased from 900)
            { 3858, {1800, 175} },  // Mithril Ore
            { 7912, {1000, 175} },  // Solid Stone
            { 7911, {2000, 205} },  // Truesilver Ore
            { 10620, {2400, 230} }, // Thorium Ore
            { 12800, {2800, 230} }, // Azerothian Diamond
            { 12364, {2800, 230} }, // Huge Emerald
            { 12363, {3000, 230} }, // Arcane Crystal
            // ... higher tiers continue scaling up ...
        };

        herbalismItemsXP = {
            { 765, {360, 1} },     // Silverleaf
            { 2447, {360, 1} },    // Peacebloom
            { 2449, {540, 15} },   // Earthroot
            { 785, {720, 50} },    // Mageroyal
            { 2450, {900, 70} },   // Briarthorn
            { 3820, {1440, 85} },  // Stranglekelp
            { 2453, {1260, 100} }, // Bruiseweed
            { 2452, {1080, 115} }, // Swiftthistle
            { 3355, {1620, 115} }, // Wild Steelbloom
            { 3369, {2160, 120} }, // Grave Moss
            { 3356, {1800, 125} }, // Kingsblood
            { 3357, {1980, 150} }, // Liferoot
            { 3818, {2340, 160} }, // Fadeleaf
            { 3821, {2700, 170} }, // Goldthorn
            { 3358, {2880, 185} }, // Khadgar's Whisker
            { 3819, {2520, 195} }, // Wintersbite
            { 4625, {3240, 205} }, // Firebloom
            { 8831, {3420, 210} }, // Purple Lotus
            { 8836, {3600, 220} }, // Arthas' Tears
            { 8838, {3780, 230} }, // Sungrass
            { 8839, {3960, 235} }, // Blindweed
            { 8845, {4140, 245} }, // Ghost Mushroom
            { 8846, {4320, 250} }, // Gromsblood
            { 13464, {4860, 260} }, // Golden Sansam
            { 13463, {4680, 270} }, // Dreamfoil
            { 13465, {5040, 280} }, // Mountain Silversage
            { 13466, {5220, 285} }, // Plaguebloom
            { 13467, {5400, 300} }, // Black Lotus
            { 22785, {5760, 300} }, // Felweed
            { 22786, {5940, 315} }, // Dreaming Glory
            { 22787, {6120, 325} }, // Ragveil
            { 22789, {6300, 325} }, // Terocone
            { 22790, {6480, 340} }, // Ancient Lichen
            { 22791, {6660, 350} }, // Netherbloom
            { 36901, {7200, 350} }, // Goldclover
            { 22792, {6840, 365} }, // Nightmare Vine
            { 22793, {7020, 375} }, // Mana Thistle
            { 36904, {7560, 375} }, // Tiger Lily
            { 36903, {7380, 400} }, // Adder's Tongue
            { 36907, {8100, 400} }, // Talandra's Rose
            { 36905, {7740, 425} }, // Lichbloom
            { 36906, {7920, 435} }, // Icethorn
        };

        fishingItemsXP = {
            { 6291, {25, 0} },     // Raw Brilliant Smallfish (was 10)
            { 6308, {150, 0} },    // Raw Bristle Whisker Catfish (was 75)
            { 6317, {100, 0} },    // Raw Loch Frenzy (was 50)
            { 6358, {200, 0} },    // Oily Blackmouth (was 100)
            { 6359, {300, 0} },    // Firefin Snapper (was 150)
            { 6361, {400, 0} },    // Raw Rainbow Fin Albacore (was 200)
            { 6362, {500, 0} },    // Raw Rockscale Cod (was 250)
            { 6289, {250, 0} },    // Raw Longjaw Mud Snapper (was 125)
            { 21071, {600, 0} },   // Raw Sagefish (was 300)
            { 21153, {700, 0} },   // Raw Greater Sagefish (was 350)
            { 27422, {400, 0} },   // Barbed Gill Trout
            { 27425, {450, 0} },   // Spotted Feltail
            { 27429, {500, 0} },   // Zangarian Sporefish
            { 27435, {550, 0} },   // Figluster's Mudfish
            { 27437, {600, 0} },   // Icefin Bluefish
            { 27438, {650, 0} },   // Golden Darter
            { 27439, {700, 0} },   // Furious Crawdad
            { 33823, {750, 0} },   // Bloodfin Catfish
            { 33824, {800, 0} },   // Crescent-Tail Skullfish
            { 41800, {850, 0} },   // Deep Sea Monsterbelly
            { 41801, {900, 0} },   // Moonglow Cuttlefish
            { 41802, {950, 0} },   // Imperial Manta Ray
            { 41803, {1000, 0} },  // Rockfin Grouper
            { 41805, {1050, 0} },  // Borean Man O' War
            { 41806, {1100, 0} },  // Musselback Sculpin
            { 41807, {1150, 0} },  // Dragonfin Angelfish
            { 41808, {1200, 0} },  // Bonescale Snapper
            { 41809, {1250, 0} },  // Glacial Salmon
            { 41810, {1300, 0} },  // Fangtooth Herring
            { 41812, {1350, 0} },  // Giant Darkwater Clam
            { 41813, {1400, 0} },  // Nettlefish
            { 41814, {1450, 0} },  // Glassfin Minnow
        };
    }

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

        // Calculate skill multiplier
        float skillMultiplier = 1.0f;  // Remove variable scaling

        // Apply level scaling formula
        float levelMultiplier = (requiredSkill <= 150) ? 1.0f : 
                                (0.5f + (0.5f * (1.0f - static_cast<float>(playerLevel) / GATHERING_MAX_LEVEL)));

        // Get rarity multiplier
        float rarityMultiplier = GetRarityMultiplier(itemId);

        // Apply zone-based multiplier for fishing
        float zoneMultiplier = IsFishingItem(itemId) ? GetFishingZoneMultiplier(player->GetZoneId()) : 1.0f;

        // Calculate final XP
        uint32 scaledXP = static_cast<uint32>(baseXP * skillMultiplier * levelMultiplier * rarityMultiplier * zoneMultiplier);

        // Higher minimum XP for fishing (scales with skill level)
        uint32 minXP = IsFishingItem(itemId) ? 
            std::max(50u, static_cast<uint32>(50 * (1.0f + currentSkill * 0.002f))) : 
            MIN_EXPERIENCE_GAIN;
        
        // Apply minimum and maximum XP constraints
        uint32 finalXP = std::clamp(scaledXP, minXP, MAX_EXPERIENCE_GAIN);

        LOG_INFO("module", "CalculateExperience - ItemId: {}, BaseXP: {}, CurrentSkill: {}, RequiredSkill: {}, SkillMultiplier: {:.2f}, LevelMultiplier: {:.2f}, RarityMultiplier: {:.2f}, ZoneMultiplier: {:.2f}, ScaledXP: {}, FinalXP: {}", 
                itemId, baseXP, currentSkill, requiredSkill, skillMultiplier, levelMultiplier, rarityMultiplier, zoneMultiplier, scaledXP, finalXP);

        return finalXP;
    }

    // Function to get base XP and required skill for different mining, herbalism items, and skinning items
    std::pair<uint32, uint32> GetGatheringBaseXPAndRequiredSkill(uint32 itemId)
    {
        if (!enabled)
            return {0, 0};
        
        // Check each profession map silently
        auto miningXP = miningItemsXP.find(itemId);
        if (miningXP != miningItemsXP.end())
            return miningXP->second;

        auto herbXP = herbalismItemsXP.find(itemId);
        if (herbXP != herbalismItemsXP.end())
            return herbXP->second;

        auto skinningXP = skinningItemsXP.find(itemId);
        if (skinningXP != skinningItemsXP.end())
            return skinningXP->second;

        auto fishingXP = fishingItemsXP.find(itemId);
        if (fishingXP != fishingItemsXP.end())
            return fishingXP->second;

        return {0, 0};
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

    // Check if the item is related to gathering (mining, herbalism, skinning, or fishing)
    bool IsGatheringItem(uint32 itemId)
    {
        // Check if the item exists in mining, herbalism, or skinning XP maps
        auto [baseXP, requiredSkill] = GetGatheringBaseXPAndRequiredSkill(itemId);
        return baseXP > 0;
    }

    float GetFishingZoneMultiplier(uint32 zoneId)
    {
        // Define zone multipliers for fishing
        const std::map<uint32, float> fishingZoneMultipliers = {
            // Example values, adjust as needed
            { 1, 1.0f },   // Dun Morogh
            { 12, 1.2f },  // Elwynn Forest
            { 14, 1.5f },  // Durotar
            { 85, 1.3f },  // Tirisfal Glades
            { 141, 1.4f }, // Teldrassil
            { 215, 1.6f }, // Mulgore
            { 3, 1.1f },   // Badlands
            { 10, 1.7f },  // Duskwood
            { 11, 1.8f },  // Wetlands
            { 33, 1.9f },  // Stranglethorn Vale
            { 38, 2.0f },  // Loch Modan
            { 40, 1.6f },  // Westfall
            { 44, 1.7f },  // Redridge Mountains
            { 47, 1.8f },  // The Hinterlands
            { 51, 1.9f },  // Searing Gorge
            { 139, 2.0f }, // Eastern Plaguelands
            { 1377, 2.1f },// Silithus
            { 1519, 2.2f },// Stormwind City
            { 1637, 2.3f },// Orgrimmar
            { 3430, 2.4f },// Eversong Woods
            // Add more zones as needed
        };

        auto it = fishingZoneMultipliers.find(zoneId);
        float multiplier = (it != fishingZoneMultipliers.end()) ? it->second : 1.0f;
        LOG_INFO("module", "GetFishingZoneMultiplier for zoneId: {}, multiplier: {}", zoneId, multiplier);
        return multiplier;
    }

    void OnLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid /*lootguid*/) override
    {
        if (!enabled)
            return;

        uint32 itemId = item->GetEntry();
        auto [baseXP, requiredSkill] = GetGatheringBaseXPAndRequiredSkill(itemId);
        
        if (baseXP == 0)
        {
            LOG_DEBUG("module", "Item {} not recognized as a gathering item", itemId);
            return;
        }

        uint32 currentSkill = 0;
        SkillType skillType = SKILL_NONE;

        // Check for each profession
        if (IsFishingItem(itemId) && player->HasSkill(SKILL_FISHING))
        {
            currentSkill = player->GetSkillValue(SKILL_FISHING);
            skillType = SKILL_FISHING;
        }
        else if (IsMiningItem(itemId) && player->HasSkill(SKILL_MINING))
        {
            currentSkill = player->GetSkillValue(SKILL_MINING);
            skillType = SKILL_MINING;
        }
        else if (IsHerbalismItem(itemId) && player->HasSkill(SKILL_HERBALISM))
        {
            currentSkill = player->GetSkillValue(SKILL_HERBALISM);
            skillType = SKILL_HERBALISM;
        }
        else if (IsSkinningItem(itemId) && player->HasSkill(SKILL_SKINNING))
        {
            currentSkill = player->GetSkillValue(SKILL_SKINNING);
            skillType = SKILL_SKINNING;
        }

        if (skillType == SKILL_NONE)
        {
            LOG_DEBUG("module", "Player lacks required skill for item {}", itemId);
            return;
        }

        uint32 xp = CalculateExperience(player, baseXP, requiredSkill, currentSkill, itemId);
        
        LOG_INFO("module", "{} XP gained - {} (skill {}): {} XP from ItemId: {}", 
                 xp, GetSkillName(skillType), currentSkill, baseXP, itemId);

        player->GiveXP(xp, nullptr);
    }

    // Helper function to get skill name
    const char* GetSkillName(SkillType skillType)
    {
        switch (skillType)
        {
            case SKILL_MINING:
                return "Mining";
            case SKILL_HERBALISM:
                return "Herbalism";
            case SKILL_SKINNING:
                return "Skinning";
            case SKILL_FISHING:
                return "Fishing";
            default:
                return "Unknown";
        }
    }

    // Modify the profession check functions to not log unless in debug mode
    bool IsFishingItem(uint32 itemId)
    {
        auto it = fishingItemsXP.find(itemId);
        return it != fishingItemsXP.end();
    }

    bool IsMiningItem(uint32 itemId)
    {
        auto it = miningItemsXP.find(itemId);
        return it != miningItemsXP.end();
    }

    bool IsHerbalismItem(uint32 itemId)
    {
        auto it = herbalismItemsXP.find(itemId);
        return it != herbalismItemsXP.end();
    }

    bool IsSkinningItem(uint32 itemId)
    {
        auto it = skinningItemsXP.find(itemId);
        return it != skinningItemsXP.end();
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
