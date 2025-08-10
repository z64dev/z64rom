#include <uLib.h>
#include <code/z_actor.h>
#include <code/z_lights.h>
#include <code/z_room.h>

typedef struct RoomPointLights {
    void*      roomSegment;
    u32        numlights;
    LightNode* light[32];
} RoomPointLights;

RoomPointLights RoomPointLightsA;
RoomPointLights RoomPointLightsB;
s8 prevTransitionTrigger = 0;

static bool NewRoom_UsesPointlight(void* roomSegment) {
    s8* header = Segment_Scene_GetHeader(roomSegment, gSaveContext.sceneLayer);
    
    while (*header != 0x14)
        header += 8;
    
    return header[1];
}

static void NewRoom_BindLight(PlayState* play, Lights* lights, Room* room) {
    if (!NewRoom_UsesPointlight(room->segment)) {
        Lights_BindAll(lights, play->lightCtx.listHead, 0);
        
        return;
    }
    
    LightInfo* sortedList[7] = { NULL };
    s32 lightCount = Lights_SortLightList(play, sortedList);
    
    if (lightCount <= 0)
        return;
    
    for (s32 i = 0; i < lightCount; i++) {
        if (sortedList[i] == NULL)
            break;
        LightInfo* info = sortedList[i];
        LightParams* params = &info->params;
        Light* light;
        
        if (info->type != LIGHT_DIRECTIONAL) {
            Lights_SetPointlight(play, lights, params, false);
        } else if (info->type == LIGHT_DIRECTIONAL) {
            if ((light = Lights_FindSlot(lights)) == NULL)
                return;
            
            light->l.col[0] = light->l.colc[0] = params->dir.color[0];
            light->l.col[1] = light->l.colc[1] = params->dir.color[1];
            light->l.col[2] = light->l.colc[2] = params->dir.color[2];
            light->l.dir[0] = params->dir.x;
            light->l.dir[1] = params->dir.y;
            light->l.dir[2] = params->dir.z;
            light->l.pad1 = 0;
        }
    }
}

static void NewRoom_DestroyExpiredLights(PlayState* play) {
    RoomPointLights* which;
    void* seg;
    int k;
    
    for (k = 0; k < 2; ++k) {
        switch (k) {
            case 0:
                which = &RoomPointLightsA;
                break;
                
            default:
                which = &RoomPointLightsB;
                break;
        }
        if (
            ((seg = which->roomSegment)
            && seg != play->roomCtx.curRoom.segment
            && seg != play->roomCtx.prevRoom.segment)
            || (play->transitionTrigger == 0 && prevTransitionTrigger > 0)
        ) {
            while (which->numlights)
                LightContext_RemoveLight(
                    play
                    ,
                    &play->lightCtx
                    ,
                    which->light[--which->numlights]
                );
            which->roomSegment = 0;
        }
    }
    
    if (!gSaveContext.fw.set)
        D_8015BC10->info->params.point.radius = 0;
    prevTransitionTrigger = play->transitionTrigger;
}

void NewRoom_Draw(PlayState* play, Room* room, u32 flags) {
    Lights* lightList;
    
    if (!room->segment)
        return;
    
    gSegments[3] = VIRTUAL_TO_PHYSICAL(room->segment);
    NewRoom_DestroyExpiredLights(play);
    lightList = LightContext_NewLights(&play->lightCtx, play->state.gfxCtx);
    NewRoom_BindLight(play, lightList, room);
    
    if (lightList->numLights > 0)
        Lights_Draw(lightList, play->state.gfxCtx);
    
    sRoomDrawHandlers[room->roomShape->base.type](play, room, flags);
}

Asm_VanillaHook(func_8009728C);
s32 func_8009728C(PlayState* play, RoomContext* roomCtx, s32 roomNum) {
    u32 size;
    
    if (roomCtx->status == 0) {
        roomCtx->prevRoom = roomCtx->curRoom;
        roomCtx->curRoom.num = roomNum;
        roomCtx->curRoom.segment = NULL;
        roomCtx->status = 1;
        
        osLibPrintf("%d / %d", roomNum, play->numRooms);
        Assert(roomNum < play->numRooms);
        
        size = play->roomList[roomNum].vromEnd - play->roomList[roomNum].vromStart;
        roomCtx->unk_34 = (void*)ALIGN16((u32)roomCtx->bufPtrs[roomCtx->unk_30] - ((size + 8) * roomCtx->unk_30 + 7));
        
        osCreateMesgQueue(&roomCtx->loadQueue, &roomCtx->loadMsg, 1);
        DmaMgr_SendRequest2(
            &roomCtx->dmaRequest, (u32)roomCtx->unk_34, play->roomList[roomNum].vromStart, size, 0,
            &roomCtx->loadQueue, NULL, "../z_room.c", 1036
        );
        roomCtx->unk_30 ^= 1;
        
        return 1;
    }
    
    return 0;
}

Asm_VanillaHook(Room_DrawCullable);
void Room_DrawCullable(PlayState* play, Room* room, u32 flags) {
    RoomShapeCullable* roomShape;
    RoomShapeCullableEntry* roomShapeCullableEntry;
    RoomShapeCullableEntryLinked linkedEntriesBuffer[ROOM_SHAPE_CULLABLE_MAX_ENTRIES];
    RoomShapeCullableEntryLinked* head = NULL;
    RoomShapeCullableEntryLinked* tail = NULL;
    RoomShapeCullableEntryLinked* iter;
    Gfx* displayList;
    RoomShapeCullableEntryLinked* insert;
    s32 j;
    s32 i;
    Vec3f pos;
    Vec3f projectedPos;
    f32 projectedW;
    RoomShapeCullableEntry* roomShapeCullableEntries;
    RoomShapeCullableEntry* roomShapeCullableEntryIter;
    f32 entryBoundsNearZ;

    OPEN_DISPS(play->state.gfxCtx, "../z_room.c", 287);
	
    if (flags & ROOM_DRAW_OPA) {
        func_800342EC(&D_801270A0, play);

        gSPSegment(POLY_OPA_DISP++, 0x03, room->segment);
        func_80093C80(&play->state);
        gSPMatrix(POLY_OPA_DISP++, &gMtxClear, G_MTX_MODELVIEW | G_MTX_LOAD);
    }

    if (flags & ROOM_DRAW_XLU) {
        func_800342EC(&D_801270A0, play);
        gSPSegment(POLY_XLU_DISP++, 0x03, room->segment);
        Gfx_SetupDL_25Xlu(play->state.gfxCtx);
        gSPMatrix(POLY_XLU_DISP++, &gMtxClear, G_MTX_MODELVIEW | G_MTX_LOAD);
    }

    roomShape = &room->roomShape->cullable;
    roomShapeCullableEntry = SEGMENTED_TO_VIRTUAL(roomShape->entries);
    insert = linkedEntriesBuffer;

    roomShapeCullableEntries = roomShapeCullableEntry;
	
    if (play->roomCtx.status < 0) {
		// has no effect in oot. the above is faked, but keeping this for the if else statment. does actually build
        /* (i = 0; i < roomShape->numEntries; i++, roomShapeCullableEntry++) {
            if (R_ROOM_CULL_DEBUG_MODE != 0) {
                if (((R_ROOM_CULL_DEBUG_MODE == ROOM_CULL_DEBUG_MODE_UP_TO_TARGET) &&
                     (i <= R_ROOM_CULL_DEBUG_TARGET)) ||
                    ((R_ROOM_CULL_DEBUG_MODE == ROOM_CULL_DEBUG_MODE_ONLY_TARGET) && (i == R_ROOM_CULL_DEBUG_TARGET))) {
                    if (flags & ROOM_DRAW_OPA) {
                        displayList = roomShapeCullableEntry->opa;
                        if (displayList != NULL) {
                            gSPDisplayList(POLY_OPA_DISP++, displayList);
                        }
                    }

                    if (flags & ROOM_DRAW_XLU) {
                        displayList = roomShapeCullableEntry->xlu;
                        if (displayList != NULL) {
                            gSPDisplayList(POLY_XLU_DISP++, displayList);
                        }
                    }
                }
            } else {
                if (flags & ROOM_DRAW_OPA) {
                    displayList = roomShapeCullableEntry->opa;
                    if (displayList != NULL) {
                        gSPDisplayList(POLY_OPA_DISP++, displayList);
                    }
                }

                if (flags & ROOM_DRAW_XLU) {
                    displayList = roomShapeCullableEntry->xlu;
                    if (displayList != NULL) {
                        gSPDisplayList(POLY_XLU_DISP++, displayList);
                    }
                }
            }
        }*/
    } else {	
	f32 var_fa1 = 1.0f; // play->billboardMtx;
    f32 var_fv1;
    s32 var_a1;

    // Pick and sort entries by depth
    for (i = 0; i < roomShape->numEntries; i++, roomShapeCullableEntry++) {

        // Project the entry position, to get the depth it is at.
        pos.x = roomShapeCullableEntry->boundsSphereCenter.x;
        pos.y = roomShapeCullableEntry->boundsSphereCenter.y;
        pos.z = roomShapeCullableEntry->boundsSphereCenter.z;
        SkinMatrix_Vec3fMtxFMultXYZW(&play->viewProjectionMtxF, &pos, &projectedPos, &projectedW);
		
        projectedPos.z *= var_fa1;

        var_fv1 = ABS_ALT(roomShapeCullableEntry->boundsSphereRadius);

        // If the entry bounding sphere isn't fully before the rendered depth range
        if (-var_fv1 < projectedPos.z) {

            // Compute the depth of the nearest point in the entry's bounding sphere
            entryBoundsNearZ = projectedPos.z - roomShapeCullableEntry->boundsSphereRadius;

            // If the entry bounding sphere isn't fully beyond the rendered depth range
            if (entryBoundsNearZ < play->lightCtx.fogFar) {

                // This entry will be rendered
                insert->entry = roomShapeCullableEntry;
                //insert->boundsNearZ = entryBoundsNearZ;
				
                if (roomShapeCullableEntry->boundsSphereRadius < 0) {
                    insert->boundsNearZ = FLT_MAX;
                } else {
                    insert->boundsNearZ = entryBoundsNearZ;
                }

                // Insert into the linked list, ordered by ascending depth of the nearest point in the bounding sphere
                iter = head;
                if (iter == NULL) {
                    head = tail = insert;
                    insert->prev = insert->next = NULL;
                } else {
                    do {
                        if (insert->boundsNearZ < iter->boundsNearZ) {
                            break;
                        }
                        iter = iter->next;
                    } while (iter != NULL);

                    if (iter == NULL) {
                        insert->prev = tail;
                        insert->next = NULL;
                        tail->next = insert;
                        tail = insert;
                    } else {
                        insert->prev = iter->prev;
                        if (insert->prev == NULL) {
                            head = insert;
                        } else {
                            insert->prev->next = insert;
                        }
                        iter->prev = insert;
                        insert->next = iter;
                    }
                }

                insert++;
            }
        }
    }

    // if this is real then I might not be
    R_ROOM_CULL_NUM_ENTRIES = roomShape->numEntries & 0xFFFF & 0xFFFF & 0xFFFF;
	
    // Draw entries, from nearest to furthest
    i = 1;

        if (flags & ROOM_DRAW_OPA) {
            for (; head != NULL; head = head->next, i++) {
//                s32 pad;

                roomShapeCullableEntry = head->entry;

                if (R_ROOM_CULL_DEBUG_MODE != ROOM_CULL_DEBUG_MODE_OFF) {
                    // Debug mode drawing

                    if (((R_ROOM_CULL_DEBUG_MODE == ROOM_CULL_DEBUG_MODE_UP_TO_TARGET) &&
                         (i <= R_ROOM_CULL_DEBUG_TARGET)) ||
                        ((R_ROOM_CULL_DEBUG_MODE == ROOM_CULL_DEBUG_MODE_ONLY_TARGET) &&
                         (i == R_ROOM_CULL_DEBUG_TARGET))) {

                        displayList = roomShapeCullableEntry->opa;
                        if (displayList != NULL) {
                            gSPDisplayList(POLY_OPA_DISP++, displayList);
                        }
                    }
                } else {
                    displayList = roomShapeCullableEntry->opa;
                    if (displayList != NULL) {
                        gSPDisplayList(POLY_OPA_DISP++, displayList);
                    }
                }
            }
        }

        if (flags & ROOM_DRAW_XLU) {
            for (; tail != NULL; tail = tail->prev) {
                f32 temp_fv0;
                f32 temp_fv1;

                roomShapeCullableEntry = tail->entry;
                displayList = roomShapeCullableEntry->xlu;

                if (displayList != NULL) {
                    if (roomShapeCullableEntry->boundsSphereRadius & 1) {

                        temp_fv0 = tail->boundsNearZ - (f32)(iREG(93) + 0xBB8);
                        temp_fv1 = iREG(94) + 0x7D0;

                        if (temp_fv0 < temp_fv1) {
                            if (temp_fv0 < 0.0f) {
                                var_a1 = 255;
                            } else {
                                var_a1 = 255 - (s32)((temp_fv0 / temp_fv1) * 255.0f);
                            }
                            gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, var_a1);
                            gSPDisplayList(POLY_XLU_DISP++, displayList);
                        }
                    } else {
                        gSPDisplayList(POLY_XLU_DISP++, displayList);
                    }
                }
            }
        }

        R_ROOM_CULL_USED_ENTRIES = i - 1;
    }

    CLOSE_DISPS(play->state.gfxCtx, "../z_room.c", 430);
}