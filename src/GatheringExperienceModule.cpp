#include "Define.h"
#include "Player.h"
#include "Creature.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "GatheringExperienceHooks.h"
#include "GatheringExperienceUtils.h"
#include "GatheringExperienceXPValues.h"

class GatheringExperienceModule : public PlayerScript, public WorldScript
{
public:
    GatheringExperienceModule() : PlayerScript("GatheringExperienceModule"), WorldScript("GatheringExperienceModule") { }

    // OnWorldInitialize hook to log that the module is loaded
    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        LOG_INFO("module", "Gathering Experience Module Loaded");
    }
};

// Register the script so AzerothCore knows to use it
void AddGatheringExperienceModuleScripts()
{
    new GatheringExperienceModule();
}

void Addmod_gathering_experience()
{
    AddGatheringExperienceModuleScripts();
}
