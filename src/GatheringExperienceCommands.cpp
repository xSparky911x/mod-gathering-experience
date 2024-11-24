/*
*Copyright (C) 2024+ xSparky911x, Thaxtin, released under GNU AGPL v3 license: https://github.com/xSparky911x/mod-gathering-experience/blob/master/LICENSE
*/

#include "ScriptMgr.h"
#include "Chat.h"
#include "Config.h"
#include "GatheringExperience.h"

using namespace Acore::ChatCommands;

class GatheringExperienceCommandScript : public CommandScript
{
public:
    GatheringExperienceCommandScript() : CommandScript("GatheringExperienceCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable gatheringCommandTable =
        {
            { "version",     HandleGatheringVersionCommand,              SEC_GAMEMASTER,  Console::Yes },
            { "reload",      HandleGatheringReloadCommand,               SEC_GAMEMASTER,  Console::Yes },
            { "add",         HandleGatheringAddCommand,                  SEC_GAMEMASTER,  Console::Yes },
            { "remove",      HandleGatheringRemoveCommand,               SEC_GAMEMASTER,  Console::Yes },
            { "modify",      HandleGatheringModifyCommand,               SEC_GAMEMASTER,  Console::Yes },
            { "list",        HandleGatheringListCommand,                 SEC_GAMEMASTER,  Console::Yes },
            { "zone",        HandleGatheringZoneCommand,                 SEC_GAMEMASTER,  Console::Yes },
            { "help",        HandleGatheringHelpCommand,                 SEC_GAMEMASTER,  Console::Yes },
            { "currentzone", HandleGatheringCurrentZoneCommand,          SEC_GAMEMASTER,  Console::No  },
            { "toggle",      HandleGatheringToggleProfessionCommand,     SEC_GAMEMASTER,  Console::Yes },
            { "status",      HandleGatheringStatusCommand,               SEC_GAMEMASTER,  Console::Yes },
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

void AddGatheringExperienceCommandScripts()
{
    new GatheringExperienceCommandScript();
}