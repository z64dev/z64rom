/*
// EasyTalk.c by @z64me, testing credits to @zeldaboy14
//
// easy textbox system for oot
//
// examples & tips:
//
//  - if you're using a custom Play_Update() implementation in your project,
//    be sure to add EasyTalkFlush(play) immediately after Message_Update(play)
//
// - add this line of code to your npc's update() hook for a basic textbox:
//      EasyTalkNpcString(&this->actor, play, 100, "Hello, world!");
//
// - here's an example of asking the player to make a choice and processing it:
        switch (EasyNpcTalk(&en->actor, globalCtx, 100, &(EasyTalk) {
            "So, how do you like your Z-Targeting?\x01\x01\x1B\x05\x42"
            "Hold\x01"
            "Switch\x05\x40\x02",
            .choices = (EasyTalk[]) {
                { "Z-Targeting style is now \x05\x42Hold\x05\x40.\x02" },
                { "Z-Targeting style is now \x05\x42Switch\x05\x40.\x02" },
            },
        })) {
            case EASYTALK_CHOICE_1: gSaveContext.zTargetSetting = 1; break;
            case EASYTALK_CHOICE_2: gSaveContext.zTargetSetting = 0; break;
        }
//
// - choices can also be nested if you want complex dialogue trees
//
// - control codes can either be hex, or use control code macros
//   e.g. "\x05\x01" "this text is red" "\x02"
//   or:  MSG_COLOR_RED "this text is red" MSG_END
//
// - hex control codes:
//   https://wiki.cloudmodding.com/oot/Text_Format#Control_Codes
//
// - control code macros: (see uLib.h)
*/

#include <uLib.h>

static const char *sEasyTalkQueuedString = 0;

void EasyTalkOverrideString(PlayState *play, const char *string)
{
	Font *font = &play->msgCtx.font;
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

void EasyTalkFlush(PlayState *play)
{
	if (sEasyTalkQueuedString)
	{
		EasyTalkOverrideString(play, sEasyTalkQueuedString);
		sEasyTalkQueuedString = 0;
	}
}

int EasyTalkNpc(Actor *actor, PlayState *play, float distance, EasyTalk *msg)
{
	// this runs once, upon talking
	if (Actor_ProcessTalkRequest(actor, play))
	{
		if (msg->text)
			sEasyTalkQueuedString = msg->text;
		
		// invoke callback on starting to talk
		if (msg->onOpen)
			msg->onOpen(actor, play);
		
		return EASYTALK_STARTED;
	}
	
	u8 state = Message_GetState(&play->msgCtx);
	if (state == TEXT_STATE_CLOSING)
	{
		// XXX this callback may be run more than once, need more testing
		if (msg->onClose)
			msg->onClose(actor, play);
		
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
			EasyTalkOverrideString(play, msg->text);
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

int EasyTalkNpcString(Actor *actor, PlayState *play, float distance, const char *msg)
{
	return EasyTalkNpc(actor, play, distance, &(EasyTalk){ msg });
}

