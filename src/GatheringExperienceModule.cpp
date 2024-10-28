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

using namespace Acore::ChatCommands;

// Define the maximum level for gathering scaling
const uint32 GATHERING_MAX_LEVEL = 80;
const uint32 MAX_EXPERIENCE_GAIN = 25000;
const uint32 MIN_EXPERIENCE_GAIN = 10;
const char* GATHERING_EXPERIENCE_VERSION = "1.1.0";

class GatheringExperienceModule : public PlayerScript, public WorldScript
{
private:
    struct GatheringData {
        uint32 baseXP;
        uint32 requiredSkill;
        uint8 profession;
        std::string name;
        float rarityMultiplier;
    };

    std::map<uint32, GatheringData> gatheringData;  // itemId -> data
    std::map<uint32, float> zoneMultipliers;        // zoneId -> multiplier

    void LoadGatheringData()
    {
        // Clear existing data
        gatheringData.clear();
        zoneMultipliers.clear();

        // Load base gathering data
        QueryResult result = WorldDatabase.Query("SELECT item_id, base_xp, required_skill, profession, name FROM gathering_experience");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 itemId = fields[0].Get<uint32>();
                GatheringData data;
                data.baseXP = fields[1].Get<uint32>();
                data.requiredSkill = fields[2].Get<uint32>();
                data.profession = fields[3].Get<uint8>();
                data.name = fields[4].Get<std::string>();
                data.rarityMultiplier = 1.0f; // Default multiplier
                gatheringData[itemId] = data;
            } while (result->NextRow());
        }

        // Load rarity multipliers
        result = WorldDatabase.Query("SELECT item_id, multiplier FROM gathering_experience_rarity");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 itemId = fields[0].Get<uint32>();
                float multiplier = fields[1].Get<float>();
                
                if (gatheringData.find(itemId) != gatheringData.end())
                {
                    gatheringData[itemId].rarityMultiplier = multiplier;
                }
            } while (result->NextRow());
        }

        // Load zone multipliers
        result = WorldDatabase.Query("SELECT zone_id, multiplier FROM gathering_experience_zones");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 zoneId = fields[0].Get<uint32>();
                float multiplier = fields[1].Get<float>();
                zoneMultipliers[zoneId] = multiplier;
            } while (result->NextRow());
        }
    }

    float GetZoneMultiplier(uint32 zoneId)
    {
        auto it = zoneMultipliers.find(zoneId);
        return it != zoneMultipliers.end() ? it->second : 1.0f;
    }

    bool IsValidGatheringItem(uint32 itemId, uint8 profession)
    {
        auto it = gatheringData.find(itemId);
        return (it != gatheringData.end() && it->second.profession == profession);
    }

    uint32 CalculateExperience(uint32 itemId, Player* player)
    {
        auto it = gatheringData.find(itemId);
        if (it == gatheringData.end())
            return 0;

        const GatheringData& data = it->second;
        float zoneMultiplier = GetZoneMultiplier(player->GetZoneId());
        
        // Calculate base experience with all multipliers
        uint32 experience = static_cast<uint32>(
            data.baseXP * 
            data.rarityMultiplier * 
            zoneMultiplier
        );

        return experience;
    }

public:
    GatheringExperienceModule() : PlayerScript("GatheringExperienceModule"), WorldScript("GatheringExperienceModule")
    {
        LoadGatheringData();
    }

    void OnAfterConfigLoad(bool reload) override
    {
        if (reload)
            LoadGatheringData();
    }

    void OnLootItem(Player* player, Item* item, uint32 count, ObjectGuid /*lootguid*/) override
    {
        if (!player || !item)
            return;

        uint32 itemId = item->GetEntry();
        uint32 experience = 0;

        // Check profession type based on item
        if (IsValidGatheringItem(itemId, 1)) // Mining
        {
            experience = CalculateExperience(itemId, player);
            player->UpdateGatherSkill(SKILL_MINING, experience);
        }
        else if (IsValidGatheringItem(itemId, 2)) // Herbalism
        {
            experience = CalculateExperience(itemId, player);
            player->UpdateGatherSkill(SKILL_HERBALISM, experience);
        }
        else if (IsValidGatheringItem(itemId, 3)) // Skinning
        {
            experience = CalculateExperience(itemId, player);
            player->UpdateGatherSkill(SKILL_SKINNING, experience);
        }
        else if (IsValidGatheringItem(itemId, 4)) // Fishing
        {
            experience = CalculateExperience(itemId, player);
            player->UpdateGatherSkill(SKILL_FISHING, experience);
        }

        if (experience > 0)
        {
            // Optional: Send message to player about experience gained
            std::string itemName = item->GetTemplate()->Name1;
            ChatHandler(player->GetSession()).PSendSysMessage("You gained %u experience in %s from gathering %s", 
                experience, 
                GetProfessionName(gatheringData[itemId].profession), 
                itemName.c_str());
        }
    }

private:
    const char* GetProfessionName(uint8 profession)
    {
        switch (profession)
        {
            case 1: return "Mining";
            case 2: return "Herbalism";
            case 3: return "Skinning";
            case 4: return "Fishing";
            default: return "Unknown";
        }
    }
};

class GatheringExperienceCommandScript : public CommandScript
{
public:
    GatheringExperienceCommandScript() : CommandScript("GatheringExperienceCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable gatheringCommandTable =
        {
            { "version", HandleGatheringVersionCommand, SEC_GAMEMASTER, Console::Yes }
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
};

void AddGatheringExperienceModuleScripts()
{
    new GatheringExperienceModule();
    new GatheringExperienceCommandScript();
}

void Addmod_gathering_experience()
{
    AddGatheringExperienceModuleScripts();
}