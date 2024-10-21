#include "Define.h"
#include "Player.h"
#include "Creature.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "ObjectMgr.h"
#include "Log.h"

#ifndef GATHERINGEXPERIENCEHOOKS_H
#define GATHERINGEXPERIENCEHOOKS_H

class GatheringExperienceHooks : public PlayerScript, public WorldScript
{
public:
    GatheringExperienceHooks();
    void OnBeforeConfigLoad(bool reload) override;
    void OnLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid) override;
    void OnKillCreature(Player* player, Creature* creature);
};

#endif // GATHERINGEXPERIENCEHOOKS_H
