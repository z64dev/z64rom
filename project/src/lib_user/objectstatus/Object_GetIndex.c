#include <uLib.h>

/*
   z64ram = 0x8009812C
   z64rom = 0xB0F2CC
 */

Asm_VanillaHook(Object_GetIndex);
s32 Object_GetIndex(ObjectContext* objectCtx, s16 objectId) {
    s32 i;

    for (i = 0; i < objectCtx->num; i++) {
        if (ABS(objectCtx->status[i].id) == objectId) {
            return i;
        }
    }

    return -1;
}

