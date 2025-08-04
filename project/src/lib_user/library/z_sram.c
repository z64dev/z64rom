#include "global.h"
#include "vt.h"

#include <uLib.h>
#include <z64hdr/include/code/z_sram.h>

static s16 sDungeonEntrancesHook[] = {
    ENTR_YDAN_0,      ENTR_DDAN_0,      ENTR_BDAN_0,         ENTR_BMORI1_0,           ENTR_HIDAN_0, ENTR_MIZUSIN_0,
    ENTR_JYASINZOU_0, ENTR_HAKADAN_0,   ENTR_HAKADANCH_0,    ENTR_ICE_DOUKUTO_0,      ENTR_GANON_0, ENTR_MEN_0,
    ENTR_GERUDOWAY_0, ENTR_GANONTIKA_0, ENTR_GANON_SONOGO_0, ENTR_GANONTIKA_SONOGO_0,
};

/**
 *  Copy save currently on the buffer to Save Context and complete various tasks to open the save.
 *  This includes:
 *  - Set proper entrance depending on where the game was saved
 *  - If health is less than 3 hearts, give 3 hearts
 *  - If either scarecrow song is set, copy them from save context to the proper location
 *  - Handle a case where the player saved and quit after zelda cutscene but didnt get the song
 *  - Give and equip master sword if player is adult and doesnt have kokiri sword (bug?)
 *  - Revert any trade items that spoil
 */
Asm_VanillaHook(Sram_OpenSave);
void Sram_OpenSave(SramContext* sramCtx) {
    u16 i;
    u16 j;
    u8* ptr;

    osSyncPrintf("個人Ｆｉｌｅ作成\n"); // "Create personal file"
    i = gSramSlotOffsets[gSaveContext.fileNum];
    osSyncPrintf("ぽいんと＝%x(%d)\n", i, gSaveContext.fileNum); // "Point="

    MemCpy(&gSaveContext, sramCtx->readBuff + i, sizeof(Save));

    osSyncPrintf(VT_FGCOL(YELLOW));
    osSyncPrintf("SCENE_DATA_ID = %d   SceneNo = %d\n", gSaveContext.savedSceneId,
                 ((void)0, gSaveContext.entranceIndex));

    switch (gSaveContext.savedSceneId) {
        case SCENE_YDAN:
        case SCENE_DDAN:
        case SCENE_BDAN:
        case SCENE_BMORI1:
        case SCENE_HIDAN:
        case SCENE_MIZUSIN:
        case SCENE_JYASINZOU:
        case SCENE_HAKADAN:
        case SCENE_HAKADANCH:
        case SCENE_ICE_DOUKUTO:
        case SCENE_GANON:
        case SCENE_MEN:
        case SCENE_GERUDOWAY:
        case SCENE_GANONTIKA:
            gSaveContext.entranceIndex = sDungeonEntrancesHook[gSaveContext.savedSceneId];
            break;

        case SCENE_YDAN_BOSS:
            gSaveContext.entranceIndex = ENTR_YDAN_0;
            break;

        case SCENE_DDAN_BOSS:
            gSaveContext.entranceIndex = ENTR_DDAN_0;
            break;

        case SCENE_BDAN_BOSS:
            gSaveContext.entranceIndex = ENTR_BDAN_0;
            break;

        case SCENE_MORIBOSSROOM:
            gSaveContext.entranceIndex = ENTR_BMORI1_0;
            break;

        case SCENE_FIRE_BS:
            gSaveContext.entranceIndex = ENTR_HIDAN_0;
            break;

        case SCENE_MIZUSIN_BS:
            gSaveContext.entranceIndex = ENTR_MIZUSIN_0;
            break;

        case SCENE_JYASINBOSS:
            gSaveContext.entranceIndex = ENTR_JYASINZOU_0;
            break;

        case SCENE_HAKADAN_BS:
            gSaveContext.entranceIndex = ENTR_HAKADAN_0;
            break;

        case SCENE_GANON_SONOGO:
        case SCENE_GANONTIKA_SONOGO:
        case SCENE_GANON_BOSS:
        case SCENE_GANON_FINAL:
        case SCENE_GANON_DEMO:
            gSaveContext.entranceIndex = ENTR_GANON_0;
            break;

        default:
            {
			#if SAVE_ANYWHERE == false
				if (gSaveContext.savedSceneId != SCENE_LINK_HOME) {
					gSaveContext.entranceIndex = (LINK_AGE_IN_YEARS == YEARS_CHILD) ? ENTR_LINK_HOME_0 : ENTR_TOKINOMA_7;
				} else {
					gSaveContext.entranceIndex = ENTR_LINK_HOME_0;
				}
            #endif
            break;
			}

			
    }

    osSyncPrintf("scene_no = %d\n", gSaveContext.entranceIndex);
    osSyncPrintf(VT_RST);

    if (gSaveContext.health < 0x30) {
        gSaveContext.health = 0x30;
    }

    if (gSaveContext.scarecrowLongSongSet) {
        osSyncPrintf(VT_FGCOL(BLUE));
        osSyncPrintf("\n====================================================================\n");

        MemCpy(gScarecrowLongSongPtr, gSaveContext.scarecrowLongSong, sizeof(gSaveContext.scarecrowLongSong));

        ptr = (u8*)gScarecrowLongSongPtr;
        for (i = 0; i < ARRAY_COUNT(gSaveContext.scarecrowLongSong); i++, ptr++) {
            osSyncPrintf("%d, ", *ptr);
        }

        osSyncPrintf("\n====================================================================\n");
        osSyncPrintf(VT_RST);
    }

    if (gSaveContext.scarecrowSpawnSongSet) {
        osSyncPrintf(VT_FGCOL(GREEN));
        osSyncPrintf("\n====================================================================\n");

        MemCpy(gScarecrowSpawnSongPtr, gSaveContext.scarecrowSpawnSong, sizeof(gSaveContext.scarecrowSpawnSong));

        ptr = gScarecrowSpawnSongPtr;
        for (i = 0; i < ARRAY_COUNT(gSaveContext.scarecrowSpawnSong); i++, ptr++) {
            osSyncPrintf("%d, ", *ptr);
        }

        osSyncPrintf("\n====================================================================\n");
        osSyncPrintf(VT_RST);
    }

    // if zelda cutscene has been watched but lullaby was not obtained, restore cutscene and take away letter
    if (GET_EVENTCHKINF(EVENTCHKINF_40) && !CHECK_QUEST_ITEM(QUEST_SONG_LULLABY)) {
        i = gSaveContext.eventChkInf[EVENTCHKINF_40_INDEX] & ~EVENTCHKINF_40_MASK;
        gSaveContext.eventChkInf[EVENTCHKINF_40_INDEX] = i;

        INV_CONTENT(ITEM_LETTER_ZELDA) = ITEM_CHICKEN;

        for (j = 1; j < 4; j++) {
            if (gSaveContext.equips.buttonItems[j] == ITEM_LETTER_ZELDA) {
                gSaveContext.equips.buttonItems[j] = ITEM_CHICKEN;
            }
        }
    }

    if (LINK_AGE_IN_YEARS == YEARS_ADULT && !CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_MASTER)) {
        gSaveContext.inventory.equipment |= OWNED_EQUIP_FLAG(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_MASTER);
        gSaveContext.equips.buttonItems[0] = ITEM_SWORD_MASTER;
        gSaveContext.equips.equipment &= ~(0xF << (EQUIP_TYPE_SWORD * 4));
        gSaveContext.equips.equipment |= EQUIP_VALUE_SWORD_MASTER << (EQUIP_TYPE_SWORD * 4);
    }

    for (i = 0; i < ARRAY_COUNT(gSpoilingItems); i++) {
        if (INV_CONTENT(ITEM_TRADE_ADULT) == gSpoilingItems[i]) {
            INV_CONTENT(gSpoilingItemReverts[i]) = gSpoilingItemReverts[i];

            for (j = 1; j < 4; j++) {
                if (gSaveContext.equips.buttonItems[j] == gSpoilingItems[i]) {
                    gSaveContext.equips.buttonItems[j] = gSpoilingItemReverts[i];
                }
            }
        }
    }

    gSaveContext.magicLevel = 0;
}


/**
 *  Write the contents of the Save Context to a main and backup slot in SRAM.
 *  Note: The whole Save Context is written even though only the `save` substruct is read back later
 */
Asm_VanillaHook(Sram_WriteSave);
void Sram_WriteSave(SramContext* sramCtx) {
    u16 offset;
    u16 checksum;
    u16 j;
    u16* ptr;

    gSaveContext.checksum = 0;

    ptr = (u16*)&gSaveContext;
    checksum = 0;
    j = 0;
	
	#if SAVE_ANYWHERE == true
	if (gSaveContext.savedSceneId == SCENE_KAKUSIANA){
	gSaveContext.entranceIndex = gSaveContext.respawn[RESPAWN_MODE_RETURN].entranceIndex;             
	}
	#endif

    for (offset = 0; offset < CHECKSUM_SIZE; offset++) {
        if (++j == 0x20) {
            j = 0;
        }
        checksum += *ptr++;
    }

    gSaveContext.checksum = checksum;

    ptr = (u16*)&gSaveContext;
    checksum = 0;

    for (offset = 0; offset < CHECKSUM_SIZE; offset++) {
        if (++j == 0x20) {
            j = 0;
        }
        checksum += *ptr++;
    }

    offset = gSramSlotOffsets[gSaveContext.fileNum];
    SsSram_ReadWrite(OS_K1_TO_PHYSICAL(0xA8000000) + offset, &gSaveContext, SLOT_SIZE, OS_WRITE);

    ptr = (u16*)&gSaveContext;
    checksum = 0;

    for (offset = 0; offset < CHECKSUM_SIZE; offset++) {
        if (++j == 0x20) {
            j = 0;
        }
        checksum += *ptr++;
    }

    offset = gSramSlotOffsets[gSaveContext.fileNum + 3];
    SsSram_ReadWrite(OS_K1_TO_PHYSICAL(0xA8000000) + offset, &gSaveContext, SLOT_SIZE, OS_WRITE);
}
