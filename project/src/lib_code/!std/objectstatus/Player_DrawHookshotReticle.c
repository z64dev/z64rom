#include <uLib.h>
#include <assets/objects/object_link_boy/object_link_boy.h>

/*
   z64ram = 0x80090AFC
   z64rom = 0xB07C9C
 */

asm("D_801260C8 = 0x801260C8;");
extern Vec3f D_801260C8;

void Player_DrawHookshotReticle(PlayState* play, Player* this, f32 arg2) {
    //static Vec3f D_801260C8 = { -500.0f, -100.0f, 0.0f };
    CollisionPoly* sp9C;
    s32 bgId;
    Vec3f sp8C;
    Vec3f sp80;
    Vec3f sp74;
    Vec3f sp68;
    f32 sp64;
    f32 sp60;

    D_801260C8.z = 0.0f;
    Matrix_MultVec3f(&D_801260C8, &sp8C);
    D_801260C8.z = arg2;
    Matrix_MultVec3f(&D_801260C8, &sp80);

    if (BgCheck_AnyLineTest3(&play->colCtx, &sp8C, &sp80, &sp74, &sp9C, 1, 1, 1, 1, &bgId)) {
        OPEN_DISPS(play->state.gfxCtx, gBlankString, 0);

        OVERLAY_DISP = Gfx_SetupDL(OVERLAY_DISP, SETUPDL_7);

        SkinMatrix_Vec3fMtxFMultXYZW(&play->viewProjectionMtxF, &sp74, &sp68, &sp64);

        sp60 = (sp64 < 200.0f) ? 0.08f : (sp64 / 200.0f) * 0.08f;

        Matrix_Translate(sp74.x, sp74.y, sp74.z, MTXMODE_NEW);
        Matrix_Scale(sp60, sp60, sp60, MTXMODE_APPLY);

        gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx, gBlankString, 0),
                  G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPSegment(OVERLAY_DISP++, 0x06, play->objectCtx.status[this->actor.objBankIndex].segment);
        gSPDisplayList(OVERLAY_DISP++, gLinkAdultHookshotReticleDL);

        CLOSE_DISPS(play->state.gfxCtx, gBlankString, 0);
    }
}

