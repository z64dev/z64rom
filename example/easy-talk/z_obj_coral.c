/*
 * File: z_obj_coral.c
 * Overlay: ovl_obj_coral
 * Description: Coral Buddies from the titular game, Fortnite.
 * Easy-Talk example actor from z64rom 1.7.0's update
 */

#include "z_obj_coral.h"
#include "global.h"
#include <uLib.h>
#include "vt.h"
#include "message_data_fmt.h"

#define ACTOR_FLAG_TARGETABLE    (1 << 0)
#define ACTOR_FLAG_FRIENDLY      (1 << 3)

#define OBJ_ID 0x0353
#define ACT_ID 0x035C
#define Actor_OfferGetItem func_8002F434

#define SWITCH_FLAG ((en->actor.params >> 8) & 0xFF)
#define TYPE (en->actor.params & 7)
#define STATE ((en->actor.params >> 4) & 7)
//#define FLAGS (ACTOR_FLAG_3 | ACTOR_FLAG_4)

#define FLAGS (ACTOR_FLAG_3 | ACTOR_FLAG_4 | ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY)

typedef struct EasyMessage
{
	const char *text;
	u16 id; // value starting at 0xA000, unique for each
	struct EasyMessage *choices;
	ActorFunc onTextBegin;
	ActorFunc ontextEnd;
} EasyMessage;

EasyMessage msgPickZtargeting = {
	.text = "So, how do you like your Z-Targeting?\x01\x01\x1B\x05\x42Hold\x01Switch\x05\x40\x02",
	.id = 0xA000,
	.choices = (EasyMessage[]) {
		{ "Z-Targeting style is now \x05\x42Hold\x05\x40.\x02" },
		{ "Z-Targeting style is now \x05\x42Switch\x05\x40.\x02" },
	},
};

EasyMessage msgTestMultipleBoxes = {
	.text = "Hello world\x04Hello Zelda fans\x04I am 13 and what is this?\x02",
	.id = 0xA001,
};

EasyMessage msgTestCorals = {
	.text = "I'm NOT soap!\x04""Do I ALWAYS have to \x01remind you guys this!\x04Please, for the love of god,\x01go kill yourself!\0x04...in Minecraft.\x02",
	.id = 0xA002,
};

EasyMessage msgTestBoo = {
	.text = "\x12\x28\xdf\x1A\x13\x03\x08\x59\x6F\x75\x20\x66\x6F\x75\x6E\x64\x20\x74\x68\x65\x20\x05\x41\x46\x61\x69\x72\x79\x20\x42\x6F\x77\x05\x40\x21\x09\x01\x4F\x6E\x20\x74\x68\x65\x20\x05\x46\x53\x65\x6C\x65\x63\x74\x20\x49\x74\x65\x6D\x20\x53\x75\x62\x73\x63\x72\x65\x65\x6E\x05\x40\x2C\x01\x79\x6F\x75\x20\x63\x61\x6E\x20\x73\x65\x74\x20\x69\x74\x20\x74\x6F\x20\x05\x46\xA7\x05\x40\x2C\x20\x05\x46\xA6\x05\x40\x20\x6F\x72\x05\x46\x20\xA8\x05\x40\x2E\x04\x1A\x13\x03\x50\x72\x65\x73\x73\x20\x05\x46\xA1\x20\x05\x40\x74\x6F\x20\x74\x61\x6B\x65\x20\x69\x74\x20\x6F\x75\x74\x01\x61\x6E\x64\x20\x68\x6F\x6C\x64\x20\x69\x74\x2E\x20\x41\x73\x20\x79\x6F\x75\x20\x68\x6F\x6C\x64\x20\x64\x6F\x77\x6E\x01\x05\x46\xA1\x05\x40\x20\x79\x6F\x75\x20\x63\x61\x6E\x20\x61\x69\x6D\x20\x77\x69\x74\x68\x20\x05\x44\xAA\x05\x40\x2E\x05\x46\x20\x05\x40\x52\x65\x6C\x65\x61\x73\x65\x01\x05\x46\xA1\x05\x40\x20\x74\x6F\x20\x6C\x65\x74\x20\x66\x6C\x79\x20\x77\x69\x74\x68\x20\x61\x6E\x20\x05\x41\x61\x72\x72\x6F\x77\x05\x40\x2E\x02",
	.id = 0xA002,
};

static void TextOverride(PlayState* play, const char* string)
{
    Font* font = &play->msgCtx.font;
    char *tmp = font->msgBuf;
    
    for (bool hasFinished = false; !hasFinished; )
    {
        int copyBytes = 1;
        
        switch (*string)
        {
            // alias 00 -> 02 in case users forget to manually terminate
            // (writing into rodata like this is safe on n64)
            case 0x00: (*(char*)string) = 0x02; hasFinished = true; break;
            
            // end-of-message terminator
            case 0x02: hasFinished = true; break;
            
            // account for multi-byte commands possibly containing terminator
            case 0x05: case 0x13: case 0x0E: case 0x0C:
            case 0x1E: case 0x06: case 0x14:
                copyBytes = 2; break;
            case 0x07: case 0x12:
                copyBytes = 3; break;
            case 0x15:
                copyBytes = 4; break;
        }
        
        while (copyBytes--)
        {
            *tmp = *string;
            ++tmp;
            ++string;
        }
    }
}

#define EASYTALK_STARTED   100
#define EASYTALK_CLOSING   200
#define EASYTALK_CHOICE_1  1
#define EASYTALK_CHOICE_2  2
#define EASYTALK_CHOICE_3  3
#define EASYTALK_NO_EVENT  0
int EasyNpcTalk(Actor *actor, PlayState *play, float distance, EasyMessage *msg)
{
	static const char *queued = 0;
	
	// TODO use uLib and z64rom to process queue at a later
	//      point w/o having to reinvoke this function
	if (queued)
	{
		TextOverride(play, queued);
		queued = 0;
	}
	
	// this runs once, upon talking
	if (Actor_ProcessTalkRequest(actor, play))
	{
		if (msg->text)
			queued = msg->text;
		
		// invoke callback on starting to talk
		if (msg->onTextBegin)
			msg->onTextBegin(actor, play);
		
		return EASYTALK_STARTED;
	}
	
	// XXX the callback may be run more than once, need more testing
	u8 state = Message_GetState(&play->msgCtx);
	if (state == TEXT_STATE_CLOSING)
	{
		if (msg->ontextEnd)
			msg->ontextEnd(actor, play);
		
		return EASYTALK_CLOSING;
	}
	else if (state == TEXT_STATE_CHOICE && Message_ShouldAdvance(play))
	{
		u16 id = msg->id;
		
		// select choice
		msg = msg->choices + play->msgCtx.choiceIndex;
		
		// if choice has no id, create one from parent id
		if (msg->id == 0)
			msg->id = (id |= (play->msgCtx.choiceIndex + 1) << 8);
		
		if (msg->text)
		{
			actor->textId = id;
			Message_ContinueTextbox(play, id);
			TextOverride(play, msg->text);
		}
		else
		{
			actor->textId = 0xffff;
			Message_CloseTextbox(play);
		}
		return play->msgCtx.choiceIndex + EASYTALK_CHOICE_1;
	}
	
	if (msg->id == 0)
		msg->id = 0xA000;
	
	// set 'A' talk near NPC and interaction distance
	actor->textId = msg->id;
	func_8002F2CC(actor, play, distance);
	
	return EASYTALK_NO_EVENT;
}

typedef struct {
	Actor actor;
	SkelAnime skelanime;
	Vec3s jointTable[SKEL_CORAL_BLUE_NUMBONES_DT];
    Vec3s morphTable[SKEL_CORAL_BLUE_NUMBONES_DT];
	//Vec3s jointTable[SKEL_CORAL_PURPLE_NUMBONES_DT];
    //Vec3s morphTable[SKEL_CORAL_PURPLE_NUMBONES_DT];
	u8 state, frame;
} Entity;

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(uncullZoneForward, 3000, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneScale, 3000, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneDownward, 700, ICHAIN_STOP),
};

static void init(Entity *en, PlayState *globalCtx) {
    Actor_ProcessInitChain(&en->actor, sInitChain);
	Actor_SetFocus(&en->actor, 10.0f);
	Actor_SetScale(&en->actor, 0.002f);
	if (TYPE == 0) {
	SkelAnime_InitFlex(globalCtx, &en->skelanime, (FlexSkeletonHeader*)SKEL_CORAL_BLUE, (AnimationHeader*)ANIM_ARMATURE_DANCE, en->jointTable, en->morphTable, SKEL_CORAL_BLUE_NUMBONES_DT);
	} else if (TYPE == 1) {
	SkelAnime_InitFlex(globalCtx, &en->skelanime, (FlexSkeletonHeader*)SKEL_CORAL_PURPLE, (AnimationHeader*)ANIM_ARMATUREPURPLE_DANCE, en->jointTable, en->morphTable, SKEL_CORAL_BLUE_NUMBONES_DT);
	}
}

static void destroy(Entity *en, PlayState *globalCtx) {
	
}

#define EasyTextbox EasyMessage
#define MSG_END           "\x02"

static void update(Entity *en, PlayState *globalCtx) {
    EasyNpcTalk(&en->actor, globalCtx, 100, &(EasyMessage){ MSG_COLOR_RED "this text is red" MSG_END });
}

static void draw(Entity *en, PlayState *globalCtx) {
	Gfx_SetupDL_25Opa(globalCtx->state.gfxCtx);
	
	SkelAnime_DrawFlexOpa(globalCtx, en->skelanime.skeleton, en->skelanime.jointTable,
		en->skelanime.dListCount, NULL, NULL, &en->actor);
	//Gfx_DrawDListOpa(globalCtx, (Gfx*)0x06009E48);
	//if ((Message_GetState(&globalCtx->msgCtx) == TEXT_STATE_DONE) && Message_ShouldAdvance(globalCtx)) {
}

ActorInit CoralBuddies = {
    ACT_ID,
    ACTORCAT_NPC,
    FLAGS,
    OBJ_ID,
    sizeof(Entity),
    (ActorFunc)init,
    (ActorFunc)destroy,
    (ActorFunc)update,
    (ActorFunc)draw,
};



