#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "StringFormat.h"

// Define the maximum level for gathering scaling
const uint32 GATHERING_MAX_LEVEL = 80;
const uint32 MAX_EXPERIENCE_GAIN = 25000;
const uint32 MIN_EXPERIENCE_GAIN = 10;
const char* GATHERING_EXPERIENCE_VERSION = "0.4.2";

enum GatheringProfessions
{
    PROF_MINING     = 1,
    PROF_HERBALISM  = 2,
    PROF_SKINNING   = 3,
    PROF_FISHING    = 4
};

enum CitiesZoneIds
{
    ZONE_STORMWIND = 1519,
    ZONE_IRONFORGE = 1537,
    ZONE_DARNASSUS = 1657,
    ZONE_ORGRIMMAR = 1637,
    ZONE_THUNDERBLUFF = 1638,
    ZONE_UNDERCITY = 1497,
    ZONE_EXODAR = 3557,
    ZONE_SILVERMOON = 3487,
    ZONE_SHATTRATH = 3703,
    ZONE_DALARAN = 4395,
};

bool IsCityZone(uint32 zoneId)
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

class GatheringExperienceModule : public PlayerScript, public WorldScript
{
private:
    struct GatheringItem {
        uint32 baseXP;
        uint32 requiredSkill;
        uint8 profession;
        std::string name;
    };

    std::map<uint32, GatheringItem> gatheringItems;
    std::map<uint32, float> zoneMultipliers;
    bool enabled;
    bool dataLoaded;

    bool miningEnabled;
    bool herbalismEnabled;
    bool skinningEnabled;
    bool fishingEnabled;

    // Constants for better readability and maintenance
    static constexpr uint32 TIER_SIZE = 75;
    static constexpr float PROGRESS_BONUS_RATE = 0.02f;
    static constexpr float BASE_ZONE_MULTIPLIER = 1.0f;

    // Skill tier thresholds
    static constexpr uint32 TIER_1_MAX = 75;
    static constexpr uint32 TIER_2_MAX = 150;
    static constexpr uint32 TIER_3_MAX = 225;
    static constexpr uint32 TIER_4_MAX = 300;

public:
    static GatheringExperienceModule* instance;

    GatheringExperienceModule() : 
        PlayerScript("GatheringExperienceModule"), 
        WorldScript("GatheringExperienceModule"),
        enabled(false),
        dataLoaded(false)
    {
        instance = this;
    }

    void OnStartup() override
    {
        LOG_INFO("server.loading", "GatheringExperienceModule - Loading data from database...");
        LoadDataFromDB();
    }

    void OnBeforeConfigLoad([[maybe_unused]] bool reload) override
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

    void LoadDataFromDB()
    {
        try 
        {
            LOG_INFO("server.loading", "LoadDataFromDB called - Starting reload...");
            
            // Clear existing data
            LOG_INFO("server.loading", "Clearing existing gathering data from memory...");
            gatheringItems.clear();
            zoneMultipliers.clear();
            dataLoaded = false;

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

            dataLoaded = true;
            LOG_INFO("server.loading", "LoadDataFromDB - Reload complete");
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("server.loading", "Error in LoadDataFromDB: {}", e.what());
            gatheringItems.clear();
            zoneMultipliers.clear();
            dataLoaded = false;
        }
    }

    void OnLogin(Player* player) override
    {
        if (!enabled)
            return;

        if (sConfigMgr->GetOption<bool>("GatheringExperience.Announce", true))
        {
            std::string message = "This server is running the |cff4CFF00Gathering Experience|r module v" + std::string(GATHERING_EXPERIENCE_VERSION) + " by xSparky911x and Thaxtin.";
            ChatHandler(player->GetSession()).SendSysMessage(message.c_str());
        }
    }

    // Function to calculate scaled experience based on player level and item base XP
    uint32 CalculateExperience(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill, uint32 itemId)
    {
        if (player->GetLevel() >= GATHERING_MAX_LEVEL)
            return 0;

        float skillMultiplier = 1.0f;
        float levelMultiplier = std::min(8.0f, std::max(0.8f, player->GetLevel() / 10.0f));

        if (IsFishingItem(itemId))
        {
            // Adjust base XP based on both level and skill
            uint32 adjustedBaseXP = baseXP;
            std::string adjustReason;

            // Standard minimum XP based on skill tiers
            if (currentSkill > 300)
            {
                uint32 skillBasedMin = 200;
                if (adjustedBaseXP < skillBasedMin)
                {
                    adjustedBaseXP = skillBasedMin;
                    adjustReason = "minimum for skill > 300";
                }
            }
            else if (currentSkill > 150)
            {
                uint32 skillBasedMin = 125;
                if (adjustedBaseXP < skillBasedMin)
                {
                    adjustedBaseXP = skillBasedMin;
                    adjustReason = "minimum for skill > 150";
                }
            }
            else if (currentSkill > 75)
            {
                uint32 skillBasedMin = 100;
                if (adjustedBaseXP < skillBasedMin)
                {
                    adjustedBaseXP = skillBasedMin;
                    adjustReason = "minimum for skill > 75";
                }
            }

            // Add level requirement adjustments
            float levelPenalty = 1.0f;
            std::string penaltyReason;
            
            // Get recommended level for this fish/zone
            uint32 recommendedLevel = 1;  // Default for starter areas
            if (baseXP >= 216)       // Northrend fish (70+)
                recommendedLevel = 70;
            else if (baseXP >= 180)  // Outland fish (60-70)
                recommendedLevel = 60;
            else if (baseXP >= 168)  // High-level vanilla (50-60)
                recommendedLevel = 50;
            else if (baseXP >= 144)  // Mid-high vanilla (40-50)
                recommendedLevel = 40;
            else if (baseXP >= 108)  // Mid-level vanilla (30-40)
                recommendedLevel = 30;
            else if (baseXP >= 72)   // Lower-level vanilla (20-30)
                recommendedLevel = 20;
            else if (baseXP >= 36)   // Beginner areas (10-20)
                recommendedLevel = 10;

            // Calculate level difference
            int32 levelDiff = player->GetLevel() - recommendedLevel;
            
            if (levelDiff < -20)  // Way too low level for area
            {
                levelPenalty = 0.1f;  // 90% reduction
                penaltyReason = fmt::format("extremely reduced (level {} << {})", 
                    player->GetLevel(), recommendedLevel);
            }
            else if (levelDiff < -10)  // Moderately too low
            {
                levelPenalty = 0.25f;  // 75% reduction
                penaltyReason = fmt::format("severely reduced (level {} < {})", 
                    player->GetLevel(), recommendedLevel);
            }
            else if (levelDiff > 20)  // Way too high level for area
            {
                levelPenalty = 0.05f;  // 95% reduction
                penaltyReason = fmt::format("heavily reduced (level {} >> {})", 
                    player->GetLevel(), recommendedLevel);
            }
            else if (levelDiff > 10)  // Moderately too high
            {
                levelPenalty = 0.5f;  // 50% reduction
                penaltyReason = fmt::format("slightly reduced (level {} > {})", 
                    player->GetLevel(), recommendedLevel);
            }

            float rawSkillTierMult = GetFishingTierMultiplier(currentSkill);
            float skillTierMult = std::min(1.2f, rawSkillTierMult);
            float progressBonus = std::min(0.3f, currentSkill / 450.0f);
            float zoneMult = GetFishingZoneMultiplier(player->GetZoneId());

            // Get zone name for logging
            std::string zoneName = "Unknown";
            if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(player->GetZoneId()))
            {
                zoneName = area->area_name[0];
            }

            bool isCity = IsCityZone(player->GetZoneId());
            if (isCity)
            {
                zoneMult = std::min(1.5f, zoneMult);
            }

            uint32 normalXP = static_cast<uint32>(adjustedBaseXP * (skillTierMult + progressBonus) * zoneMult * levelPenalty);
            uint32 finalXP = normalXP;

            // Check if player has any rested XP available
            if (player->GetRestBonus() > 0)
            {
                uint32 restedXP = player->GetXPRestBonus(normalXP);
                finalXP += restedXP;
                
                float currentRestBonus = player->GetRestBonus();
                player->SetRestBonus(currentRestBonus - (float(restedXP) / 2.0f));
            }

            LOG_INFO("module.gathering", "Fishing XP Calculation:");
            LOG_INFO("module.gathering", "- Zone: {} (ID: {}) {}", zoneName, player->GetZoneId(), isCity ? "[City]" : "");
            LOG_INFO("module.gathering", "- Base XP: {}", baseXP);
            if (adjustedBaseXP != baseXP)
            {
                LOG_INFO("module.gathering", "- Adjusted Base XP: {} ({})", adjustedBaseXP, adjustReason);
            }
            LOG_INFO("module.gathering", "- Level ({}) Multiplier: {}", player->GetLevel(), levelMultiplier);
            if (levelPenalty < 1.0f)
            {
                LOG_INFO("module.gathering", "- Level Penalty: {} ({})", levelPenalty, penaltyReason);
            }
            LOG_INFO("module.gathering", "- Skill ({}) Tier Multiplier: {} (capped from {})", currentSkill, skillTierMult, rawSkillTierMult);
            LOG_INFO("module.gathering", "- Progress Bonus: {}", progressBonus);
            LOG_INFO("module.gathering", "- Zone Multiplier: {}", zoneMult);
            LOG_INFO("module.gathering", "- Normal XP (before rested): {}", normalXP);
            LOG_INFO("module.gathering", "- Rested Bonus Applied: {}", finalXP - normalXP);
            LOG_INFO("module.gathering", "- Final XP: {}", finalXP);

            return finalXP;
        }
        else
        {
            // Get zone name for logging
            std::string zoneName = "Unknown";
            if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(player->GetZoneId()))
            {
                zoneName = area->area_name[0];
            }

            // Skill level multiplier for non-fishing gathering
            std::string skillColor;
            if (currentSkill < requiredSkill)
            {
                skillMultiplier = 0.1f;  // Gray skill - minimal XP (too low skill)
                skillColor = "Gray (Too Low)";
            }
            else if (currentSkill < requiredSkill + 25)
            {
                skillMultiplier = 1.2f;  // Orange skill - highest XP (challenging)
                skillColor = "Orange";
            }
            else if (currentSkill < requiredSkill + 50)
            {
                skillMultiplier = 1.0f;  // Yellow skill - normal XP (moderate)
                skillColor = "Yellow";
            }
            else if (currentSkill < requiredSkill + 75)
            {
                skillMultiplier = 0.8f;  // Green skill - reduced XP (easy)
                skillColor = "Green";
            }
            else
            {
                skillMultiplier = 0.5f;  // Gray skill - minimal XP (trivial)
                skillColor = "Gray (Trivial)";
            }

            uint32 finalXP = static_cast<uint32>(baseXP * skillMultiplier * levelMultiplier);

            LOG_INFO("module.gathering", "Gathering XP Calculation:");
            LOG_INFO("module.gathering", "- Zone: {} (ID: {})", zoneName, player->GetZoneId());
            LOG_INFO("module.gathering", "- Base XP: {}", baseXP);
            LOG_INFO("module.gathering", "- Level ({}) Multiplier: {}", player->GetLevel(), levelMultiplier);
            LOG_INFO("module.gathering", "- Skill ({}/{}) Color: {} Multiplier: {}", currentSkill, requiredSkill, skillColor, skillMultiplier);
            LOG_INFO("module.gathering", "- Final XP: {}", finalXP);

            return finalXP;
        }
    }

    // Function to get base XP and required skill for different mining, herbalism items, and skinning items
    std::pair<uint32, uint32> GetGatheringBaseXPAndRequiredSkill(uint32 itemId)
    {
        if (gatheringItems.empty())
        {
            LOG_INFO("server.loading", "GetGatheringBaseXPAndRequiredSkill - gatheringItems map is empty!");
            return {0, 0};
        }

        LOG_INFO("server.loading", "GetGatheringBaseXPAndRequiredSkill - Checking item {}", itemId);
        LOG_INFO("server.loading", "GetGatheringBaseXPAndRequiredSkill - Items in memory: {}", gatheringItems.size());
        
        auto it = gatheringItems.find(itemId);
        if (it != gatheringItems.end())
        {
            LOG_INFO("server.loading", "GetGatheringBaseXPAndRequiredSkill - Found item {} with baseXP {}", 
                itemId, it->second.baseXP);
            return {it->second.baseXP, it->second.requiredSkill};
        }
        
        LOG_INFO("server.loading", "GetGatheringBaseXPAndRequiredSkill - Item {} not found in memory", itemId);
        return {0, 0};
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
        auto it = zoneMultipliers.find(zoneId);
        return it != zoneMultipliers.end() ? it->second : 1.0f;
    }

    void OnLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid /*lootguid*/) override
    {
        if (!enabled)
            return;

        uint32 itemId = item->GetEntry();
        
        auto [baseXP, requiredSkill] = GetGatheringBaseXPAndRequiredSkill(itemId);
        
        if (baseXP == 0)
            return;

        uint32 currentSkill = 0;
        SkillType skillType = SKILL_NONE;

        // Check each profession with its enabled state
        if (fishingEnabled && IsFishingItem(itemId) && player->HasSkill(SKILL_FISHING))
        {
            currentSkill = player->GetSkillValue(SKILL_FISHING);
            skillType = SKILL_FISHING;
        }
        else if (miningEnabled && IsMiningItem(itemId) && player->HasSkill(SKILL_MINING))
        {
            currentSkill = player->GetSkillValue(SKILL_MINING);
            skillType = SKILL_MINING;
        }
        else if (herbalismEnabled && IsHerbalismItem(itemId) && player->HasSkill(SKILL_HERBALISM))
        {
            currentSkill = player->GetSkillValue(SKILL_HERBALISM);
            skillType = SKILL_HERBALISM;
        }
        else if (skinningEnabled && IsSkinningItem(itemId) && player->HasSkill(SKILL_SKINNING))
        {
            currentSkill = player->GetSkillValue(SKILL_SKINNING);
            skillType = SKILL_SKINNING;
        }

        if (skillType == SKILL_NONE)
        {
            LOG_DEBUG("server.loading", "Player {} lacks required skill for item {}", player->GetName(), itemId);
            return;
        }

        uint32 xp = CalculateExperience(player, baseXP, requiredSkill, currentSkill, itemId);
        
        LOG_INFO("server.loading", "Player {} gained {} XP from {} (skill {}) {} ({})",
                 player->GetName(),         // Player name
                 xp,                        // XP gained
                 GetSkillName(skillType),   // Skill name
                 currentSkill,              // Current skill level
                 item->GetTemplate()->Name1, // Item name
                 itemId);                   // Item ID

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
        return HasGatheringItem(itemId, PROF_FISHING);
    }

    bool IsMiningItem(uint32 itemId)
    {
        auto it = gatheringItems.find(itemId);
        return it != gatheringItems.end() && it->second.profession == 1;
    }

    bool IsHerbalismItem(uint32 itemId)
    {
        auto it = gatheringItems.find(itemId);
        return it != gatheringItems.end() && it->second.profession == 2;
    }

    bool IsSkinningItem(uint32 itemId)
    {
        auto it = gatheringItems.find(itemId);
        return it != gatheringItems.end() && it->second.profession == 3;
    }

    bool IsEnabled() const { return enabled; }
    
    void SetEnabled(bool state) 
    { 
        enabled = state;
        LOG_INFO("server.loading", "Gathering Experience Module {} by toggle command", 
            enabled ? "enabled" : "disabled");
    }

    // Add this new public method
    bool HasGatheringItem(uint32 itemId, uint8 profession) const
    {
        auto it = gatheringItems.find(itemId);
        return it != gatheringItems.end() && it->second.profession == profession;
    }

    bool ToggleMining()
    {
        miningEnabled = !miningEnabled;
        SaveSettingToDB("mining", miningEnabled);
        return miningEnabled;
    }

    bool ToggleHerbalism()
    {
        herbalismEnabled = !herbalismEnabled;
        SaveSettingToDB("herbalism", herbalismEnabled);
        return herbalismEnabled;
    }

    bool ToggleSkinning()
    {
        skinningEnabled = !skinningEnabled;
        SaveSettingToDB("skinning", skinningEnabled);
        return skinningEnabled;
    }

    bool ToggleFishing()
    {
        fishingEnabled = !fishingEnabled;
        SaveSettingToDB("fishing", fishingEnabled);
        return fishingEnabled;
    }

    bool IsMiningEnabled() const { return miningEnabled; }
    bool IsHerbalismEnabled() const { return herbalismEnabled; }
    bool IsSkinningEnabled() const { return skinningEnabled; }
    bool IsFishingEnabled() const { return fishingEnabled; }

private:
    // Helper functions for cleaner code
    float GetFishingTierMultiplier(uint32 currentSkill) const
    {
        float tierMultiplier;
        // Adjusted multipliers to give ~400 XP with zone multiplier of 1.0
        if (currentSkill <= TIER_1_MAX)
            tierMultiplier = 1.0f;        // Beginner tier (starter areas) - 400 * 1.0 * 1.0 = 400 XP
        else if (currentSkill <= TIER_2_MAX)
            tierMultiplier = 1.1f;        // Apprentice tier
        else if (currentSkill <= TIER_3_MAX)
            tierMultiplier = 1.2f;        // Journeyman tier
        else if (currentSkill <= TIER_4_MAX)
            tierMultiplier = 1.3f;        // Expert tier
        else
            tierMultiplier = 1.4f;        // Artisan tier and beyond

        LOG_INFO("module.gathering", "Fishing Tier Multiplier for skill {}: {:.2f}", currentSkill, tierMultiplier);
        return tierMultiplier;
    }

    float CalculateProgressBonus(uint32 currentSkill)
    {
        // Provide a more meaningful bonus at low levels
        return std::max(0.1f, currentSkill / 300.0f);  // Minimum 0.1 bonus
    }

    void LogXPCalculation(uint32 baseXP, float skillTierMult, float progressBonus, 
                         float zoneMult, uint32 playerLevel) const
    {
        uint32 afterSkillMultiplier = static_cast<uint32>(baseXP * skillTierMult);
        uint32 afterProgressBonus = static_cast<uint32>(afterSkillMultiplier * (1.0f + progressBonus));
        uint32 afterZoneMultiplier = static_cast<uint32>(afterProgressBonus * zoneMult);
        float levelMult = playerLevel / 10.0f;

        LOG_DEBUG("module.gathering", "XP Calculation Breakdown:");
        LOG_DEBUG("module.gathering", "- Initial XP: {}", baseXP);
        LOG_DEBUG("module.gathering", "- After Skill Multiplier ({}x): {}", skillTierMult, afterSkillMultiplier);
        LOG_DEBUG("module.gathering", "- After Progress Bonus ({}): {}", progressBonus, afterProgressBonus);
        LOG_DEBUG("module.gathering", "- After Zone Multiplier ({}x): {}", zoneMult, afterZoneMultiplier);
        LOG_DEBUG("module.gathering", "- After Level Multiplier ({}x): {}", levelMult, 
                 static_cast<uint32>(afterZoneMultiplier * levelMult));
    }

    void LoadProfessionStates()
    {
        miningEnabled = sConfigMgr->GetOption<bool>("GatheringExperience.Mining.Enable", true);
        herbalismEnabled = sConfigMgr->GetOption<bool>("GatheringExperience.Herbalism.Enable", true);
        skinningEnabled = sConfigMgr->GetOption<bool>("GatheringExperience.Skinning.Enable", true);
        fishingEnabled = sConfigMgr->GetOption<bool>("GatheringExperience.Fishing.Enable", true);
    }

    void LoadSettingsFromDB()
    {
        QueryResult result = WorldDatabase.Query("SELECT profession, enabled FROM gathering_experience_settings");
        if (!result)
        {
            LOG_INFO("module", "Gathering Experience: No settings found in DB, using defaults");
            return;
        }

        do
        {
            Field* fields = result->Fetch();
            std::string profession = fields[0].Get<std::string>();
            bool isEnabled = fields[1].Get<bool>();

            if (profession == "mining")
                miningEnabled = isEnabled;
            else if (profession == "herbalism")
                herbalismEnabled = isEnabled;
            else if (profession == "skinning")
                skinningEnabled = isEnabled;
            else if (profession == "fishing")
                fishingEnabled = isEnabled;

        } while (result->NextRow());

        LOG_INFO("module", "Gathering Experience: Settings loaded from database");
    }

    void SaveSettingToDB(std::string const& profession, bool enabled)
    {
        WorldDatabase.Execute("UPDATE gathering_experience_settings SET enabled = {} WHERE profession = '{}'",
            enabled ? 1 : 0, profession);
    }
};

// Initialize the static member
GatheringExperienceModule* GatheringExperienceModule::instance = nullptr;

class GatheringExperienceCommandScript : public CommandScript
{
public:
    GatheringExperienceCommandScript() : CommandScript("GatheringExperienceCommandScript") { }

    Acore::ChatCommands::ChatCommandTable GetCommands() const override
    {
        static Acore::ChatCommands::ChatCommandTable gatheringCommandTable =
        {
            { "version",    HandleGatheringVersionCommand,   SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
            { "reload",     HandleGatheringReloadCommand,    SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
            { "add",        HandleGatheringAddCommand,       SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
            { "remove",     HandleGatheringRemoveCommand,    SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
            { "modify",     HandleGatheringModifyCommand,    SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
            { "list",       HandleGatheringListCommand,      SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
            { "zone",       HandleGatheringZoneCommand,      SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
            { "help",       HandleGatheringHelpCommand,      SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
            { "currentzone", HandleGatheringCurrentZoneCommand, SEC_GAMEMASTER,  Acore::ChatCommands::Console::No },
            { "toggle",     HandleGatheringToggleProfessionCommand, SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
            { "status",     HandleGatheringStatusCommand,           SEC_GAMEMASTER,  Acore::ChatCommands::Console::Yes },
        };

        static Acore::ChatCommands::ChatCommandTable commandTable =
        {
            { "gathering", gatheringCommandTable }
        };

        return commandTable;
    }

    static bool HandleGatheringVersionCommand(ChatHandler* handler, const char* /*args*/)
    {
        handler->PSendSysMessage("Gathering Experience Module Version: {}", GATHERING_EXPERIENCE_VERSION);
        return true;
    }

    static bool HandleGatheringReloadCommand(ChatHandler* handler, const char* /*args*/)
    {
        LOG_INFO("server.loading", "Manual reload command triggered");
        if (!GatheringExperienceModule::instance)
        {
            handler->PSendSysMessage("Failed to reload Gathering Experience data.");
            return false;
        }

        GatheringExperienceModule::instance->LoadDataFromDB();
        handler->PSendSysMessage("Gathering Experience data reloaded from database.");
        return true;
    }

    static bool HandleGatheringAddCommand(ChatHandler* handler, const char* args)
    {
        char* itemIdStr = strtok((char*)args, " ");
        char* baseXPStr = strtok(nullptr, " ");
        char* reqSkillStr = strtok(nullptr, " ");
        char* professionStr = strtok(nullptr, " ");
        char* nameStr = strtok(nullptr, "\n");

        if (!itemIdStr || !baseXPStr || !reqSkillStr || !professionStr || !nameStr)
        {
            handler->SendSysMessage("Usage: .gathering add <itemId> <baseXP> <requiredSkill> <profession> <name>");
            handler->SendSysMessage("Professions: Mining, Herbalism, Skinning, Fishing");
            return false;
        }

        uint32 itemId = atoi(itemIdStr);
        uint32 baseXP = atoi(baseXPStr);
        uint32 reqSkill = atoi(reqSkillStr);
        std::string profName = professionStr;

        uint8 professionId = GetProfessionIdByName(profName);
        if (professionId == 0)
        {
            handler->PSendSysMessage("Invalid profession: {}", profName);
            handler->SendSysMessage("Valid professions: Mining, Herbalism, Skinning, Fishing");
            return false;
        }

        WorldDatabase.DirectExecute("INSERT INTO gathering_experience (item_id, base_xp, required_skill, profession, name) VALUES ({}, {}, {}, {}, '{}')",
            itemId, baseXP, reqSkill, professionId, nameStr);

        if (!GatheringExperienceModule::instance)
        {
            handler->PSendSysMessage("Failed to reload Gathering Experience data.");
            return false;
        }

        GatheringExperienceModule::instance->LoadDataFromDB();
        handler->PSendSysMessage("Added gathering item {} to database for profession {} and reloaded data.", itemId, profName);
        return true;
    }

    static bool HandleGatheringRemoveCommand(ChatHandler* handler, const char* args)
    {
        char* itemIdStr = strtok((char*)args, " ");
        if (!itemIdStr)
        {
            handler->SendSysMessage("Usage: .gathering remove <itemId>");
            return false;
        }

        uint32 itemId = atoi(itemIdStr);

        // Get item details before removing
        QueryResult result = WorldDatabase.Query(
            "SELECT ge.item_id, ge.base_xp, ge.required_skill, gep.name as prof_name, ge.name "
            "FROM gathering_experience ge "
            "JOIN gathering_experience_professions gep ON ge.profession = gep.profession_id "
            "WHERE ge.item_id = {}", itemId);

        if (!result)
        {
            handler->PSendSysMessage("Item ID {} not found in gathering database.", itemId);
            return false;
        }

        WorldDatabase.DirectExecute("DELETE FROM gathering_experience WHERE item_id = {}", itemId);

        if (!GatheringExperienceModule::instance)
        {
            handler->PSendSysMessage("Failed to reload Gathering Experience data.");
            return false;
        }

        GatheringExperienceModule::instance->LoadDataFromDB();
        handler->PSendSysMessage("Removed gathering item {} and reloaded data.", itemId);
        return true;
    }

    static bool HandleGatheringModifyCommand(ChatHandler* handler, const char* args)
    {
        char* itemIdStr = strtok((char*)args, " ");
        char* fieldStr = strtok(nullptr, " ");
        char* valueStr = strtok(nullptr, " ");

        if (!itemIdStr || !fieldStr || !valueStr)
        {
            handler->SendSysMessage("Usage: .gathering modify <itemId> <field> <value>");
            handler->SendSysMessage("Fields: basexp, reqskill, profession, name");
            handler->SendSysMessage("Example: .gathering modify 2770 profession Mining");
            return false;
        }

        uint32 itemId = atoi(itemIdStr);
        std::string field = fieldStr;
        std::string value = valueStr;

        // First check if item exists
        QueryResult checkItem = WorldDatabase.Query("SELECT 1 FROM gathering_experience WHERE item_id = {}", itemId);
        if (!checkItem)
        {
            handler->PSendSysMessage("Item ID {} not found in gathering database.", itemId);
            return false;
        }

        std::string query = "UPDATE gathering_experience SET ";
        if (field == "basexp")
        {
            query += Acore::StringFormat("base_xp = {}", atoi(value.c_str()));
        }
        else if (field == "reqskill")
        {
            query += Acore::StringFormat("required_skill = {}", atoi(value.c_str()));
        }
        else if (field == "profession")
        {
            uint8 professionId = GetProfessionIdByName(value);
            if (professionId == 0)
            {
                handler->PSendSysMessage("Invalid profession: {}", value);
                handler->SendSysMessage("Valid professions: Mining, Herbalism, Skinning, Fishing");
                return false;
            }
            query += Acore::StringFormat("profession = {}", professionId);
        }
        else if (field == "name")
        {
            query += Acore::StringFormat("name = '{}'", value.c_str());
        }
        else
        {
            handler->SendSysMessage("Invalid field specified.");
            handler->SendSysMessage("Valid fields: basexp, reqskill, profession, name");
            return false;
        }

        query += Acore::StringFormat(" WHERE item_id = {}", itemId);
        WorldDatabase.DirectExecute(query);
        
        // Force reload of data after modification
        if (!GatheringExperienceModule::instance)
        {
            handler->PSendSysMessage("Failed to reload data after modifying item {}.", itemId);
            return false;
        }

        GatheringExperienceModule::instance->LoadDataFromDB();
        
        // Show updated item details AFTER reload
        QueryResult result = WorldDatabase.Query(
            "SELECT ge.item_id, ge.base_xp, ge.required_skill, gep.name as prof_name, ge.name "
            "FROM gathering_experience ge "
            "JOIN gathering_experience_professions gep ON ge.profession = gep.profession_id "
            "WHERE ge.item_id = {}", itemId);

        if (result)
        {
            Field* fields = result->Fetch();
            handler->PSendSysMessage("Updated values - ItemID: {}, BaseXP: {}, ReqSkill: {}, Profession: {}, Name: {}",
                fields[0].Get<uint32>(),
                fields[1].Get<uint32>(),
                fields[2].Get<uint32>(),
                fields[3].Get<std::string>(),
                fields[4].Get<std::string>());
        }

        return true;
    }

    static bool HandleGatheringListCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
        {
            // List all items
            QueryResult result = WorldDatabase.Query(
                "SELECT ge.item_id, ge.base_xp, ge.required_skill, gep.name as prof_name, ge.name "
                "FROM gathering_experience ge "
                "JOIN gathering_experience_professions gep ON ge.profession = gep.profession_id "
                "ORDER BY gep.name, ge.required_skill");

            if (!result)
            {
                handler->SendSysMessage("No gathering items found.");
                return true;
            }

            handler->SendSysMessage("Current gathering items:");
            do
            {
                Field* fields = result->Fetch();
                handler->PSendSysMessage("ItemID: {}, BaseXP: {}, ReqSkill: {}, Profession: {}, Name: {}",
                    fields[0].Get<uint32>(),    // ItemID
                    fields[1].Get<uint32>(),    // BaseXP
                    fields[2].Get<uint32>(),    // ReqSkill
                    fields[3].Get<std::string>(), // Profession name
                    fields[4].Get<std::string>()); // Item name
            } while (result->NextRow());
        }
        else
        {
            // List items for specific profession
            std::string profName = args;
            // Convert to lowercase for case-insensitive comparison
            std::transform(profName.begin(), profName.end(), profName.begin(), ::tolower);

            QueryResult result = WorldDatabase.Query(
                "SELECT ge.item_id, ge.base_xp, ge.required_skill, gep.name as prof_name, ge.name "
                "FROM gathering_experience ge "
                "JOIN gathering_experience_professions gep ON ge.profession = gep.profession_id "
                "WHERE LOWER(gep.name) = '{}' "
                "ORDER BY ge.required_skill", profName);

            if (!result)
            {
                handler->PSendSysMessage("No gathering items found for profession: {}", args);
                return true;
            }

            handler->PSendSysMessage("Current gathering items for {}:", args);
            do
            {
                Field* fields = result->Fetch();
                handler->PSendSysMessage("ItemID: {}, BaseXP: {}, ReqSkill: {}, Name: {}",
                    fields[0].Get<uint32>(),    // ItemID
                    fields[1].Get<uint32>(),    // BaseXP
                    fields[2].Get<uint32>(),    // ReqSkill
                    fields[4].Get<std::string>()); // Item name
            } while (result->NextRow());
        }

        return true;
    }

    // Helper function to get profession ID from name
    static uint8 GetProfessionIdByName(const std::string& name)
    {
        QueryResult result = WorldDatabase.Query(
            "SELECT profession_id FROM gathering_experience_professions WHERE LOWER(name) = LOWER('{}')",
            name);

        if (result)
            return result->Fetch()[0].Get<uint8>();
        return 0;
    }

    // Helper function to validate profession name
    static bool IsValidProfession(const std::string& name)
    {
        return GetProfessionIdByName(name) != 0;
    }

    static bool HandleGatheringHelpCommand(ChatHandler* handler, const char* /*args*/)
    {
        handler->SendSysMessage("Gathering Experience Module Commands:");
        handler->SendSysMessage("  .gathering version - Shows module version");
        handler->SendSysMessage("  .gathering reload - Reloads data from database");
        handler->SendSysMessage("  .gathering add <itemId> <baseXP> <reqSkill> <profession> <name>");
        handler->SendSysMessage("  .gathering remove <itemId>");
        handler->SendSysMessage("  .gathering modify <itemId> <field> <value>");
        handler->SendSysMessage("    Fields: basexp, reqskill, profession, name");
        handler->SendSysMessage("  .gathering list [profession]");
        handler->SendSysMessage("  .gathering zone add <zoneId> <multiplier> <zoneName>");
        handler->SendSysMessage("  .gathering zone remove <zoneId>");
        handler->SendSysMessage("  .gathering zone modify <zoneId> <multiplier>");
        handler->SendSysMessage("  .gathering zone list");
        handler->SendSysMessage("  .gathering currentzone - Shows current zone info and multiplier");
        handler->SendSysMessage("  Valid professions: Mining, Herbalism, Skinning, Fishing");
        return true;
    }

    static bool HandleGatheringZoneCommand(ChatHandler* handler, const char* args)
    {
        if (!args || !*args)
        {
            handler->SendSysMessage("Usage:");
            handler->SendSysMessage("  .gathering zone add <zoneId> <multiplier> <zoneName>");
            handler->SendSysMessage("  .gathering zone remove <zoneId>");
            handler->SendSysMessage("  .gathering zone modify <zoneId> <multiplier>");
            handler->SendSysMessage("  .gathering zone list");
            return true;
        }

        char* actionStr = strtok((char*)args, " ");
        if (!actionStr)
        {
            handler->SendSysMessage("Invalid command format.");
            return false;
        }

        std::string action = actionStr;

        if (action == "list")
        {
            QueryResult result = WorldDatabase.Query(
                "SELECT zone_id, multiplier, name FROM gathering_experience_zones ORDER BY zone_id");
            if (!result)
            {
                handler->SendSysMessage("No zone multipliers found.");
                return true;
            }

            handler->SendSysMessage("Current zone multipliers:");
            do
            {
                Field* fields = result->Fetch();
                handler->PSendSysMessage("Zone: {} (ID: {}), Multiplier: {:.2f}x", 
                    fields[2].Get<std::string>(),  // name
                    fields[0].Get<uint32>(),       // zone_id
                    fields[1].Get<float>());       // multiplier
            } while (result->NextRow());
            return true;
        }

        char* zoneIdStr = strtok(nullptr, " ");
        if (!zoneIdStr)
        {
            handler->SendSysMessage("Zone ID required.");
            return false;
        }
        uint32 zoneId = atoi(zoneIdStr);

        if (action == "remove")
        {
            WorldDatabase.DirectExecute("DELETE FROM gathering_experience_zones WHERE zone_id = {}", zoneId);
            handler->PSendSysMessage("Removed multiplier for zone ID: {}", zoneId);
        }
        else if (action == "add" || action == "modify")
        {
            char* multiplierStr = strtok(nullptr, " ");
            if (!multiplierStr)
            {
                handler->SendSysMessage("Multiplier value required.");
                return false;
            }
            float multiplier = atof(multiplierStr);

            if (multiplier <= 0.0f)
            {
                handler->SendSysMessage("Multiplier must be greater than 0.");
                return false;
            }

            if (action == "add")
            {
                char* nameStr = strtok(nullptr, "\n");  // Get rest of string as zone name
                if (!nameStr)
                {
                    handler->SendSysMessage("Zone name required for add command.");
                    return false;
                }

                WorldDatabase.DirectExecute(
                    "REPLACE INTO gathering_experience_zones (zone_id, multiplier, name) VALUES ({}, {}, '{}')", 
                    zoneId, multiplier, nameStr);
                handler->PSendSysMessage("Added multiplier {:.2f}x for zone: {} (ID: {})", 
                    multiplier, nameStr, zoneId);
            }
            else // modify
            {
                WorldDatabase.DirectExecute(
                    "UPDATE gathering_experience_zones SET multiplier = {} WHERE zone_id = {}", 
                    multiplier, zoneId);
                
                // Get zone name for feedback message
                QueryResult result = WorldDatabase.Query(
                    "SELECT name FROM gathering_experience_zones WHERE zone_id = {}", zoneId);
                std::string zoneName = result ? result->Fetch()[0].Get<std::string>() : "Unknown";
                
                handler->PSendSysMessage("Modified multiplier to {:.2f}x for zone: {} (ID: {})", 
                    multiplier, zoneName, zoneId);
            }
        }
        else
        {
            handler->SendSysMessage("Invalid action. Use: add, remove, modify, or list");
            return false;
        }

        // Reload data
        if (GatheringExperienceModule::instance)
        {
            GatheringExperienceModule::instance->LoadDataFromDB();
        }
        return true;
    }

    static bool HandleGatheringCurrentZoneCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("This command can only be used in-game.");
            return true;
        }

        uint32 zoneId = player->GetZoneId();
        std::string zoneName = "Unknown";

        // Get zone name from AreaTableEntry
        if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(zoneId))
        {
            zoneName = area->area_name[0];
        }

        float multiplier = 1.0f;  // Default multiplier

        // Get the zone multiplier if it exists
        QueryResult result = WorldDatabase.Query(
            "SELECT multiplier FROM gathering_experience_zones WHERE zone_id = {}", 
            zoneId);

        if (result)
        {
            multiplier = result->Fetch()[0].Get<float>();
        }

        handler->PSendSysMessage("Current Zone: {} (ID: {})", zoneName, zoneId);
        handler->PSendSysMessage("Gathering Experience Multiplier: {:.2f}x", multiplier);

        return true;
    }

    static bool HandleGatheringToggleProfessionCommand(ChatHandler* handler, const char* args)
    {
        if (!args)
        {
            handler->SendSysMessage("Usage: .gathering toggle <profession>");
            handler->SendSysMessage("Valid professions: mining, herbalism, skinning, fishing");
            return true;
        }

        std::string profession = args;
        std::transform(profession.begin(), profession.end(), profession.begin(), ::tolower);

        if (!GatheringExperienceModule::instance)
        {
            handler->PSendSysMessage("Module instance not found.");
            return false;
        }

        bool newState;
        if (profession == "mining")
        {
            newState = GatheringExperienceModule::instance->ToggleMining();
            handler->PSendSysMessage("Mining experience is now {}.", newState ? "enabled" : "disabled");
        }
        else if (profession == "herbalism")
        {
            newState = GatheringExperienceModule::instance->ToggleHerbalism();
            handler->PSendSysMessage("Herbalism experience is now {}.", newState ? "enabled" : "disabled");
        }
        else if (profession == "skinning")
        {
            newState = GatheringExperienceModule::instance->ToggleSkinning();
            handler->PSendSysMessage("Skinning experience is now {}.", newState ? "enabled" : "disabled");
        }
        else if (profession == "fishing")
        {
            newState = GatheringExperienceModule::instance->ToggleFishing();
            handler->PSendSysMessage("Fishing experience is now {}.", newState ? "enabled" : "disabled");
        }
        else
        {
            handler->SendSysMessage("Invalid profession specified.");
            handler->SendSysMessage("Valid professions: mining, herbalism, skinning, fishing");
            return false;
        }

        return true;
    }

    static bool HandleGatheringStatusCommand(ChatHandler* handler, const char* /*args*/)
    {
        if (!GatheringExperienceModule::instance)
        {
            handler->PSendSysMessage("Module instance not found.");
            return false;
        }

        handler->SendSysMessage("Gathering Experience Settings:");
        handler->PSendSysMessage("Mining: {}", GatheringExperienceModule::instance->IsMiningEnabled() ? "Enabled" : "Disabled");
        handler->PSendSysMessage("Herbalism: {}", GatheringExperienceModule::instance->IsHerbalismEnabled() ? "Enabled" : "Disabled");
        handler->PSendSysMessage("Skinning: {}", GatheringExperienceModule::instance->IsSkinningEnabled() ? "Enabled" : "Disabled");
        handler->PSendSysMessage("Fishing: {}", GatheringExperienceModule::instance->IsFishingEnabled() ? "Enabled" : "Disabled");
        return true;
    }
};

void AddGatheringExperienceModuleScripts()
{
    LOG_INFO("server.loading", "Adding GatheringExperienceModule scripts");
    new GatheringExperienceModule();
    new GatheringExperienceCommandScript();
    LOG_INFO("server.loading", "GatheringExperienceModule scripts added");
}

void Addmod_gathering_experience()
{
    AddGatheringExperienceModuleScripts();
}
