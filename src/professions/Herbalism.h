#ifndef MODULE_GATHERING_EXPERIENCE_HERBALISM_H
#define MODULE_GATHERING_EXPERIENCE_HERBALISM_H

#include "GatheringExperience.h"

class HerbalismExperience
{
public:
    static HerbalismExperience* instance();
    uint32 CalculateHerbalismExperience(Player* player, uint32 itemId);
    bool IsHerbalismItem(uint32 itemId) const;
    float GetRarityMultiplier(uint32 itemId) const;
};

#define sHerbalismExperience HerbalismExperience::instance()

#endif //MODULE_GATHERING_EXPERIENCE_HERBALISM_H 