#include "GatheringExperience.h"
#include "GatheringExperienceCommands.h"
#include "professions/Fishing.h"
#include "professions/Skinning.h"
#include "ScriptMgr.h"

// Declare the function to register the module scripts
void AddGatheringExperienceModuleScripts()
{
    new GatheringExperienceModule();
    new GatheringExperienceCommandScript();
    // Initialize the profession singletons
    sFishingExperience->instance();
    sSkinningExperience->instance();
}

// This is the function that the module system looks for
void Addmod_gathering_experienceScripts()
{
    AddGatheringExperienceModuleScripts();
}