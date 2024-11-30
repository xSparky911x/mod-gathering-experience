#ifndef GATHERING_EXPERIENCE_H
#define GATHERING_EXPERIENCE_H

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "StringFormat.h"

// Constants
const uint32 GATHERING_MAX_LEVEL = 80;
const uint32 MAX_EXPERIENCE_GAIN = 25000;
const uint32 MIN_EXPERIENCE_GAIN = 10;
extern const char* GATHERING_EXPERIENCE_VERSION;

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

class GatheringExperienceModule : public PlayerScript, public WorldScript
{
private:
    // Constants for better readability and maintenance
    static constexpr uint32 TIER_SIZE = 75;
    static constexpr float PROGRESS_BONUS_RATE = 0.02f;
    static constexpr float BASE_ZONE_MULTIPLIER = 1.0f;

    // Skill tier thresholds
    static constexpr uint32 TIER_1_MAX = 75;
    static constexpr uint32 TIER_2_MAX = 150;
    static constexpr uint32 TIER_3_MAX = 225;
    static constexpr uint32 TIER_4_MAX = 300;

    struct GatheringItem {
        uint32 baseXP;
        uint32 requiredSkill;
        uint8 profession;
        std::string name;
        uint8 rarity;
    };

    std::map<uint32, GatheringItem> gatheringItems;
    std::map<uint32, float> zoneMultipliers;
    bool enabled{false};
    bool dataLoaded{false};

    bool miningEnabled{true};
    bool herbalismEnabled{true};
    bool skinningEnabled{true};
    bool fishingEnabled{true};

public:
    static GatheringExperienceModule* instance;

    GatheringExperienceModule() : 
        PlayerScript("GatheringExperienceModule"), 
        WorldScript("GatheringExperienceModule")
    {
        instance = this;
    }

    // Override functions
    void OnStartup() override;
    void OnBeforeConfigLoad(bool reload) override;
    void OnLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid) override;
    void OnAfterConfigLoad(bool reload) override;
    void OnLogin(Player* player) override;

    // Database loading
    void LoadDataFromDB();
    void LoadSettingsFromDB();
    void SaveSettingToDB(std::string const& profession, bool enabled);
    
    // Profession toggle functions
    bool ToggleMining();
    bool ToggleHerbalism();
    bool ToggleSkinning();
    bool ToggleFishing();
    
    // Status checks
    bool IsMiningEnabled() const { return miningEnabled; }
    bool IsHerbalismEnabled() const { return herbalismEnabled; }
    bool IsSkinningEnabled() const { return skinningEnabled; }
    bool IsFishingEnabled() const { return fishingEnabled; }
    
    // XP calculation functions
    uint32 CalculateExperience(Player* player, uint32 baseXP, uint32 requiredSkill, uint32 currentSkill, uint32 itemId);
    float GetZoneMultiplier(uint32 zoneId) const;

    bool IsEnabled() const { return enabled; }
    void SetEnabled(bool state) { enabled = state; }

    std::optional<std::tuple<uint32, uint32, uint8, std::string, uint8>> GetGatheringData(uint32 itemId) const
    {
        auto it = gatheringItems.find(itemId);
        if (it != gatheringItems.end())
        {
            return std::make_tuple(
                it->second.baseXP,
                it->second.requiredSkill,
                it->second.profession,
                it->second.name,
                it->second.rarity
            );
        }
        return std::nullopt;
    }

    void LoadGatheringData();

    bool IsGatheringItem(uint32 itemId) const
    {
        return gatheringItems.find(itemId) != gatheringItems.end();
    }

    bool IsCityZone(uint32 zoneId) const;

private:
    // Helper functions
    float GetFishingTierMultiplier(uint32 currentSkill) const;
    float CalculateProgressBonus(uint32 currentSkill);
    void LoadZoneData();
};

#define sGatheringExperience GatheringExperienceModule::instance

#endif // GATHERING_EXPERIENCE_H