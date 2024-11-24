#ifndef GATHERING_EXPERIENCE_COMMANDS_H
#define GATHERING_EXPERIENCE_COMMANDS_H

#include "ScriptMgr.h"
#include "Chat.h"
#include "GatheringExperience.h"

class GatheringExperienceCommandScript : public CommandScript
{
public:
    GatheringExperienceCommandScript() : CommandScript("GatheringExperienceCommandScript") { }

    Acore::ChatCommands::ChatCommandTable GetCommands() const override;

private:
    static bool HandleGatheringVersionCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringReloadCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringAddCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringRemoveCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringModifyCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringListCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringZoneCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringHelpCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringCurrentZoneCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringToggleProfessionCommand(ChatHandler* handler, const char* args);
    static bool HandleGatheringStatusCommand(ChatHandler* handler, const char* args);
};

#endif // GATHERING_EXPERIENCE_COMMANDS_H 