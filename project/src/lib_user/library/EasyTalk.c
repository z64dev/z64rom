/*
// EasyTalk.c by @z64me, testing credits to @zeldaboy14
//
// easy textbox system for oot
//
// examples & tips:
//
//  - if you're updating an existing project, please open OvlMessage.c and
//    ensure the Message_OpenText() function has EasyTalkFlush(playState);
//    at the end of the function
//
// - add this line of code to your npc's update() hook for a basic textbox:
//      EasyTalkNpc(&this->actor, play, .string = "Hello, world!");
//
// - you can have text be triggered instantly (aka the player does not have
//   to interact with the actor or press A in order to show the textbox) by
//   using EASYTALK_DISTANCE_ACTIVATE_INSTANTLY for distance:
//      EasyTalkNpc(&this->actor, play,
//                  .distance = EASYTALK_DISTANCE_ACTIVATE_INSTANTLY,
//                  .string = "Hello, world!");
//  you would want to manage states in your actor and only use this feature
//  conditionally, otherwise it will be continually reactivated (callbacks
//  are a good way to facilitate state changes for things like this)
//
// - you can also assign any actor you can z-target a navi description like so:
//      EasyTalkSetNaviActorDescriptionString(&this->actor, play, "Hello!");
//
// - here's an example of asking the player to make a choice and processing it:
        switch (EasyTalkNpc(&this->actor, play, .robust = &(EasyTalk) {
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
// - choices can be further nested if you want more complex dialogue trees
//
// - when instantiating an EasyTalk struct, the 'style' field is optional
//   (the default 0 will give a standard npc textbox with dynamic placement)
//   but you can specify styles e.g. MSGBOX_TYPE_WOODEN | MSGBOX_POS_MIDDLE
//
// - control codes can either be hex, or use control code macros
//   e.g. "\x05\x01" "this text is red" "\x02"
//   or:  MSG_COLOR_RED "this text is red" MSG_END
//
// - hex control codes:
//   https://wiki.cloudmodding.com/oot/Text_Format#Control_Codes
//
// - control code macros: (see uLib.h)
//   https://github.com/z64utils/z64rom/blob/2f39822f2acc54e7626878ea6dadc694a24afdb6/project/src/lib_user/uLib.h#L250-L290
//
// - one benefit to using hex control codes is Zelda64-Text-Editor can
//   easily generate them for you; it's a WYSIWYG text editor for oot/mm
//   that does a great job showing you how your text will look in-game,
//   and editing compatible strings is as easy as:
//      copy: right-click -> copy C code
//      paste: right-click -> paste C code
//   download it here: https://github.com/skawo/Zelda64-Text-Editor
//
// motivation:
//
//   the text system in oot is not very friendly to work with
//
//   it involves invoking many functions in many different places,
//   splitting what should be simple logic in one function, into
//   complex logic spread across many functions
//
//   analyzing the oot decomp reveals that many actors go about
//   handling text in very different ways from one another, meaning
//   there is no standardization, and adding text to actors feels
//   like writing a lot of boilerplate again and again, every time
//
//   furthermore, the game's text is stored in a way that is inconvenient
//   to edit: messages must be referenced by id (poor for code readability),
//   and there is a limit on the number available (albeit a high one!)
//
//   easytalk aims to address these problems; using it allows you to
//   keep your actor logic simple and code clean; don't forget to
//   read the examples included above if you haven't already!
*/

#include <uLib.h>

static const char *sEasyTalkQueuedString = 0;
static const char *sEasyTalkQueuedNaviDescriptionString = 0;

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

void EasyTalkQueueOverrideString(const char *text)
{
	sEasyTalkQueuedString = text;
}

void EasyTalkApplyQueuedNaviActorDescription(void)
{
	EasyTalkQueueOverrideString(sEasyTalkQueuedNaviDescriptionString);
}

void EasyTalkSetNaviActorDescriptionString(Actor *actor, PlayState *play, const char *text)
{
	Player *player = GET_PLAYER(play);
	
	// if this actor is being z-targeted
	if (actor == player->targetActor)
	{
		actor->naviEnemyId = NAVI_ENEMY_NONE - 1; // signal
		sEasyTalkQueuedNaviDescriptionString = text;
	}
}

// when invoking this function, do it like so:
// EasyTalkNpc(&this->actor, play, .string = "Hello world");
//  1. notice how one of the arguments passed in was .string = "Hello world"
//  2. that is how different settings are handled; for example, you can also
//     inject an optional detection range, optional style, etc, like so:
//     .string = "Hello world", .distance = 123, .style = MSGBOX_TYPE_WOODEN
//  3. please see the documentation at the top of this file for more examples
int (EasyTalkNpc)(Actor *actor, PlayState *play, const EasyTalkNpcArgs *args)
{
	const EasyTalk *msg = args->robust;
	float distance = args->distance;
	u8 style = args->style;
	EasyTalk msgFromString = { args->string };
	
	// user used string instead of robust
	if (!msg)
		msg = &msgFromString;
	
	// might refactor active -> play->msgCtx->talkActor
	// eventually, if doing so doesn't break anything
	static Actor *active = 0;
	u16 id = 0xfffc;//msg->id;
	
	// overwrite style for message 0xfffc
	*((u8*)0x8014F532) = style;
	
	// only allow active npc to capture these events
	if (active == actor)
	{
		u8 state = Message_GetState(&play->msgCtx);
		
		if (state == TEXT_STATE_CLOSING)
		{
			// ensure this block will only be entered once per close
			active = 0;
			
			if (msg->onClose)
				msg->onClose(actor, play);
			
			return EASYTALK_CLOSED;
		}
		else if (state == TEXT_STATE_CHOICE && Message_ShouldAdvance(play))
		{
			// select choice
			msg = msg->choices + play->msgCtx.choiceIndex;
			
			// if choice has no id, create one from parent id
			//if (msg->id) id = msg->id;
			//else id |= (play->msgCtx.choiceIndex + 1) << 8;
			
			// onClose doubles as onChoose, for option textboxes
			if (msg->onClose)
				msg->onClose(actor, play);
			
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
	}
	else
	{
		// this runs once, upon talking
		if (Actor_ProcessTalkRequest(actor, play)
			// or instantly, if this constant was passed in
			|| distance == EASYTALK_DISTANCE_ACTIVATE_INSTANTLY
		)
		{
			active = actor;
			
			EasyTalkQueueOverrideString(msg->text);
			
			// activated instantly, aka input from player is required
			if (distance == EASYTALK_DISTANCE_ACTIVATE_INSTANTLY)
				Message_StartTextbox(play, id, actor);
			
			// invoke callback on starting to talk
			if (msg->onOpen)
				msg->onOpen(actor, play);
			
			return EASYTALK_OPENED;
		}
	}
	
	// set 'A' talk near NPC and interaction distance
	actor->textId = id;
	func_8002F2CC(actor, play, distance);
	
	return EASYTALK_NO_EVENT;
}

