#include <uLib.h>
#include <code/z_scene.h>
#include <assets/objects/object_link_boy/object_link_boy.h>

asm("D_801260C8 = 0x801260C8;");
extern Vec3f D_801260C8;

Asm_VanillaHook(Object_IsLoaded);
s32 Object_IsLoaded(ObjectContext* objectCtx, s32 bankIndex)
{
    return objectCtx->status[bankIndex].id > 0;
}

Asm_VanillaHook(Object_GetIndex);
s32 Object_GetIndex(ObjectContext* objectCtx, s16 objectId)
{
    for (int i = 0; i < objectCtx->num; ++i)
        if (ABS(objectCtx->status[i].id) == objectId)
            return i;
    
    return -1;
}

Asm_VanillaHook(Actor_SetObjectDependency);
void Actor_SetObjectDependency(PlayState* play, Actor* actor)
{
    gSegments[6] = VIRTUAL_TO_PHYSICAL(play->objectCtx.status[actor->objBankIndex].segment);
}

Asm_VanillaHook(Actor_Draw);
void Actor_Draw(PlayState* play, Actor* actor) {
    //FaultClient faultClient;
    Lights* lights;

    //Fault_AddClient(&faultClient, Actor_FaultPrint, actor, "Actor_draw");

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

    //Fault_RemoveClient(&faultClient);
}

Asm_VanillaHook(Player_DrawHookshotReticle);
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

Asm_VanillaHook(Scene_CommandObjectList);
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

Asm_VanillaHook(Scene_CommandSpecialFiles);
void Scene_CommandSpecialFiles(PlayState* play, SceneCmd* cmd) {
    if (cmd->specialFiles.keepObjectId != OBJECT_INVALID) {
        play->objectCtx.subKeepIndex = Object_Spawn(&play->objectCtx, cmd->specialFiles.keepObjectId);
        gSegments[5] = VIRTUAL_TO_PHYSICAL(play->objectCtx.status[play->objectCtx.subKeepIndex].segment);
    }

    if (cmd->specialFiles.cUpElfMsgNum != 0) {
        play->cUpElfMsgs = Play_LoadFile(play, &sNaviMsgFiles[cmd->specialFiles.cUpElfMsgNum - 1]);
    }
}

