#include "GatheringExperience.h"
#include "GatheringExperienceCommands.h"
#include "ScriptMgr.h"

// Declare the function to register the module scripts
void AddGatheringExperienceModuleScripts()
{
    new GatheringExperienceModule();
    new GatheringExperienceCommandScript();
}

// This is the function that the module system looks for
void Addmod_gathering_experienceScripts()
{
    AddGatheringExperienceModuleScripts();
}