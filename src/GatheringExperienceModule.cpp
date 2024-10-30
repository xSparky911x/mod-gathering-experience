#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "GameEventMgr.h"
#include "SkillDiscovery.h"
#include "Config.h"
#include "Chat.h"
#include <iostream> // Include for logging
#include "ChatCommand.h"
#include "StringFormat.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include <thread> // Include for sleep

using namespace Acore::ChatCommands;

// Define the maximum level for gathering scaling
const uint32 GATHERING_MAX_LEVEL = 80;
const uint32 MAX_EXPERIENCE_GAIN = 25000;
const uint32 MIN_EXPERIENCE_GAIN = 10;
const char* GATHERING_EXPERIENCE_VERSION = "0.2";

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
    std::map<uint32, float> rarityMultipliers;
    std::map<uint32, float> zoneMultipliers;
    bool enabled;
    bool dataLoaded;

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

    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        enabled = sConfigMgr->GetOption<bool>("GatheringExperience.Enable", true);
        if (!enabled)
        {
            LOG_INFO("server.loading", "Gathering Experience Module is disabled by config.");
            return;
        }

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
            rarityMultipliers.clear();
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

            // Load rarity multipliers
            if (QueryResult result = WorldDatabase.Query("SELECT item_id, multiplier FROM gathering_experience_rarity"))
            {
                uint32 count = 0;
                do
                {
                    Field* fields = result->Fetch();
                    rarityMultipliers[fields[0].Get<uint32>()] = fields[1].Get<float>();
                    count++;
                } while (result->NextRow());
                LOG_INFO("server.loading", "Loaded {} rarity multipliers", count);
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
            rarityMultipliers.clear();
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
            std::string message = "This server is running the |cff4CFF00Gathering Experience|r module v" + std::string(GATHERING_EXPERIENCE_VERSION) + " by Thaxtin.";
            ChatHandler(player->GetSession()).SendSysMessage(message.c_str());
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
        float skillMultiplier;
        if (IsFishingItem(itemId)) {
            // Define fishing skill tiers
            uint32 effectiveRequiredSkill;
            if (currentSkill <= 75)
                effectiveRequiredSkill = 0;
            else if (currentSkill <= 150)
                effectiveRequiredSkill = 75;
            else if (currentSkill <= 225)
                effectiveRequiredSkill = 150;
            else if (currentSkill <= 300)
                effectiveRequiredSkill = 225;
            else
                effectiveRequiredSkill = 300;

            // Calculate base multiplier based on current tier
            float baseMultiplier;
            if (currentSkill <= 75)
                baseMultiplier = 1.0f;
            else if (currentSkill <= 150)
                baseMultiplier = 2.0f;
            else if (currentSkill <= 225)
                baseMultiplier = 3.0f;
            else if (currentSkill <= 300)
                baseMultiplier = 4.0f;
            else
                baseMultiplier = 5.0f;

            // Calculate skill difference within current tier
            uint32 skillDifference = (currentSkill > effectiveRequiredSkill) ? (currentSkill - effectiveRequiredSkill) : 0;
            
            // Add bonus based on progress within tier (up to 0.5 additional multiplier per tier)
            float progressBonus = (skillDifference * 0.5f) / 75.0f;  // 75 is the size of each tier
            skillMultiplier = baseMultiplier + progressBonus;

            LOG_DEBUG("module.gathering", "Fishing XP Calculation - Skill: {}, EffectiveRequired: {}, BaseMultiplier: {:.2f}, ProgressBonus: {:.2f}, FinalMultiplier: {:.2f}", 
                currentSkill, effectiveRequiredSkill, baseMultiplier, progressBonus, skillMultiplier);
        }
        else if (IsSkinningItem(itemId)) {
            // Keep existing skinning calculation
            skillMultiplier = 1.5f + (currentSkill * 0.004f);
            skillMultiplier = std::min(skillMultiplier, 3.0f);
        }
        else {
            // Mining/herbalism calculation
            if (currentSkill >= requiredSkill + 100) {
                skillMultiplier = 0.5f;
            }
            else if (currentSkill >= requiredSkill + 50) {
                // Green skill - standard multiplier
                skillMultiplier = 1.0f;
            }
            else if (currentSkill >= requiredSkill + 25) {
                // Yellow skill - increased multiplier
                skillMultiplier = 1.5f;
            }
            else {
                // Orange skill (at required) - maximum multiplier
                skillMultiplier = 2.0f;
            }
        }

        // New level multiplier calculation
        float levelMultiplier = static_cast<float>(player->GetLevel()) / 10.0f;  // Level 10 = 1.0x, Level 60 = 6.0x

        // Get rarity multiplier
        float rarityMultiplier = GetRarityMultiplier(itemId);

        // Apply zone-based multiplier for fishing
        float zoneMultiplier = IsFishingItem(itemId) ? GetFishingZoneMultiplier(player->GetZoneId()) : 1.0f;

        // Calculate final XP with scaling and diminishing returns
        uint32 scaledXP = static_cast<uint32>(baseXP * skillMultiplier * levelMultiplier * rarityMultiplier * zoneMultiplier);

        // Higher minimum XP for fishing (scales with skill level)
        uint32 minXP = IsFishingItem(itemId) ? 
            std::max(50u, static_cast<uint32>(50 * (1.0f + currentSkill * 0.002f))) : 
            MIN_EXPERIENCE_GAIN;
        
        // Apply minimum and maximum XP constraints
        uint32 finalXP = std::clamp(scaledXP, minXP, MAX_EXPERIENCE_GAIN);

        LOG_DEBUG("server.loading", "CalculateExperience - ItemId: {}, BaseXP: {}, CurrentSkill: {}, RequiredSkill: {}, SkillMultiplier: {:.2f}, LevelMultiplier: {:.2f}, RarityMultiplier: {:.2f}, ZoneMultiplier: {:.2f}, ScaledXP: {}, FinalXP: {}", 
                itemId, baseXP, currentSkill, requiredSkill, skillMultiplier, levelMultiplier, rarityMultiplier, zoneMultiplier, scaledXP, finalXP);

        return finalXP;
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

    // Function to apply rarity-based multipliers for special items
    float GetRarityMultiplier(uint32 itemId)
    {
        auto it = rarityMultipliers.find(itemId);
        return it != rarityMultipliers.end() ? it->second : 1.0f;
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
        
        // Add debug logging
        LOG_INFO("server.loading", "OnLootItem triggered for item {} - Checking if it's in gathering items...", itemId);
        LOG_INFO("server.loading", "Current number of gathering items in memory: {}", gatheringItems.size());
        
        auto [baseXP, requiredSkill] = GetGatheringBaseXPAndRequiredSkill(itemId);
        
        if (baseXP == 0)
        {
            LOG_INFO("server.loading", "Item {} not recognized as a gathering item", itemId);
            return;
        }

        // If we get here, the item was found in memory
        LOG_INFO("server.loading", "Found item {} in memory with baseXP {}", itemId, baseXP);

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
        auto it = gatheringItems.find(itemId);
        return it != gatheringItems.end() && it->second.profession == 4;
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
    bool HasGatheringItem(uint32 itemId) const
    {
        return gatheringItems.count(itemId) > 0;
    }
};

// Initialize the static member
GatheringExperienceModule* GatheringExperienceModule::instance = nullptr;

class GatheringExperienceCommandScript : public CommandScript
{
public:
    GatheringExperienceCommandScript() : CommandScript("GatheringExperienceCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable gatheringCommandTable =
        {
            { "version",     HandleGatheringVersionCommand,   SEC_GAMEMASTER,  Console::Yes },
            { "reload",      HandleGatheringReloadCommand,    SEC_GAMEMASTER,  Console::Yes },
            { "add",        HandleGatheringAddCommand,       SEC_GAMEMASTER,  Console::Yes },
            { "remove",     HandleGatheringRemoveCommand,    SEC_GAMEMASTER,  Console::Yes },
            { "modify",     HandleGatheringModifyCommand,    SEC_GAMEMASTER,  Console::Yes },
            { "list",       HandleGatheringListCommand,      SEC_GAMEMASTER,  Console::Yes },
            { "zone",       HandleGatheringZoneCommand,     SEC_GAMEMASTER,  Console::Yes },
            { "help",       HandleGatheringHelpCommand,     SEC_GAMEMASTER,  Console::Yes }
        };

        static ChatCommandTable commandTable =
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
