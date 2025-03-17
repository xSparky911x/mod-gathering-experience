/*
*Copyright (C) 2024+ xSparky911x, Thaxtin, released under GNU AGPL v3 license: https://github.com/xSparky911x/mod-gathering-experience/blob/master/LICENSE
*/

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "StringFormat.h"
#include "GatheringExperience.h"
#include "professions/Fishing.h"
#include "professions/Skinning.h"
#include "professions/Herbalism.h"
#include "professions/Mining.h"

GatheringExperienceModule* GatheringExperienceModule::instance = nullptr;

// Define the version string here
const char* GATHERING_EXPERIENCE_VERSION = "0.6.1";

void GatheringExperienceModule::LoadDataFromDB()
{
    LOG_INFO("module", "Loading Gathering Experience data...");
    
    LoadSettingsFromDB();
    LoadGatheringData();

    // Load zone multipliers
    QueryResult result = WorldDatabase.Query("SELECT * FROM gathering_experience_zones");
    if (result)
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            zoneMultipliers[fields[0].Get<uint32>()] = fields[1].Get<float>();
            count++;
        } while (result->NextRow());
        LOG_INFO("module", "Loaded {} zone multipliers", count);
    }

    // Load rarity multipliers
    result = WorldDatabase.Query("SELECT * FROM gathering_experience_rarity");
    if (result)
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            rarityMultipliers[fields[0].Get<uint32>()] = fields[1].Get<float>();
            count++;
        } while (result->NextRow());
        LOG_INFO("module", "Loaded {} rarity multipliers", count);
    }
}

void GatheringExperienceModule::LoadSettingsFromDB()
{
    QueryResult result = WorldDatabase.Query("SELECT profession, enabled FROM gathering_experience_settings");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        std::string profession = fields[0].Get<std::string>();
        bool enabled = fields[1].Get<bool>();

        if (profession == "Mining")
            miningEnabled = enabled;
        else if (profession == "Herbalism")
            herbalismEnabled = enabled;
        else if (profession == "Skinning")
            skinningEnabled = enabled;
        else if (profession == "Fishing")
            fishingEnabled = enabled;
    } while (result->NextRow());
}

void GatheringExperienceModule::LoadGatheringData()
{
    // Clear existing data
    gatheringItems.clear();

    // Load gathering items
    if (QueryResult result = WorldDatabase.Query(
        "SELECT item_id, base_xp, required_skill, profession, name FROM gathering_experience"))
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 itemId = fields[0].Get<uint32>();
            GatheringItem item;
            item.baseXP = fields[1].Get<uint32>();
            item.requiredSkill = fields[2].Get<uint32>();
            item.profession = fields[3].Get<uint8>();
            item.name = fields[4].Get<std::string>();
            item.rarity = 0; // Default to common if not specified
            gatheringItems[itemId] = item;
            count++;
        } while (result->NextRow());
        LOG_INFO("module", "Loaded {} gathering items", count);
    }
    else
    {
        LOG_INFO("module", "No gathering items found in database");
    }
}

bool GatheringExperienceModule::ToggleMining()
{
    miningEnabled = !miningEnabled;
    SaveSettingToDB("Mining", miningEnabled);
    return miningEnabled;
}

bool GatheringExperienceModule::ToggleHerbalism()
{
    herbalismEnabled = !herbalismEnabled;
    SaveSettingToDB("Herbalism", herbalismEnabled);
    return herbalismEnabled;
}

bool GatheringExperienceModule::ToggleSkinning()
{
    skinningEnabled = !skinningEnabled;
    SaveSettingToDB("Skinning", skinningEnabled);
    return skinningEnabled;
}

bool GatheringExperienceModule::ToggleFishing()
{
    fishingEnabled = !fishingEnabled;
    SaveSettingToDB("Fishing", fishingEnabled);
    return fishingEnabled;
}

float GatheringExperienceModule::GetZoneMultiplier(uint32 zoneId) const
{
    auto it = zoneMultipliers.find(zoneId);
    if (it != zoneMultipliers.end())
    {
        return it->second;
    }
    return 1.0f; // Default multiplier if zone not found
}

uint32 GatheringExperienceModule::CalculateExperience(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill, uint32 /*itemId*/)
{
    if (!player || !enabled)
        return 0;

    uint32 playerLevel = player->GetLevel();
    if (playerLevel >= GATHERING_MAX_LEVEL)
        return 0;

    // Check skill requirement
    if (currentSkill < requiredSkill)
        return 0;

    // Get zone multiplier
    float zoneMultiplier = 1.0f;
    auto zoneIt = zoneMultipliers.find(player->GetZoneId());
    if (zoneIt != zoneMultipliers.end())
    {
        zoneMultiplier = zoneIt->second;
    }

    // Calculate progress bonus
    float progressBonus = CalculateProgressBonus(currentSkill);

    // Calculate final experience
    uint32 experience = static_cast<uint32>(baseXP * (1.0f + progressBonus) * zoneMultiplier);

    // Apply min/max bounds
    experience = std::max(MIN_EXPERIENCE_GAIN, std::min(experience, MAX_EXPERIENCE_GAIN));

    return experience;
}

float GatheringExperienceModule::CalculateProgressBonus(uint32 currentSkill)
{
    uint32 currentTier = (currentSkill / TIER_SIZE) * TIER_SIZE;
    uint32 progressInTier = currentSkill - currentTier;
    return progressInTier * PROGRESS_BONUS_RATE;
}

void GatheringExperienceModule::OnLootItem(Player* player, Item* item, [[maybe_unused]] uint32 count, [[maybe_unused]] ObjectGuid lootguid)
{
    if (!enabled || !player || !item)
        return;

    uint32 itemId = item->GetEntry();
    uint32 xpGained = 0;

    // Check each profession
    if (sFishingExperience->IsFishingItem(itemId))
    {
        xpGained = sFishingExperience->CalculateFishingExperience(player, itemId);
    }
    else if (sSkinningExperience->IsSkinningItem(itemId))
    {
        xpGained = sSkinningExperience->CalculateSkinningExperience(player, itemId);
    }
    else if (sHerbalismExperience->IsHerbalismItem(itemId))
    {
        xpGained = sHerbalismExperience->CalculateHerbalismExperience(player, itemId);
    }
    else if (sMiningExperience->IsMiningItem(itemId))
    {
        xpGained = sMiningExperience->CalculateMiningExperience(player, itemId);
    }

    if (xpGained > 0)
    {
        player->GiveXP(xpGained, nullptr);
    }
}

void GatheringExperienceModule::SaveSettingToDB(std::string const& profession, bool enabled)
{
    WorldDatabase.DirectExecute(
        "REPLACE INTO gathering_experience_settings (profession, enabled) VALUES ('{}', {})",
        profession, enabled ? 1 : 0);
}

void GatheringExperienceModule::OnStartup()
{
    LOG_INFO("server.loading", "GatheringExperienceModule - Loading data from database...");

    // Apply database updates
    if (!sWorld->getBoolConfig(CONFIG_DISABLE_DATABASE_UPDATES))
    {
        LOG_INFO("module", "Applying database updates for Gathering Experience module...");
        DatabaseLoader loader("mod-gathering-experience");
        
        // Use absolute paths based on module directory
        std::string moduleDir = "/modules/mod-gathering-experience/";
        loader.AddUpdatePath(moduleDir + "data/sql/db-world/updates/");
        loader.AddUpdatePath(moduleDir + "data/sql/db-auth/updates/");
        loader.AddUpdatePath(moduleDir + "data/sql/db-characters/updates/");
        loader.Update();
    }

    LoadDataFromDB();
}

void GatheringExperienceModule::OnBeforeConfigLoad(bool /*reload*/)
{
    enabled = sConfigMgr->GetOption<bool>("GatheringExperience.Enable", true);
    if (!enabled)
    {
        LOG_INFO("server.loading", "Gathering Experience Module is disabled by config.");
        return;
    }

    // Load initial values from config
    miningEnabled = sConfigMgr->GetOption<bool>("GatheringExperience.Mining.Enable", true);
    herbalismEnabled = sConfigMgr->GetOption<bool>("GatheringExperience.Herbalism.Enable", true);
    skinningEnabled = sConfigMgr->GetOption<bool>("GatheringExperience.Skinning.Enable", true);
    fishingEnabled = sConfigMgr->GetOption<bool>("GatheringExperience.Fishing.Enable", true);

    // Override with DB values if they exist
    LoadSettingsFromDB();

    LOG_INFO("server.loading", "Gathering Experience Module configuration loaded.");
}

void GatheringExperienceModule::OnAfterConfigLoad(bool /*reload*/)
{
    if (enabled)
    {
        LOG_INFO("module", "Gathering Experience Module {} Loaded", GATHERING_EXPERIENCE_VERSION);
        LOG_INFO("module", "Mining: {}, Herbalism: {}, Skinning: {}, Fishing: {}",
            miningEnabled ? "Enabled" : "Disabled",
            herbalismEnabled ? "Enabled" : "Disabled",
            skinningEnabled ? "Enabled" : "Disabled",
            fishingEnabled ? "Enabled" : "Disabled"
        );
    }
}

void GatheringExperienceModule::OnLogin(Player* player)
{
    if (!enabled)
        return;

    if (sConfigMgr->GetOption<bool>("GatheringExperience.Announce", true))
    {
        std::string message = "This server is running the |cff4CFF00Gathering Experience|r module v" + 
            std::string(GATHERING_EXPERIENCE_VERSION) + " by xSparky911x and Thaxtin.";
        ChatHandler(player->GetSession()).SendSysMessage(message.c_str());
    }
}

