//
// example scene overlay
//
// these can be placed in scene folders eventually, to
// automatically compile and inject them into scenes
//

#include "global.h"

// function name must be Init()
void Init(PlayState* play)
{
    u32 gameplayFrames;
    u8 spA3;
    u16 spA0;
    Gfx* displayListHead;

    spA3 = 128;
    spA0 = 500;
    displayListHead = Graph_Alloc(play->state.gfxCtx, 6 * sizeof(Gfx));

    if (1) {}
    if (1) {}

    OPEN_DISPS(play->state.gfxCtx, "../z_scene_table.c", 6965);

    gameplayFrames = play->gameplayFrames;
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScroll(play->state.gfxCtx, G_TX_RENDERTILE, 127 - gameplayFrames % 128,
                                (gameplayFrames * 1) % 128, 32, 32, 1, gameplayFrames % 128, (gameplayFrames * 1) % 128,
                                32, 32));
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScroll(play->state.gfxCtx, G_TX_RENDERTILE, 127 - gameplayFrames % 128,
                                (gameplayFrames * 10) % 128, 32, 32, 1, gameplayFrames % 128,
                                (gameplayFrames * 10) % 128, 32, 32));

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 128, 128, 128, 128);

    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 128, 128, 128, 128);

    if (gSaveContext.sceneLayer == 4) {
        spA3 = 255 - (u8)play->roomCtx.unk_74[0];
    } else if (gSaveContext.sceneLayer == 6) {
        spA0 = play->roomCtx.unk_74[0] + 500;
    } else if ((!IS_CUTSCENE_LAYER || LINK_IS_ADULT) && GET_EVENTCHKINF(EVENTCHKINF_07)) {
        spA0 = 2150;
    }

    gSPSegment(POLY_OPA_DISP++, 0x0A, displayListHead);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 128, 128, 128, spA3);
    gSPEndDisplayList(displayListHead++);

    gSPSegment(POLY_XLU_DISP++, 0x0B, displayListHead);
    gSPSegment(POLY_OPA_DISP++, 0x0B, displayListHead);
    gDPPipeSync(displayListHead++);
    gDPSetEnvColor(displayListHead++, 128, 128, 128, spA0 * 0.1f);
    gSPEndDisplayList(displayListHead);

    gSPSegment(POLY_OPA_DISP++, 0x0C,
               Gfx_TwoTexScroll(play->state.gfxCtx, G_TX_RENDERTILE, 0, (s16)(-play->roomCtx.unk_74[0] * 0.02f), 32, 16,
                                1, 0, (s16)(-play->roomCtx.unk_74[0] * 0.02f), 32, 16));

    CLOSE_DISPS(play->state.gfxCtx, "../z_scene_table.c", 7044);
}
#endif
