#include "Fishing.h"
#include "Player.h"
#include "ScriptMgr.h"

FishingExperience* FishingExperience::instance()
{
    static FishingExperience instance;
    return &instance;
}

uint32 FishingExperience::CalculateFishingExperience(Player* player, uint32 itemId)
{
    if (!player || !IsFishingItem(itemId))
        return 0;

    if (!sGatheringExperience->IsFishingEnabled())
        return 0;

    auto gatherData = sGatheringExperience->GetGatheringData(itemId);
    if (!gatherData)
        return 0;

    uint32 baseXP = std::get<0>(*gatherData);
    uint16 playerSkill = player->GetSkillValue(SKILL_FISHING);
    std::string itemName = std::get<3>(*gatherData);

    // Adjust base XP based on skill tiers
    uint32 adjustedBaseXP = baseXP;
    std::string adjustReason;

    if (playerSkill > 300)
    {
        uint32 skillBasedMin = 200;
        if (adjustedBaseXP < skillBasedMin)
        {
            adjustedBaseXP = skillBasedMin;
            adjustReason = "minimum for skill > 300";
        }
    }
    else if (playerSkill > 150)
    {
        uint32 skillBasedMin = 125;
        if (adjustedBaseXP < skillBasedMin)
        {
            adjustedBaseXP = skillBasedMin;
            adjustReason = "minimum for skill > 150";
        }
    }
    else if (playerSkill > 75)
    {
        uint32 skillBasedMin = 100;
        if (adjustedBaseXP < skillBasedMin)
        {
            adjustedBaseXP = skillBasedMin;
            adjustReason = "minimum for skill > 75";
        }
    }

    // Get recommended level for this fish based on base XP
    uint32 recommendedLevel = 1;
    if (baseXP >= 700)           recommendedLevel = 70;  // Northrend
    else if (baseXP >= 600)      recommendedLevel = 60;  // Outland
    else if (baseXP >= 500)      recommendedLevel = 50;  // High vanilla
    else if (baseXP >= 400)      recommendedLevel = 40;  // Mid-high vanilla
    else if (baseXP >= 300)      recommendedLevel = 30;  // Mid vanilla
    else if (baseXP >= 200)      recommendedLevel = 20;  // Low vanilla
    else                         recommendedLevel = 10;  // Beginner

    int32 levelDiff = player->GetLevel() - recommendedLevel;
    float levelPenalty = 1.0f;
    std::string penaltyReason;

    if (levelDiff < 0)  // Player is below recommended level
    {
        levelPenalty = std::max(0.4f, 1.0f - (std::abs(levelDiff) * 0.03f));
        penaltyReason = fmt::format("reduced by {}% (level {} < {})", 
            static_cast<int>((1.0f - levelPenalty) * 100), 
            player->GetLevel(), 
            recommendedLevel);
    }
    else if (levelDiff > 0)  // Player is above recommended level
    {
        levelPenalty = std::max(0.4f, 1.0f - (levelDiff * 0.03f));
        penaltyReason = fmt::format("reduced by {}% (level {} > {})", 
            static_cast<int>((1.0f - levelPenalty) * 100), 
            player->GetLevel(), 
            recommendedLevel);
    }

    // Calculate progress bonus (0-30% based on skill)
    float progressBonus = std::min(0.3f, playerSkill / 450.0f);

    // Get zone info
    std::string zoneName = "Unknown";
    uint32 zoneId = player->GetZoneId();
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(zoneId))
    {
        zoneName = area->area_name[0];
    }

    float zoneMult = sGatheringExperience->GetZoneMultiplier(zoneId);

    if (sGatheringExperience->IsCityZone(zoneId))
    {
        zoneMult *= 0.5f;  // 50% penalty in cities since they're safe zones
    }

    float rarityMult = GetRarityMultiplier(itemId);
    uint32 normalXP = static_cast<uint32>(adjustedBaseXP * levelPenalty * (1.0f + progressBonus) * zoneMult * rarityMult);
    uint32 finalXP = std::min(normalXP, MAX_EXPERIENCE_GAIN);

    // Logging
    LOG_INFO("module", "Fishing XP Calculation for {}:", player->GetName());
    LOG_INFO("module", "- Fish: {} (Item ID: {})", itemName, itemId);
    LOG_INFO("module", "- Zone: {} (ID: {}) {}", zoneName, zoneId, "");
    LOG_INFO("module", "- Base XP: {}", baseXP);
    LOG_INFO("module", "- Level Penalty: {} {}", levelPenalty, 
        levelPenalty < 1.0f ? fmt::format("({})", penaltyReason) : "");
    LOG_INFO("module", "- Skill Level: {}", playerSkill);
    LOG_INFO("module", "- Progress Bonus: {}", progressBonus);
    LOG_INFO("module", "- Zone Multiplier: {}", zoneMult);
    LOG_INFO("module", "- Normal XP: {}", normalXP);
    LOG_INFO("module", "- Final XP: {}", finalXP);
    if (rarityMult > 1.0f)
    {
        std::string rarityText = (rarityMult == 1.5f) ? "Rare" : "Uncommon";
        LOG_INFO("module", "- Rarity: {} (+{}% bonus)", 
            rarityText, 
            static_cast<int>((rarityMult - 1.0f) * 100));
    }

    return finalXP;
}

bool FishingExperience::IsFishingItem(uint32 itemId) const
{
    auto gatherData = sGatheringExperience->GetGatheringData(itemId);
    if (!gatherData)
        return false;

    return std::get<2>(*gatherData) == PROF_FISHING;
}

float FishingExperience::GetRarityMultiplier(uint32 itemId) const
{
    auto gatherData = sGatheringExperience->GetGatheringData(itemId);
    if (!gatherData)
        return 1.0f;

    uint8 rarity = std::get<4>(*gatherData);
    switch (rarity)
    {
        case 1:  // Uncommon
            return 1.25f;
        case 2:  // Rare
            return 1.5f;
        default: // Common or any invalid value
            return 1.0f;
    }
} 