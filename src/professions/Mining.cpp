#include "Mining.h"
#include "Player.h"
#include "ScriptMgr.h"

MiningExperience* MiningExperience::instance()
{
    static MiningExperience instance;
    return &instance;
}

uint32 MiningExperience::CalculateMiningExperience(Player* player, uint32 itemId)
{
    if (!player || !IsMiningItem(itemId))
    {
        LOG_INFO("module", "Invalid player or not a mining item");
        return 0;
    }

    if (!sGatheringExperience->IsMiningEnabled())
    {
        LOG_INFO("module", "Mining XP is disabled");
        return 0;
    }

    auto gatherData = sGatheringExperience->GetGatheringData(itemId);
    if (!gatherData)
    {
        LOG_INFO("module", "No gather data found for item");
        return 0;
    }

    uint32 baseXP = std::get<0>(*gatherData);
    uint32 requiredSkill = std::get<1>(*gatherData);
    uint16 playerSkill = player->GetSkillValue(SKILL_MINING);
    std::string itemName = std::get<3>(*gatherData);

    // Calculate progress bonus (0-30% based on skill)
    float progressBonus = std::min(0.3f, playerSkill / 450.0f);

    // Get zone info
    std::string zoneName = "Unknown";
    uint32 zoneId = player->GetZoneId();
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(zoneId))
    {
        zoneName = area->area_name[0];
    }

    float rarityMult = GetRarityMultiplier(itemId);

    // Skill level multiplier
    float skillMultiplier;
    std::string skillColor;
    if (playerSkill < requiredSkill + 25)
    {
        skillMultiplier = 1.2f; // Orange skill - highest XP (challenging)
        skillColor = "Orange";
    }
    else if (playerSkill < requiredSkill + 50)
    {
        skillMultiplier = 1.0f; // Yellow skill - normal XP (moderate)
        skillColor = "Yellow";
    }
    else if (playerSkill < requiredSkill + 75)
    {
        skillMultiplier = 0.8f; // Green skill - reduced XP (easy)
        skillColor = "Green";
    }
    else
    {
        skillMultiplier = 0.5f; // Gray skill - minimal XP (trivial)
        skillColor = "Gray (Trivial)";
    }

    // Get recommended level for this item based on base XP
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

    if (levelDiff < -10)  // Player is below recommended level
    {
        float penaltyLevels = std::abs(levelDiff) - 10;  // Only count levels after -10
        levelPenalty = std::max(0.10f, 1.0f - (penaltyLevels * 0.02f)); // Penalty increases by 2% per level below recommendedto a max of 90% penalty
        penaltyReason = fmt::format("reduced by {}% (level {} < {})", 
            static_cast<int>((1.0f - levelPenalty) * 100), 
            player->GetLevel(), 
            recommendedLevel);
    }

    uint32 normalXP = static_cast<uint32>(baseXP * skillMultiplier * levelPenalty * (1.0f + progressBonus) * rarityMult);
    uint32 finalXP = std::min(normalXP, MAX_EXPERIENCE_GAIN);

    // Detailed logging
    LOG_INFO("module", "Mining XP Calculation for {}:", player->GetName());
    LOG_INFO("module", "- Item: {} (Item ID: {})", itemName, itemId);
    LOG_INFO("module", "- Base XP: {}", baseXP);
    LOG_INFO("module", "- Level Penalty: {} {}", levelPenalty, 
        levelPenalty < 1.0f ? fmt::format("({})", penaltyReason) : "");
    LOG_INFO("module", "- Skill Level: {} ({} - {})", playerSkill, skillColor, skillMultiplier);
    LOG_INFO("module", "- Progress Bonus: {}", progressBonus);
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

bool MiningExperience::IsMiningItem(uint32 itemId) const
{
    auto gatherData = sGatheringExperience->GetGatheringData(itemId);
    if (!gatherData)
        return false;

    return std::get<2>(*gatherData) == PROF_MINING;
}

float MiningExperience::GetRarityMultiplier(uint32 itemId) const
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