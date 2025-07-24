#include <uLib.h>

/*
   z64ram = 0x80030BE8
   z64rom = 0xAA7D88
 */

void Actor_Draw(PlayState* play, Actor* actor) {
    FaultClient faultClient;
    Lights* lights;

    Fault_AddClient(&faultClient, Actor_FaultPrint, actor, gBlankString/*"Actor_draw"*/);

    OPEN_DISPS(play->state.gfxCtx, gBlankString, 0);

    lights = LightContext_NewLights(&play->lightCtx, play->state.gfxCtx);

    Lights_BindAll(lights, play->lightCtx.listHead, (actor->flags & ACTOR_FLAG_22) ? NULL : &actor->world.pos);
    Lights_Draw(lights, play->state.gfxCtx);

    if (actor->flags & ACTOR_FLAG_12) {
        Matrix_SetTranslateRotateYXZ(actor->world.pos.x + play->mainCamera.skyboxOffset.x,
                                     actor->world.pos.y +
                                         ((actor->shape.yOffset * actor->scale.y) + play->mainCamera.skyboxOffset.y),
                                     actor->world.pos.z + play->mainCamera.skyboxOffset.z, &actor->shape.rot);
    } else {
        Matrix_SetTranslateRotateYXZ(actor->world.pos.x, actor->world.pos.y + (actor->shape.yOffset * actor->scale.y),
                                     actor->world.pos.z, &actor->shape.rot);
    }

    Matrix_Scale(actor->scale.x, actor->scale.y, actor->scale.z, MTXMODE_APPLY);
    Actor_SetObjectDependency(play, actor);

    gSPSegment(POLY_OPA_DISP++, 0x06, play->objectCtx.status[actor->objBankIndex].segment);
    gSPSegment(POLY_XLU_DISP++, 0x06, play->objectCtx.status[actor->objBankIndex].segment);

    if (actor->colorFilterTimer != 0) {
        Color_RGBA8 color = { 0, 0, 0, 255 };

        if (actor->colorFilterParams & 0x8000) {
            color.r = color.g = color.b = ((actor->colorFilterParams & 0x1F00) >> 5) | 7;
        } else if (actor->colorFilterParams & 0x4000) {
            color.r = ((actor->colorFilterParams & 0x1F00) >> 5) | 7;
        } else {
            color.b = ((actor->colorFilterParams & 0x1F00) >> 5) | 7;
        }

        if (actor->colorFilterParams & 0x2000) {
            func_80026860(play, &color, actor->colorFilterTimer, actor->colorFilterParams & 0xFF);
        } else {
            func_80026400(play, &color, actor->colorFilterTimer, actor->colorFilterParams & 0xFF);
        }
    }

    actor->draw(actor, play);

    if (actor->colorFilterTimer != 0) {
        if (actor->colorFilterParams & 0x2000) {
            func_80026A6C(play);
        } else {
            func_80026608(play);
        }
    }

    if (actor->shape.shadowDraw != NULL) {
        actor->shape.shadowDraw(actor, lights, play);
    }

    CLOSE_DISPS(play->state.gfxCtx, gBlankString, 0);

    Fault_RemoveClient(&faultClient);
}

