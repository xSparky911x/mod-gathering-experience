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

GatheringExperienceModule* GatheringExperienceModule::instance = nullptr;

// Define the version string here
const char* GATHERING_EXPERIENCE_VERSION = "0.4.2";

void GatheringExperienceModule::LoadDataFromDB()
{
    LOG_INFO("module", "Loading Gathering Experience data...");
    
    LoadSettingsFromDB();
    LoadGatheringData();
    LoadZoneData();
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
    if (QueryResult result = WorldDatabase.Query("SELECT item_id, base_xp, required_skill, profession, name FROM gathering_experience"))
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
            gatheringItems[itemId] = item;
            count++;
        } while (result->NextRow());
        LOG_INFO("server.loading", "Loaded {} gathering items", count);
    }
    else
    {
        LOG_INFO("server.loading", "No gathering items found in database");
    }

    // Load zone multipliers
    if (QueryResult result = WorldDatabase.Query("SELECT zone_id, multiplier FROM gathering_experience_zones"))
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            zoneMultipliers[fields[0].Get<uint32>()] = fields[1].Get<float>();
            count++;
        } while (result->NextRow());
        LOG_INFO("server.loading", "Loaded {} zone multipliers", count);
    }
}

void GatheringExperienceModule::LoadZoneData()
{
    // Clear existing zone data
    zoneMultipliers.clear();

    // Load zone multipliers
    if (QueryResult result = WorldDatabase.Query("SELECT zone_id, multiplier FROM gathering_experience_zones"))
    {
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 zoneId = fields[0].Get<uint32>();
            float multiplier = fields[1].Get<float>();
            zoneMultipliers[zoneId] = multiplier;
            count++;
        } while (result->NextRow());
        LOG_INFO("server.loading", "Loaded {} zone multipliers", count);
    }
    else
    {
        LOG_INFO("server.loading", "No zone multipliers found in database");
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
    if (playerLevel > GATHERING_MAX_LEVEL)
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

    // Check if in a city
    if (IsCityZone(player->GetZoneId()))
    {
        zoneMultiplier *= 0.5f; // 50% reduction in cities
    }

    // Calculate skill tier multiplier and progress bonus
    float skillTierMult = GetFishingTierMultiplier(currentSkill);
    float progressBonus = CalculateProgressBonus(currentSkill);

    // Calculate final experience
    uint32 experience = static_cast<uint32>(baseXP * skillTierMult * (1.0f + progressBonus) * zoneMultiplier);

    // Apply min/max bounds
    experience = std::max(MIN_EXPERIENCE_GAIN, std::min(experience, MAX_EXPERIENCE_GAIN));

    return experience;
}

float GatheringExperienceModule::GetFishingTierMultiplier(uint32 currentSkill) const
{
    if (currentSkill <= TIER_1_MAX)
        return 1.0f;
    else if (currentSkill <= TIER_2_MAX)
        return 0.8f;
    else if (currentSkill <= TIER_3_MAX)
        return 0.6f;
    else if (currentSkill <= TIER_4_MAX)
        return 0.4f;
    else
        return 0.2f;
}

float GatheringExperienceModule::CalculateProgressBonus(uint32 currentSkill)
{
    uint32 currentTier = (currentSkill / TIER_SIZE) * TIER_SIZE;
    uint32 progressInTier = currentSkill - currentTier;
    return progressInTier * PROGRESS_BONUS_RATE;
}

void GatheringExperienceModule::OnLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid /*lootguid*/)
{
    if (!enabled || !player || !item)
        return;

    uint32 itemId = item->GetEntry();
    if (!IsGatheringItem(itemId))
        return;

    auto it = gatheringItems.find(itemId);
    if (it == gatheringItems.end())
        return;

    const GatheringItem& gatherItem = it->second;

    // Get current skill level based on profession
    uint32 currentSkill = 0;
    switch (gatherItem.profession)
    {
        case PROF_MINING:
            if (!miningEnabled) return;
            currentSkill = player->GetSkillValue(SKILL_MINING);
            break;
        case PROF_HERBALISM:
            if (!herbalismEnabled) return;
            currentSkill = player->GetSkillValue(SKILL_HERBALISM);
            break;
        case PROF_SKINNING:
            if (!skinningEnabled) return;
            currentSkill = player->GetSkillValue(SKILL_SKINNING);
            break;
        case PROF_FISHING:
            if (!fishingEnabled) return;
            currentSkill = player->GetSkillValue(SKILL_FISHING);
            break;
        default:
            return;
    }

    uint32 experience = CalculateExperience(player, gatherItem.baseXP, gatherItem.requiredSkill, currentSkill, itemId);
    if (experience > 0)
    {
        player->GiveXP(experience, nullptr);
        ChatHandler(player->GetSession()).PSendSysMessage("You gain %u experience from gathering.", experience);
    }
}

bool GatheringExperienceModule::IsCityZone(uint32 zoneId) const
{
    switch (zoneId)
    {
        case ZONE_STORMWIND:
        case ZONE_IRONFORGE:
        case ZONE_DARNASSUS:
        case ZONE_ORGRIMMAR:
        case ZONE_THUNDERBLUFF:
        case ZONE_UNDERCITY:
        case ZONE_EXODAR:
        case ZONE_SILVERMOON:
        case ZONE_SHATTRATH:
        case ZONE_DALARAN:
            return true;
        default:
            return false;
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

