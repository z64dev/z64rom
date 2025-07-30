/*
   z64ram = 0x80097DD8
   z64rom = 0xB0EF78
 */

#include <uLib.h>

Asm_VanillaHook(Object_InitBank);
void Object_InitBank(PlayState *playState, ObjectContext *objectCtx) {
    u32 spaceSize;
    s32 i;
    
    spaceSize = 0x200000; // 2.0 MB
    
    objectCtx->num = objectCtx->unk_09 = 0;
    objectCtx->mainKeepIndex = objectCtx->subKeepIndex = 0;
    
    // if array was changed to pointer, ensure it points to something!
    //
    // unfortunately, there is no way to check this using C preprocessor,
    // so casting to a pointer-to-pointer is used to achieve polymorphism
    //
    // the compiler will optimize if (false) away
    if (sizeof(playState->objectCtx.status) == sizeof(void*))
    {
        // repurpose unused PAL text table for extended object status limit
        ObjectStatus **status = (ObjectStatus**)&objectCtx->status;
        *status = (void*)0x80153764; // tail of message entry table
        *status -= OBJECT_EXCHANGE_BANK_MAX; // step backwards into PAL entries
    }
    
    for (i = 0; i < OBJECT_EXCHANGE_BANK_MAX; i++) {
        objectCtx->status[i].id = OBJECT_INVALID;
    }
    
    objectCtx->spaceStart = objectCtx->status[0].segment =
        GameState_Alloc(&playState->state, spaceSize, __FILE__, __LINE__);
    objectCtx->spaceEnd = (void*)((s32)objectCtx->spaceStart + spaceSize);
    
    objectCtx->mainKeepIndex = Object_Spawn(objectCtx, OBJECT_GAMEPLAY_KEEP);
    gSegments[4] = VIRTUAL_TO_PHYSICAL(objectCtx->status[objectCtx->mainKeepIndex].segment);
}
