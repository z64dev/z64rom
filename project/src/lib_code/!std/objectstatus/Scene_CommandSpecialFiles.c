#include <uLib.h>
#include <code/z_scene.h>

/*
   z64ram = 0x8009883C
   z64rom = 0xB0F9DC
 */

void Scene_CommandSpecialFiles(PlayState* play, SceneCmd* cmd) {
    if (cmd->specialFiles.keepObjectId != OBJECT_INVALID) {
        play->objectCtx.subKeepIndex = Object_Spawn(&play->objectCtx, cmd->specialFiles.keepObjectId);
        gSegments[5] = VIRTUAL_TO_PHYSICAL(play->objectCtx.status[play->objectCtx.subKeepIndex].segment);
    }

    if (cmd->specialFiles.cUpElfMsgNum != 0) {
        play->cUpElfMsgs = Play_LoadFile(play, &sNaviMsgFiles[cmd->specialFiles.cUpElfMsgNum - 1]);
    }
}

