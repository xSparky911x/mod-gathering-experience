/*
*Copyright (C) 2024+ xSparky911x, Thaxtin, released under GNU AGPL v3 license: https://github.com/xSparky911x/mod-gathering-experience/blob/master/LICENSE
*/

#include "ScriptMgr.h"
#include "GatheringExperience.h"

class GatheringExperienceWorldScript : public WorldScript
{
public:
    GatheringExperienceWorldScript() : WorldScript("GatheringExperienceWorldScript") { }

    void OnStartup() override
    {
        GatheringExperienceModule::instance = new GatheringExperienceModule();
        GatheringExperienceModule::instance->LoadDataFromDB();
    }

    void OnShutdown() override
    {
        delete GatheringExperienceModule::instance;
        GatheringExperienceModule::instance = nullptr;
    }
};

void AddGatheringExperienceScripts()
{
    new GatheringExperienceWorldScript();
}