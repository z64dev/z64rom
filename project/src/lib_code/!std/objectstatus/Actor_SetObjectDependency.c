#include <uLib.h>

/*
   z64ram = 0x8002D644
   z64rom = 0xAA47E4
 */

void Actor_SetObjectDependency(PlayState* play, Actor* actor) {
    gSegments[6] = VIRTUAL_TO_PHYSICAL(play->objectCtx.status[actor->objBankIndex].segment);
}

