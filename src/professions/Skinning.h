#ifndef MOD_GATHERING_EXPERIENCE_SKINNING_H
#define MOD_GATHERING_EXPERIENCE_SKINNING_H

#include "Player.h"
#include "GatheringExperience.h"

class SkinningExperience
{
public:
    static SkinningExperience* instance();
    
    uint32 CalculateSkinningExperience(Player* player, uint32 itemId);
    bool IsSkinningItem(uint32 itemId) const;

private:
    SkinningExperience() = default;
    ~SkinningExperience() { }
    static SkinningExperience* _instance;
    
    float GetRarityMultiplier(uint32 itemId) const;
};

#define sSkinningExperience SkinningExperience::instance()

#endif 