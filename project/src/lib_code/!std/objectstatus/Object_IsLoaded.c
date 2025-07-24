#include <uLib.h>

/*
   z64ram = 0x80098188
   z64rom = 0xB0F328
 */

s32 Object_IsLoaded(ObjectContext* objectCtx, s32 bankIndex) {
    if (objectCtx->status[bankIndex].id > 0) {
        return true;
    } else {
        return false;
    }
}

