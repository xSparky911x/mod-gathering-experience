#ifndef MODULE_GATHERING_EXPERIENCE_MINING_H
#define MODULE_GATHERING_EXPERIENCE_MINING_H

#include "GatheringExperience.h"

class MiningExperience
{
public:
    static MiningExperience* instance();
    uint32 CalculateMiningExperience(Player* player, uint32 itemId);
    bool IsMiningItem(uint32 itemId) const;
    float GetRarityMultiplier(uint32 itemId) const;
};

#define sMiningExperience MiningExperience::instance()

#endif //MODULE_GATHERING_EXPERIENCE_MINING_H 