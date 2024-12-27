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

    static bool HandleGatheringAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage("Usage: .gathering add #itemId #baseXP #requiredSkill #profession \"name\"");
            handler->SendSysMessage("Professions: 1=Mining, 2=Herbalism, 3=Skinning, 4=Fishing");
            handler->SendSysMessage("Example: .gathering add 2447 100 1 2 \"Peacebloom\"");
            return true;
        }

        char* itemIdStr = strtok((char*)args, " ");
        char* baseXPStr = strtok(NULL, " ");
        char* requiredSkillStr = strtok(NULL, " ");
        char* professionStr = strtok(NULL, " ");
        char* rawName = strtok(NULL, "\0");

        if (!itemIdStr || !baseXPStr || !requiredSkillStr || !professionStr || !rawName)
        {
            handler->SendSysMessage("Missing required arguments.");
            handler->SendSysMessage("Usage: .gathering add #itemId #baseXP #requiredSkill #profession \"name\"");
            return false;
        }

        uint32 itemId = atoi(itemIdStr);
        uint32 baseXP = atoi(baseXPStr);
        uint32 requiredSkill = atoi(requiredSkillStr);
        uint32 profession = atoi(professionStr);

        if (profession < 1 || profession > 4)
        {
            handler->SendSysMessage("Invalid profession ID. Must be 1-4.");
            return false;
        }

        // Get the raw name string
        std::string name = rawName;
        while (!name.empty() && (name[0] == '"' || name[0] == ' '))
            name = name.substr(1);
        while (!name.empty() && (name.back() == '"' || name.back() == ' '))
            name.pop_back();

        // Handle apostrophes in names
        std::string escapedName = name;
        size_t pos = 0;
        while ((pos = escapedName.find("'", pos)) != std::string::npos) {
            escapedName.replace(pos, 1, "''");
            pos += 2;
        }

        WorldDatabase.DirectExecute(
            "INSERT INTO gathering_experience (item_id, base_xp, required_skill, profession, name) "
            "VALUES ({}, {}, {}, {}, '{}')",
            itemId, baseXP, requiredSkill, profession, escapedName);

        // Force reload of data after adding
        if (!GatheringExperienceModule::instance)
        {
            handler->PSendSysMessage("Failed to reload data after adding item {}.", itemId);
            return false;
        }

        GatheringExperienceModule::instance->LoadDataFromDB();
        handler->PSendSysMessage("Added gathering experience entry for item {}.", itemId);
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
            "SELECT ge.item_id, ge.base_xp, ge.required_skill, gep.name as prof_name, ge.multiplier, ge.name "
            "FROM gathering_experience ge "
            "JOIN gathering_experience_professions gep ON ge.profession = gep.profession_id "
            "WHERE ge.item_id = {}", itemId);

        if (!result)
        {
            handler->PSendSysMessage("Item ID {} not found in gathering database.", itemId);
            return false;
        }

        WorldDatabase.DirectExecute("DELETE FROM gathering_experience WHERE item_id = {}", itemId);

        WorldDatabase.DirectExecute(
            "DELETE FROM gathering_experience_rarity WHERE item_id = {}", 
            itemId);

        // Force reload of data after removing
        if (!GatheringExperienceModule::instance)
        {
            handler->PSendSysMessage("Failed to reload data after removing item {}.", itemId);
            return false;
        }

        GatheringExperienceModule::instance->LoadDataFromDB();
        handler->PSendSysMessage("Removed gathering item {} and reloaded data.", itemId);
        return true;
    }

    static bool HandleGatheringModifyCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage("Usage: .gathering modify <itemId> <field> <value>");
            handler->SendSysMessage("Fields: basexp, reqskill, profession, multiplier, name");
            return true;
        }

        char* itemIdStr = strtok((char*)args, " ");
        char* fieldStr = strtok(nullptr, " ");

        if (!itemIdStr || !fieldStr)
        {
            handler->SendSysMessage("Usage: .gathering modify <itemId> <field> <value>");
            handler->SendSysMessage("Fields: basexp, reqskill, profession, multiplier, name");
            return false;
        }

        uint32 itemId = atoi(itemIdStr);
        std::string field = fieldStr;

        // First check if item exists
        QueryResult checkItem = WorldDatabase.Query("SELECT 1 FROM gathering_experience WHERE item_id = {}", itemId);
        if (!checkItem)
        {
            handler->PSendSysMessage("Item ID {} not found in gathering database.", itemId);
            return false;
        }

        std::string query = "UPDATE gathering_experience SET ";
        
        // Handle name field differently - don't split the value
        if (field == "name")
        {
            char* nameStr = strtok(nullptr, "\0");  // Get everything after "name"
            if (!nameStr)
            {
                handler->SendSysMessage("Name value required.");
                return false;
            }

            // Clean up the name by removing quotes only
            std::string itemName = nameStr;
            while (!itemName.empty() && (itemName[0] == '"' || itemName[0] == ' '))
                itemName = itemName.substr(1);
            while (!itemName.empty() && (itemName.back() == '"' || itemName.back() == ' '))
                itemName.pop_back();

            query += Acore::StringFormat("name = '{}'", itemName);
        }
        else  // Handle other fields normally
        {
            char* valueStr = strtok(nullptr, " ");
            if (!valueStr)
            {
                handler->SendSysMessage("Value required.");
                return false;
            }
            std::string value = valueStr;

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
            else if (field == "multiplier")
            {
                float multiplier = atof(value.c_str());
                if (multiplier <= 0.0f)
                {
                    handler->SendSysMessage("Multiplier must be greater than 0.");
                    return false;
                }
                query += Acore::StringFormat("multiplier = {}", multiplier);
            }
            else
            {
                handler->SendSysMessage("Invalid field specified.");
                handler->SendSysMessage("Valid fields: basexp, reqskill, profession, multiplier, name");
                return false;
            }
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
            "SELECT ge.item_id, ge.base_xp, ge.required_skill, gep.name as prof_name, COALESCE(ger.multiplier, 1.0) as multiplier, ge.name "
            "FROM gathering_experience ge "
            "JOIN gathering_experience_professions gep ON ge.profession = gep.profession_id "
            "LEFT JOIN gathering_experience_rarity ger ON ge.item_id = ger.item_id "
            "WHERE ge.item_id = {}", itemId);

        if (result)
        {
            Field* fields = result->Fetch();
            handler->PSendSysMessage("Updated values - ItemID: {}, BaseXP: {}, ReqSkill: {}, Profession: {}, Multiplier: {:.2f}, Name: {}",
                fields[0].Get<uint32>(),
                fields[1].Get<uint32>(),
                fields[2].Get<uint32>(),
                fields[3].Get<std::string>(),
                fields[4].Get<float>(),
                fields[5].Get<std::string>());
        }

        return true;
    }

    static bool HandleGatheringListCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            // List all items
            QueryResult result = WorldDatabase.Query(
                "SELECT ge.item_id, ge.base_xp, ge.required_skill, gep.name as prof_name, COALESCE(ger.multiplier, 1.0) as multiplier, ge.name "
                "FROM gathering_experience ge "
                "JOIN gathering_experience_professions gep ON ge.profession = gep.profession_id "
                "LEFT JOIN gathering_experience_rarity ger ON ge.item_id = ger.item_id "
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
                handler->PSendSysMessage("ItemID: {}, BaseXP: {}, ReqSkill: {}, Profession: {}, Multiplier: {:.2f}, Name: {}",
                    fields[0].Get<uint32>(),    // ItemID
                    fields[1].Get<uint32>(),    // BaseXP
                    fields[2].Get<uint32>(),    // ReqSkill
                    fields[3].Get<std::string>(), // Profession name
                    fields[4].Get<float>(),    // Multiplier
                    fields[5].Get<std::string>()); // Item name
            } while (result->NextRow());
        }
        else
        {
            // List items for specific profession
            std::string profName = args;
            // Convert to lowercase for case-insensitive comparison
            std::transform(profName.begin(), profName.end(), profName.begin(), ::tolower);

            QueryResult result = WorldDatabase.Query(
                "SELECT ge.item_id, ge.base_xp, ge.required_skill, gep.name as prof_name, COALESCE(ger.multiplier, 1.0) as multiplier, ge.name "
                "FROM gathering_experience ge "
                "JOIN gathering_experience_professions gep ON ge.profession = gep.profession_id "
                "LEFT JOIN gathering_experience_rarity ger ON ge.item_id = ger.item_id "
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
                handler->PSendSysMessage("ItemID: {}, BaseXP: {}, ReqSkill: {}, Multiplier: {:.2f}, Name: {}",
                    fields[0].Get<uint32>(),    // ItemID
                    fields[1].Get<uint32>(),    // BaseXP
                    fields[2].Get<uint32>(),    // ReqSkill
                    fields[4].Get<float>(),    // Multiplier
                    fields[5].Get<std::string>()); // Item name
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
        handler->SendSysMessage("  .gathering add <itemId> <baseXP> <reqSkill> <profession> <multiplier> <name>");
        handler->SendSysMessage("  .gathering remove <itemId>");
        handler->SendSysMessage("  .gathering modify <itemId> <field> <value>");
        handler->SendSysMessage("  .gathering list [profession]");
        handler->SendSysMessage("  .gathering zone <zoneId> <multiplier>");
        handler->SendSysMessage("  .gathering currentzone");
        handler->SendSysMessage("  .gathering toggle <profession>");
        handler->SendSysMessage("  .gathering status");
        handler->SendSysMessage("Fields for modify: basexp, reqskill, profession, multiplier, name");
        return true;
    }

    static bool HandleGatheringZoneCommand(ChatHandler* handler, const char* args)
    {
        if (!args || !*args)
        {
            handler->SendSysMessage("Usage:");
            handler->SendSysMessage("  .gathering zone add <zoneId> <multiplier> <zoneName>");
            handler->SendSysMessage("  .gathering zone remove <zoneId>");
            handler->SendSysMessage("  .gathering zone modify <zoneId> <field> <value>");
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
            // Get zone name before removing
            QueryResult result = WorldDatabase.Query(
                "SELECT name FROM gathering_experience_zones WHERE zone_id = {}", zoneId);
            
            if (!result)
            {
                handler->PSendSysMessage("Zone ID {} not found in database.", zoneId);
                return false;
            }

            std::string zoneName = result->Fetch()[0].Get<std::string>();
            
            WorldDatabase.DirectExecute("DELETE FROM gathering_experience_zones WHERE zone_id = {}", zoneId);
            handler->PSendSysMessage("Removed multiplier for zone: {} (ID: {})", zoneName, zoneId);
        }
        else if (action == "add" || action == "modify")
        {
            char* fieldStr = strtok(nullptr, " ");
            char* valueStr = strtok(nullptr, " ");

            if (!fieldStr || !valueStr)
            {
                handler->SendSysMessage("Usage: .gathering zone modify <zoneId> <field> <value>");
                handler->SendSysMessage("Fields: multiplier, name");
                return false;
            }

            std::string field = fieldStr;
            std::string query = "UPDATE gathering_experience_zones SET ";

            if (field == "multiplier")
            {
                float multiplier = atof(valueStr);
                if (multiplier <= 0.0f)
                {
                    handler->SendSysMessage("Multiplier must be greater than 0.");
                    return false;
                }
                query += Acore::StringFormat("multiplier = {}", multiplier);
            }
            else if (field == "name")
            {
                // Get the rest of the string after "name"
                char* nameStr = strtok(nullptr, "\0");
                if (!nameStr)
                {
                    handler->SendSysMessage("Name value required.");
                    return false;
                }

                // Clean up the name by removing quotes
                std::string zoneName = nameStr;
                while (!zoneName.empty() && (zoneName[0] == '"' || zoneName[0] == ' '))
                    zoneName = zoneName.substr(1);
                while (!zoneName.empty() && (zoneName.back() == '"' || zoneName.back() == ' '))
                    zoneName.pop_back();

                // Handle apostrophes
                std::string escapedName = zoneName;
                size_t pos = 0;
                while ((pos = escapedName.find("'", pos)) != std::string::npos) {
                    escapedName.replace(pos, 1, "''");
                    pos += 2;
                }

                query += Acore::StringFormat("name = '{}'", escapedName);
            }
            else
            {
                handler->SendSysMessage("Invalid field specified.");
                handler->SendSysMessage("Valid fields: multiplier, name");
                return false;
            }

            query += Acore::StringFormat(" WHERE zone_id = {}", zoneId);
            WorldDatabase.DirectExecute(query);

            // Get updated zone info for feedback message
            QueryResult result = WorldDatabase.Query(
                "SELECT name, multiplier FROM gathering_experience_zones WHERE zone_id = {}", zoneId);
            if (result)
            {
                Field* fields = result->Fetch();
                handler->PSendSysMessage("Updated zone: {} (ID: {}) - Multiplier: {:.2f}x", 
                    fields[0].Get<std::string>(), zoneId, fields[1].Get<float>());
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
        uint32 areaId = player->GetAreaId();  // Get the area ID too
        std::string zoneName = "Unknown";

        // Get zone name from AreaTableEntry
        if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(zoneId))
        {
            zoneName = area->area_name[0];
            handler->PSendSysMessage("Debug - Area Entry Found: {} (ID: {})", area->area_name[0], zoneId);
        }
        else
        {
            handler->PSendSysMessage("Debug - No Area Entry Found for ID: {}", zoneId);
        }

        // Also show area info
        if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId))
        {
            handler->PSendSysMessage("Debug - Current Area: {} (ID: {})", area->area_name[0], areaId);
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

    static bool HandleGatheringZoneAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage("Usage: .gathering zone add #zoneId #multiplier #name");
            return true;
        }

        char* zoneIdStr = strtok((char*)args, " ");
        char* multiplierStr = strtok(nullptr, " ");
        char* nameStr = strtok(nullptr, "\0");  // Get the rest of the string

        if (!zoneIdStr || !multiplierStr || !nameStr)
        {
            handler->SendSysMessage("Missing parameters!");
            handler->SendSysMessage("Usage: .gathering zone add #zoneId #multiplier #name");
            return true;
        }

        uint32 zoneId = atoi(zoneIdStr);
        float multiplier = atof(multiplierStr);
        
        // Clean up the name by removing quotes only
        std::string zoneName = nameStr;
        if (!zoneName.empty() && zoneName[0] == '"')
            zoneName = zoneName.substr(1);
        if (!zoneName.empty() && zoneName.back() == '"')
            zoneName.pop_back();

        WorldDatabase.DirectExecute("REPLACE INTO gathering_experience_zones (zone_id, multiplier, name) VALUES ({}, {}, '{}')",
            zoneId, multiplier, zoneName);

        handler->PSendSysMessage("Added multiplier {:.2f}x for zone: {} (ID: {})", 
            multiplier, zoneName, zoneId);

        return true;
    }
};

void AddGatheringExperienceCommandScripts()
{
    new GatheringExperienceCommandScript();
}