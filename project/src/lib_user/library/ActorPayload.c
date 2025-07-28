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

void *(Actor_GetPayload)(Actor *actor, PlayState *play)
{
	// actor payload data starts immediately after actor setup list
	u32 *payloadBlock = (u32*)(&play->setupActorList[gNumSetupActorsSafe]);
	
	return &payloadBlock[(u16)(actor->params)];
}

void *Actor_GetPayloadAndDoRelocs(Actor *actor, PlayState *play, u8 *relocs)
{
	void *oldSeg3 = gSegments[3];
	u8 *data = (Actor_GetPayload)(actor, play);
	
	gSegments[3] = VIRTUAL_TO_PHYSICAL(play->roomCtx.unk_34);
	for ( ; *relocs != 0xff; ++relocs)
	{
		u32 *tmp = ((u32*)data) + *relocs;
		
		*tmp = (u32)SEGMENTED_TO_VIRTUAL(*tmp);
	}
	
	gSegments[3] = oldSeg3;
	return data;
}

