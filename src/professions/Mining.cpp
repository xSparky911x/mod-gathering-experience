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
        return 0;

    if (!sGatheringExperience->IsMiningEnabled())
        return 0;

    auto gatherData = sGatheringExperience->GetGatheringData(itemId);
    if (!gatherData)
        return 0;

    uint32 baseXP = std::get<0>(*gatherData);
    uint32 requiredSkill = std::get<1>(*gatherData);
    uint16 playerSkill = player->GetSkillValue(SKILL_MINING);
    std::string itemName = std::get<3>(*gatherData);

    // Calculate progress bonus (0-30% based on skill)
    float progressBonus = std::min(0.3f, playerSkill / 450.0f);

    float rarityMult = GetRarityMultiplier(itemId);

    uint32 normalXP = static_cast<uint32>(baseXP * (1.0f + progressBonus) * rarityMult);
    uint32 finalXP = std::min(normalXP, MAX_EXPERIENCE_GAIN);

    // Detailed logging
    LOG_INFO("module", "Mining XP Calculation for {}:", player->GetName());
    LOG_INFO("module", "- Item: {} (Item ID: {})", itemName, itemId);
    LOG_INFO("module", "- Base XP: {}", baseXP);
    LOG_INFO("module", "- Progress Bonus: {}", progressBonus);
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