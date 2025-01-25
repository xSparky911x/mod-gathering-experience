#ifndef MOD_GATHERING_EXPERIENCE_FISHING_H
#define MOD_GATHERING_EXPERIENCE_FISHING_H

#include "Player.h"
#include "GatheringExperience.h"

class FishingExperience
{
public:
    static FishingExperience* instance();
    
    uint32 CalculateFishingExperience(Player* player, uint32 itemId);
    bool IsFishingItem(uint32 itemId) const;

private:
    FishingExperience() = default;
    ~FishingExperience() { }
    static FishingExperience* _instance;
    
    float GetRarityMultiplier(uint32 itemId) const;
};

#define sFishingExperience FishingExperience::instance()

#endif 