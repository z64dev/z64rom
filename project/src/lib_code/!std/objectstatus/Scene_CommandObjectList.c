#include <uLib.h>

/*
   z64ram = 0x8009899C
   z64rom = 0xB0FB3C
 */

void Scene_CommandObjectList(PlayState* play, SceneCmd* cmd) {
    s32 i;
    s32 j;
    s32 k;
    ObjectStatus* status;
    ObjectStatus* status2;
    ObjectStatus* firstStatus;
    s16* objectEntry = SEGMENTED_TO_VIRTUAL(cmd->objectList.data);
    void* nextPtr;

    k = 0;
    i = play->objectCtx.unk_09;
    firstStatus = &play->objectCtx.status[0];
    status = &play->objectCtx.status[i];

    while (i < play->objectCtx.num) {
        if (status->id != *objectEntry) {
            status2 = &play->objectCtx.status[i];
            for (j = i; j < play->objectCtx.num; j++) {
                status2->id = OBJECT_INVALID;
                status2++;
            }
            play->objectCtx.num = i;
            func_80031A28(play, &play->actorCtx);

            continue;
        }

        i++;
        k++;
        objectEntry++;
        status++;
    }

    //ASSERT(cmd->objectList.length <= OBJECT_EXCHANGE_BANK_MAX,
    //       "scene_info->object_bank.num <= OBJECT_EXCHANGE_BANK_MAX", gBlankString, 0);

    while (k < cmd->objectList.length) {
        nextPtr = func_800982FC(&play->objectCtx, i, *objectEntry);
        if (i < OBJECT_EXCHANGE_BANK_MAX - 1) {
            firstStatus[i + 1].segment = nextPtr;
        }
        i++;
        k++;
        objectEntry++;
    }

    play->objectCtx.num = i;
}

