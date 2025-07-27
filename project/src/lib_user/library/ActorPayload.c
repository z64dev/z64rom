//
// ActorPayload.c by z64me
//
// system for fetching polymorphic actor payloads from rooms
//

#include <uLib.h>

// will only be changed in this compilation unit
static u16 gNumSetupActorsSafe;

Asm_VanillaHook(Scene_CommandActorList);
void Scene_CommandActorList(PlayState* play, SceneCmd* cmd)
{
	play->numSetupActors = gNumSetupActorsSafe = cmd->actorList.length;
	play->setupActorList = SEGMENTED_TO_VIRTUAL(cmd->actorList.data);
}

void *Actor_GetPayload(Actor *actor, PlayState *play)
{
	// actor payloads start at end
	u32 *payloads = (u32*)(&play->setupActorList[gNumSetupActorsSafe]);
	
	return &payloads[(u16)(actor->params)];
}

