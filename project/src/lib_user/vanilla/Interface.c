#include <uLib.h>
#include <code/z_parameter.h>
#include <overlays/actors/ovl_En_Horse_Game_Check/z_en_horse_game_check.h>
#include <code/z_map_exp.h>
#define dropShadowAlphaRupee 1.0f

asm ("sSetupDL_80125A60 = 0x80125A60;");
asm ("D_80125A5C = 0x80125A5C;");
asm ("sHBAScoreDigits = 0x80125A5C");

typedef enum {
    /* 0x0 */ TIMER_STATE_OFF,
    /* 0x1 */ TIMER_STATE_ENV_HAZARD_INIT, // Init env timer that counts down, total time based on health, resets on void-out, kills at 0
    /* 0x2 */ TIMER_STATE_ENV_HAZARD_PREVIEW, // Display initial time, keep it fixed at the screen center
    /* 0x3 */ TIMER_STATE_ENV_HAZARD_MOVE, // Move to top-left corner
    /* 0x4 */ TIMER_STATE_ENV_HAZARD_TICK, // Counting down
    /* 0x5 */ TIMER_STATE_DOWN_INIT, // Init timer that counts down
    /* 0x6 */ TIMER_STATE_DOWN_PREVIEW, // Display initial time, keep it fixed at the screen center
    /* 0x7 */ TIMER_STATE_DOWN_MOVE, // Move to top-left corner
    /* 0x8 */ TIMER_STATE_DOWN_TICK, // Counting down
    /* 0xA */ TIMER_STATE_STOP = 10,
    /* 0xB */ TIMER_STATE_UP_INIT, // Init timer that counts up
    /* 0xC */ TIMER_STATE_UP_PREVIEW, // Display initial time, keep it fixed at the screen center
    /* 0xD */ TIMER_STATE_UP_MOVE, // Move to top-left corner
    /* 0xE */ TIMER_STATE_UP_TICK, // Counting up
    /* 0xF */ TIMER_STATE_UP_FREEZE  // Stop counting the timer
} TimerState;

typedef enum {
    /* 0x0 */ SUBTIMER_STATE_OFF,
    /* 0x1 */ SUBTIMER_STATE_DOWN_INIT, // Init timer that counts down
    /* 0x2 */ SUBTIMER_STATE_DOWN_PREVIEW, // Display initial time, keep it fixed at the screen center
    /* 0x3 */ SUBTIMER_STATE_DOWN_MOVE, // Move to top-left corner
    /* 0x4 */ SUBTIMER_STATE_DOWN_TICK, // Counting down
    /* 0x5 */ SUBTIMER_STATE_RESPAWN, // Time is up, trigger a transition, reset button items, spoil trade quest items
    /* 0x6 */ SUBTIMER_STATE_STOP, // Time is up, stop counting
    /* 0x7 */ SUBTIMER_STATE_UP_INIT, // Init timer that counts up
    /* 0x8 */ SUBTIMER_STATE_UP_PREVIEW, // Display initial time, keep it fixed at the screen center
    /* 0x9 */ SUBTIMER_STATE_UP_MOVE, // Move to top-left corner
    /* 0xA */ SUBTIMER_STATE_UP_TICK // Counting up
} SubTimerState;

s16 sMagicBorderR = 255;
s16 sMagicBorderG = 255;
s16 sMagicBorderB = 255;

typedef void* TexturePtr;

/**
 * Draw an IA8 texture on a rectangle with a shadow slightly offset to the bottom-right
 *
 * @param gfx the display list pointer
 * @param texture
 * @param textureWidth texture image width in texels
 * @param textureHeight texture image height in texels
 * @param rectLeft the x-coordinate of upper-left corner of rectangle
 * @param rectTop the y-coordinate of upper-left corner of rectangle
 * @param rectWidth rectangle width in texels
 * @param rectHeight rectangle height in texels
 * @param dsdx the change in s for each change in x (s5.10)
 * @param dtdy the change in t for each change in y (s5.10)
 * @param r texture red
 * @param g texture green
 * @param b texture blue
 * @param a texture alpha
 * @return Gfx* the display list pointer
 */
Gfx* Gfx_DrawTexRectIA8_DropShadow(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, s16 rectLeft,
                                   s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy, s16 r, s16 g, s16 b,
                                   s16 a) {
    s16 dropShadowAlpha = a;

    if (a > 100) {
        dropShadowAlpha = 100;
    }

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, dropShadowAlpha);

    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(gfx++, (rectLeft + 2) * 4, (rectTop + 2) * 4, (rectLeft + rectWidth + 2) * 4,
                        (rectTop + rectHeight + 2) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, r, g, b, a);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return gfx;
}

/**
 * Draw a colored rectangle with a shadow slightly offset to the bottom-right
 *
 * @param gfx the display list pointer
 * @param rectLeft the x-coordinate of upper-left corner of rectangle
 * @param rectTop the y-coordinate of upper-left corner of rectangle
 * @param rectWidth rectangle width in texels
 * @param rectHeight rectangle height in texels
 * @param dsdx the change in s for each change in x (s5.10)
 * @param dtdy the change in t for each change in y (s5.10)
 * @param r // rectangle red
 * @param g // rectangle green
 * @param b // rectangle blue
 * @param a // rectangle alpha
 * @return Gfx* the display list pointer
 */
Gfx* Gfx_DrawRect_DropShadow(Gfx* gfx, s16 rectLeft, s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy,
                             s16 r, s16 g, s16 b, s16 a) {
    s16 dropShadowAlpha = a;

    if (a > 100) {
        dropShadowAlpha = 100;
    }

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, dropShadowAlpha);
    gSPTextureRectangle(gfx++, (rectLeft + 2) * 4, (rectTop + 2) * 4, (rectLeft + rectWidth + 2) * 4,
                        (rectTop + rectHeight + 2) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, r, g, b, a);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return gfx;
}

/**
 * Draw an IA8 texture on a rectangle with a shadow slightly offset to the bottom-right with additional texture offsets
 *
 * @param gfx the display list pointer
 * @param texture
 * @param textureWidth texture image width in texels
 * @param textureHeight texture image height in texels
 * @param rectLeft the x-coordinate of upper-left corner of rectangle
 * @param rectTop the y-coordinate of upper-left corner of rectangle
 * @param rectWidth rectangle width in texels
 * @param rectHeight rectangle height in texels
 * @param dsdx the change in s for each change in x (s5.10)
 * @param dtdy the change in t for each change in y (s5.10)
 * @param r // texture red
 * @param g // texture green
 * @param b // texture blue
 * @param a // texture alpha
 * @param masks specify the mask for the s axis
 * @param rects the texture coordinate s of upper-left corner of rectangle (s10.5)
 * @return Gfx* the display list pointer
 */
Gfx* Gfx_DrawTexRectIA8_DropShadowOffset(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight,
                                         s16 rectLeft, s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy,
                                         s16 r, s16 g, s16 b, s16 a, s32 masks, s32 rects) {
    s16 dropShadowAlpha = a;

    if (a > 100) {
        dropShadowAlpha = 100;
    }

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, dropShadowAlpha);

    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_MIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, masks, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(gfx++, (rectLeft + 2) * 4, (rectTop + 2) * 4, (rectLeft + rectWidth + 2) * 4,
                        (rectTop + rectHeight + 2) * 4, G_TX_RENDERTILE, rects, 0, dsdx, dtdy);

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, r, g, b, a);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, rects, 0, dsdx, dtdy);

    return gfx;
}

/**
 * Draw an IA8 texture on a rectangle
 *
 * @param gfx the display list pointer
 * @param texture
 * @param textureWidth texture image width in texels
 * @param textureHeight texture image height in texels
 * @param rectLeft the x-coordinate of upper-left corner of rectangle
 * @param rectTop the y-coordinate of upper-left corner of rectangle
 * @param rectWidth rectangle width in texels
 * @param rectHeight rectangle height in texels
 * @param dsdx the change in s for each change in x (s5.10)
 * @param dtdy the change in t for each change in y (s5.10)
 * @return Gfx*  the display list pointer
 */
Gfx* Gfx_DrawTexRectIA8(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                        s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(gfx++, rectLeft << 2, rectTop << 2, (rectLeft + rectWidth) << 2, (rectTop + rectHeight) << 2,
                        G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return gfx;
}

/**
 * Draw an I8 texture on a rectangle
 *
 * @param gfx the display list pointer
 * @param texture
 * @param textureWidth texture image width in texels
 * @param textureHeight texture image height in texels
 * @param rectLeft the x-coordinate of upper-left corner of rectangle
 * @param rectTop the y-coordinate of upper-left corner of rectangle
 * @param rectWidth rectangle width in texels
 * @param rectHeight rectangle height in texels
 * @param dsdx the change in s for each change in x (s5.10)
 * @param dtdy the change in t for each change in y (s5.10)
 * @return Gfx* the display list pointer
 */
Gfx* Gfx_DrawTexRectI8(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                       s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_I, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(gfx++, rectLeft << 2, rectTop << 2, (rectLeft + rectWidth) << 2, (rectTop + rectHeight) << 2,
                        G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return gfx;
}

Asm_VanillaHook(Interface_Draw);
void Interface_Draw(PlayState* play) {
    static s16 magicArrowEffectsR[] = { 255, 100, 255 };
    static s16 magicArrowEffectsG[] = { 0, 100, 255 };
    static s16 magicArrowEffectsB[] = { 0, 255, 100 };
	static s16 timerDigitLeftPos[] = {
        16, 25, 34, 42, 51, 60, 68, 77,
    };
    static s16 digitWidth[] = {
        9, 9, 8, 9, 9, 8, 9, 9,
    };
    static s16 rupeeDigitsFirst[] = { 1, 0, 0 };
    static s16 rupeeDigitsCount[] = { 2, 3, 3 };
    static s16 spoilingItemEntrances[] = { ENTR_SPOT10_2, ENTR_SPOT07_3, ENTR_SPOT07_3 };
    static s16 D_8015FFE0;
    static s16 D_8015FFE2;
    static s16 D_8015FFE4;
    static s16 D_8015FFE6;
    
    #define sTimerNextSecondTimer     D_8015FFE0
    #define sTimerStateTimer          D_8015FFE2
    #define sSubTimerNextSecondTimer  D_8015FFE4
    #define sSubTimerStateTimer       D_8015FFE6
    
    #define TIMER_DIGITS 5
    #if Patch_MM_TIMER == true
        #undef TIMER_DIGITS
        #define TIMER_DIGITS 8
    #endif
    static s16 timerDigits[TIMER_DIGITS];
	extern u16 sCUpInvisible;
	extern u16 sCUpTimer;
	asm ("sCUpInvisible = 0x80125a10 ;");
	asm ("sCUpTimer = 0x80125a14 ;");
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    PauseContext* pauseCtx = &play->pauseCtx;
    MessageContext* msgCtx = &play->msgCtx;
    Player* player = GET_PLAYER(play);
    s16 svar1;
    s16 svar2;
    s16 svar3;
    s16 svar5;
    s16 svar6;
	s16 magicAlpha;
    
    OPEN_DISPS(play->state.gfxCtx, "../z_parameter.c", 3405);
    
    gSPSegment(OVERLAY_DISP++, 0x02, interfaceCtx->parameterSegment);
    gSPSegment(OVERLAY_DISP++, 0x07, interfaceCtx->doActionSegment);
    gSPSegment(OVERLAY_DISP++, 0x08, interfaceCtx->iconItemSegment);
    gSPSegment(OVERLAY_DISP++, 0x0B, interfaceCtx->mapSegment);
    
    int whichWallet = CUR_UPG_VALUE(UPG_WALLET);
    static Color_RGB8 sRupeeCounterIconPrimColors[] = {
        { 200, 255, 100 },
        { 170, 170, 255 },
        { 255, 105, 105 },
    };
    static Color_RGB8 sRupeeCounterIconEnvColors[] = {
        { 0, 80, 0 },
        { 10, 10, 80 },
        { 40, 10, 0 },
    };
    Color_RGB8 walletPrim = sRupeeCounterIconPrimColors[whichWallet];
    Color_RGB8 walletEnv = sRupeeCounterIconEnvColors[whichWallet];
    
#if DEV_BUILD
    
    if (gLibCtx.cinematic && play->pauseCtx.state == 0) {
        Letterbox_SetSizeTarget(0x20);
        
        return;
    }
    
#endif

    if (pauseCtx->debugState == 0) {
        Interface_InitVertices(play);
        func_8008A994(interfaceCtx);
        Health_DrawMeter(play);
		
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
        gDPSetEnvColor(OVERLAY_DISP++, 255, 255, 255, 255);
        
        Gfx_SetupDL_39Overlay(play->state.gfxCtx);
        
        // Rupee Icon
		#if Patch_MM_INTERFACE_RUPEE_UPGRADES == true
		// Commented out code below is legacy code by z64me. Using real MM code now
        //gDPSetPrimColor(OVERLAY_DISP++, 0, 0, walletPrim.r, walletPrim.g, walletPrim.b, interfaceCtx->magicAlpha);
        //gDPSetEnvColor(OVERLAY_DISP++, walletEnv.r, walletEnv.g, walletEnv.b, 255);
        // Draw Rupee Icon
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sRupeeCounterIconPrimColors[CUR_UPG_VALUE(UPG_WALLET)].r,
                        sRupeeCounterIconPrimColors[CUR_UPG_VALUE(UPG_WALLET)].g,
                        sRupeeCounterIconPrimColors[CUR_UPG_VALUE(UPG_WALLET)].b, interfaceCtx->magicAlpha);
        gDPSetEnvColor(OVERLAY_DISP++, sRupeeCounterIconEnvColors[CUR_UPG_VALUE(UPG_WALLET)].r,
                       sRupeeCounterIconEnvColors[CUR_UPG_VALUE(UPG_WALLET)].g,
                       sRupeeCounterIconEnvColors[CUR_UPG_VALUE(4)].b, 255);
		#elif Patch_MM_INTERFACE_RUPEE_UPGRADES == false
		gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 255, 100, interfaceCtx->magicAlpha);
        gDPSetEnvColor(OVERLAY_DISP++, 0, 80, 0, 255);
		#endif
       // OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, gRupeeCounterIconTex, 16, 16, 26, 206, 16, 16, 1 << 10, 1 << 10);
	           OVERLAY_DISP =
            Gfx_DrawTexRectIA8(OVERLAY_DISP, gRupeeCounterIconTex, 16, 16, 26, 206, 16, 16, 1 << 10, 1 << 10);
        
        switch (play->sceneId) {
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
            case SCENE_GANON_SONOGO:
            case SCENE_GANONTIKA_SONOGO:
            case SCENE_TAKARAYA:
                if (gSaveContext.inventory.dungeonKeys[gSaveContext.mapIndex] >= 0) {
                    // Small Key Icon
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 230, 255, interfaceCtx->magicAlpha);
                    gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 20, 255);
                    OVERLAY_DISP = Gfx_TextureIA8(
                        OVERLAY_DISP, gSmallKeyCounterIconTex, 16, 16, 26, 190, 16, 16,
                        1 << 10, 1 << 10
                    );
                    
                    // Small Key Counter
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
                    gDPSetCombineLERP(
                        OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE,
                        TEXEL0, 0, PRIMITIVE, 0
                    );
                    
                    interfaceCtx->counterDigits[2] = 0;
                    interfaceCtx->counterDigits[3] = gSaveContext.inventory.dungeonKeys[gSaveContext.mapIndex];
                    
                    while (interfaceCtx->counterDigits[3] >= 10) {
                        interfaceCtx->counterDigits[2]++;
                        interfaceCtx->counterDigits[3] -= 10;
                    }
                    
                    svar3 = 42;
                    
                    if (interfaceCtx->counterDigits[2] != 0) {
                        OVERLAY_DISP = Gfx_TextureI8(
                            OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * interfaceCtx->counterDigits[2])), 8, 16,
                            svar3, 190, 8, 16, 1 << 10, 1 << 10
                        );
                        svar3 += 8;
                    }
                    
                    OVERLAY_DISP = Gfx_TextureI8(
                        OVERLAY_DISP,
                        ((u8*)gCounterDigit0Tex + (8 * 16 * interfaceCtx->counterDigits[3])),
                        8, 16, svar3, 190, 8, 16, 1 << 10, 1 << 10
                    );
                }
                break;
            default:
                break;
        }
	// Legacy code specifically from z64me. I left this as I do like this code, and wanted to preserve it instead of outright deleting it
	// It was the old draw code specifically for the rupee counter shadows. I've ported over the MM code for simplicity sake, now that I understand MM's code more at this point.
	// - Zeldaboy14
	/*for (int wow = 0; wow <= 1; ++wow)
	{			
        // Rupee Counter
		#if Patch_MM_INTERFACE_SHADOWS == true
        gDPPipeSync(OVERLAY_DISP++);
            gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);
		#endif
        if (gSaveContext.rupees == CUR_CAPACITY(UPG_WALLET)) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 120, 255, 0, interfaceCtx->magicAlpha);
        } else if (gSaveContext.rupees != 0) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
        } else {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 100, 100, 100, interfaceCtx->magicAlpha);
        }
		
		#if Patch_MM_INTERFACE_SHADOWS == true
		if (wow == 0)
		    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, dropShadowAlphaRupee * interfaceCtx->magicAlpha);	
        
        gDPSetCombineLERP(
            OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
            PRIMITIVE, 0
        );
		#elif Patch_MM_INTERFACE_SHADOWS == false	
	    gDPSetCombineLERP(
            OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
            PRIMITIVE, 0
        );
		#endif
        
        interfaceCtx->counterDigits[0] = interfaceCtx->counterDigits[1] = 0;
        interfaceCtx->counterDigits[2] = gSaveContext.rupees;
        
        if ((interfaceCtx->counterDigits[2] > 9999) || (interfaceCtx->counterDigits[2] < 0)) {
            interfaceCtx->counterDigits[2] &= 0xDDD;
        }
        
        while (interfaceCtx->counterDigits[2] >= 100) {
            interfaceCtx->counterDigits[0]++;
            interfaceCtx->counterDigits[2] -= 100;
        }
        
        while (interfaceCtx->counterDigits[2] >= 10) {
            interfaceCtx->counterDigits[1]++;
            interfaceCtx->counterDigits[2] -= 10;
        }
        
        svar2 = rupeeDigitsFirst[CUR_UPG_VALUE(UPG_WALLET)];
        svar5 = rupeeDigitsCount[CUR_UPG_VALUE(UPG_WALLET)];
        
        for (svar1 = 0, svar3 = 42; svar1 < svar5; svar1++, svar2++, svar3 += 8) {
		#if Patch_MM_INTERFACE_SHADOWS == true	
            OVERLAY_DISP =
                Gfx_TextureI8(
                OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * interfaceCtx->counterDigits[svar2])), 8,
                16, svar3 + (!wow) * 1, 206 + (!wow) * 1, 8, 16, 1 << 10, 1 << 10
                );
		#elif Patch_MM_INTERFACE_SHADOWS == false			
            OVERLAY_DISP =
                Gfx_TextureI8(
                OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * interfaceCtx->counterDigits[svar2])), 8,
                16, svar3, 206, 8, 16, 1 << 10, 1 << 10
                );
		#endif
		}
	}*/
	
        // Rupee Counter
        gDPPipeSync(OVERLAY_DISP++);
		#if Patch_MM_INTERFACE_SHADOWS == true
		
        gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                          PRIMITIVE, 0);

        interfaceCtx->counterDigits[0] = interfaceCtx->counterDigits[1] = 0;
        interfaceCtx->counterDigits[2] = gSaveContext.rupees;
		
		#elif Patch_MM_INTERFACE_SHADOWS == false
		
        if (gSaveContext.rupees == CUR_CAPACITY(UPG_WALLET)) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 120, 255, 0, interfaceCtx->magicAlpha);
        } else if (gSaveContext.rupees != 0) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
        } else {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 100, 100, 100, interfaceCtx->magicAlpha);
        }

        gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                          PRIMITIVE, 0);
						  
        interfaceCtx->counterDigits[0] = interfaceCtx->counterDigits[1] = 0;
        interfaceCtx->counterDigits[2] = gSaveContext.rupees;
		
		#endif

        if ((interfaceCtx->counterDigits[2] > 9999) || (interfaceCtx->counterDigits[2] < 0)) {
            interfaceCtx->counterDigits[2] &= 0xDDD;
        }

        while (interfaceCtx->counterDigits[2] >= 100) {
            interfaceCtx->counterDigits[0]++;
            interfaceCtx->counterDigits[2] -= 100;
        }

        while (interfaceCtx->counterDigits[2] >= 10) {
            interfaceCtx->counterDigits[1]++;
            interfaceCtx->counterDigits[2] -= 10;
        }

        svar2 = rupeeDigitsFirst[CUR_UPG_VALUE(UPG_WALLET)];
        svar5 = rupeeDigitsCount[CUR_UPG_VALUE(UPG_WALLET)];

		#if Patch_MM_INTERFACE_SHADOWS == true
        magicAlpha = interfaceCtx->magicAlpha;
        if (magicAlpha > 180) {
            magicAlpha = 180;
        }
		#endif

        for (svar1 = 0, svar3 = 42; svar1 < svar5; svar1++, svar2++, svar3 += 8) {
			#if Patch_MM_INTERFACE_SHADOWS == true
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, magicAlpha);
			
            OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * interfaceCtx->counterDigits[svar2]), 8,
                                             16, svar3 + 1, 207, 8, 16, 1 << 10, 1 << 10);
							  
            gDPPipeSync(OVERLAY_DISP++);

			if (gSaveContext.rupees == CUR_CAPACITY(UPG_WALLET)) {
				gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 120, 255, 0, interfaceCtx->magicAlpha);
			} else if (gSaveContext.rupees != 0) {
				gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
			} else {
				gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 100, 100, 100, interfaceCtx->magicAlpha);
			}
			
            gSPTextureRectangle(OVERLAY_DISP++, svar3 * 4 + 1, 824, (svar3 * 4 + 1) + 0x20, 888, G_TX_RENDERTILE, 0, 0, 1 << 10,
                                1 << 10);
								
			#elif Patch_MM_INTERFACE_SHADOWS == false
								
            OVERLAY_DISP =
                Gfx_TextureI8(OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * interfaceCtx->counterDigits[svar2])), 8,
                              16, svar3, 206, 8, 16, 1 << 10, 1 << 10);
							  
			#endif

        }
	
        Magic_DrawMeter(play);
        Minimap_Draw(play);
        
        if ((R_PAUSE_MENU_MODE != 2) && (R_PAUSE_MENU_MODE != 3)) {
            func_8002C124(&play->actorCtx.targetCtx, play); // Draw Z-Target
        }
		
		// this GFX setup is still required. Helps with drawing the interface in the overworld when not z-targeting (and when minimap is on)
		Gfx_SetupDL_39Overlay(play->state.gfxCtx);

Gfx* Gfx_DrawTexQuadIA8(Gfx* gfx, void* texture, s16 textureWidth, s16 textureHeight, u16 point) {
    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSP1Quadrangle(gfx++, point, point + 1, point + 2, point + 0, 0);

    return gfx;
}
        
		
Asm_VanillaHook(Interface_DrawItemButtons);
void Interface_DrawItemButtons(PlayState* play) {
    static void* cUpLabelTextures[] = { gNaviCUpJPTex, gNaviCUpJPTex, gNaviCUpJPTex };
    static s16 startButtonLeftPos[] = { 132, 130, 130 };
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    Player* player = GET_PLAYER(play);
    PauseContext* pauseCtx = &play->pauseCtx;
    s16 temp; // Used as both an alpha value and a button index
    s16 texCoordScale;
    s16 width;
    s16 height;
	//s16 aAlpha;
	
	s16 D_801BF9D4[] = {
		0xA7,  // EQUIP_SLOT_B
		0xE3,  // EQUIP_SLOT_C_LEFT
		0xF9,  // EQUIP_SLOT_C_DOWN
		0x10F, // EQUIP_SLOT_C_RIGHT
	};
	
	s16 D_801BF9DC[] = {
		0x11, // EQUIP_SLOT_B
		0x12, // EQUIP_SLOT_C_LEFT
		0x22, // EQUIP_SLOT_C_DOWN
		0x12, // EQUIP_SLOT_C_RIGHT
	};
	
	s16 D_801BF9E4[] = {
		0x23F, // EQUIP_SLOT_B
		0x26C, // EQUIP_SLOT_C_LEFT
		0x26C, // EQUIP_SLOT_C_DOWN
		0x26C, // EQUIP_SLOT_C_RIGHT
	};
	
    static s16 D_801BFAF4[] = {
        0x1D, // EQUIP_SLOT_B
        0x1B, // EQUIP_SLOT_C_LEFT
        0x1B, // EQUIP_SLOT_C_DOWN
        0x1B, // EQUIP_SLOT_C_RIGHT
    };
	
	typedef enum {
		/* -1 */ EQUIP_SLOT_NONE = -1,
		/*  0 */ EQUIP_SLOT_B,
		/*  1 */ EQUIP_SLOT_C_LEFT,
		/*  2 */ EQUIP_SLOT_C_DOWN,
		/*  3 */ EQUIP_SLOT_C_RIGHT,
		/*  4 */ EQUIP_SLOT_A
	} EquipSlot;
	
	#define R_C_BTN_COLOR_MM0  255
    #define R_C_BTN_COLOR_MM1  240
    #define R_C_BTN_COLOR_MM2  0
	
    OPEN_DISPS(play->state.gfxCtx, "../z_parameter.c", 2900);
	
    // B Button Color & Texture
    // Also loads the Item Button Texture reused by other buttons afterwards
    gDPPipeSync(OVERLAY_DISP++);
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_B_BTN_COLOR(0), R_B_BTN_COLOR(1), R_B_BTN_COLOR(2), 
						interfaceCtx->bAlpha);
    gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
	#if Patch_MM_INTERFACE_BUTTONS_CORDS == true
		#if Patch_MM_INTERFACE_SHADOWS == true
	    OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(
        OVERLAY_DISP, gButtonBackgroundTex, 0x20, 0x20, D_801BF9D4[EQUIP_SLOT_B], D_801BF9DC[EQUIP_SLOT_B],
        D_801BFAF4[EQUIP_SLOT_B], D_801BFAF4[EQUIP_SLOT_B], D_801BF9E4[EQUIP_SLOT_B] * 2, D_801BF9E4[EQUIP_SLOT_B] * 2,
        R_B_BTN_COLOR(0), R_B_BTN_COLOR(1), R_B_BTN_COLOR(2), interfaceCtx->bAlpha);
		#elif Patch_MM_INTERFACE_SHADOWS == false
    OVERLAY_DISP =
        Gfx_TextureIA8(OVERLAY_DISP, gButtonBackgroundTex, 32, 32, D_801BF9D4[EQUIP_SLOT_B], D_801BF9DC[EQUIP_SLOT_B],
                       D_801BFAF4[EQUIP_SLOT_B], D_801BFAF4[EQUIP_SLOT_B], D_801BF9E4[EQUIP_SLOT_B] * 2, D_801BF9E4[EQUIP_SLOT_B] * 2);
		#endif
	#elif Patch_MM_INTERFACE_BUTTONS_CORDS == false
		#if Patch_MM_INTERFACE_SHADOWS == true
	    OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(
        OVERLAY_DISP, gButtonBackgroundTex, 0x20, 0x20, R_ITEM_BTN_X(0), R_ITEM_BTN_Y(0),
        R_ITEM_BTN_WIDTH(0), R_ITEM_BTN_WIDTH(0), R_ITEM_BTN_DD(0) << 1, R_ITEM_BTN_DD(0) << 1,
        R_B_BTN_COLOR(0), R_B_BTN_COLOR(1), R_B_BTN_COLOR(2), interfaceCtx->bAlpha);
		#elif Patch_MM_INTERFACE_SHADOWS == false
	OVERLAY_DISP =
    Gfx_TextureIA8(OVERLAY_DISP, gButtonBackgroundTex, 32, 32, R_ITEM_BTN_X(0), R_ITEM_BTN_Y(0),
					   R_ITEM_BTN_WIDTH(0), R_ITEM_BTN_WIDTH(0), R_ITEM_BTN_DD(0) << 1, R_ITEM_BTN_DD(0) << 1);
		#endif
	#endif

    // C-Left Button Color & Texture
    gDPPipeSync(OVERLAY_DISP++);
	#if Patch_INTERFACE_C_BUTTON_COLORS_MM == true
	gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR_MM0, R_C_BTN_COLOR_MM1, R_C_BTN_COLOR_MM2,
					interfaceCtx->cLeftAlpha);	
	#elif Patch_INTERFACE_C_BUTTON_COLORS_MM == false
	gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR(0), R_C_BTN_COLOR(1), R_C_BTN_COLOR(2),
                    interfaceCtx->cLeftAlpha);
	#endif
	
	#if Patch_MM_INTERFACE_SHADOWS == true
    OVERLAY_DISP = Gfx_DrawRect_DropShadow(OVERLAY_DISP, D_801BF9D4[EQUIP_SLOT_C_LEFT], D_801BF9DC[EQUIP_SLOT_C_LEFT],
                                           D_801BFAF4[EQUIP_SLOT_C_LEFT], D_801BFAF4[EQUIP_SLOT_C_LEFT],
                                           D_801BF9E4[EQUIP_SLOT_C_LEFT] * 2, D_801BF9E4[EQUIP_SLOT_C_LEFT] * 2, 255,
                                           240, 0, interfaceCtx->cLeftAlpha);	
	#elif Patch_MM_INTERFACE_SHADOWS == false
    gSPTextureRectangle(OVERLAY_DISP++, R_ITEM_BTN_X(1) << 2, R_ITEM_BTN_Y(1) << 2,
                        (R_ITEM_BTN_X(1) + R_ITEM_BTN_WIDTH(1)) << 2, (R_ITEM_BTN_Y(1) + R_ITEM_BTN_WIDTH(1)) << 2,
                        G_TX_RENDERTILE, 0, 0, R_ITEM_BTN_DD(1) << 1, R_ITEM_BTN_DD(1) << 1);
	#endif

    // C-Down Button Color & Texture
	gDPPipeSync(OVERLAY_DISP++);
	#if Patch_INTERFACE_C_BUTTON_COLORS_MM == true
	gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR_MM0, R_C_BTN_COLOR_MM1, R_C_BTN_COLOR_MM2,
					interfaceCtx->cDownAlpha);
	#elif Patch_INTERFACE_C_BUTTON_COLORS_MM == false
	gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR(0), R_C_BTN_COLOR(1), R_C_BTN_COLOR(2),
                    interfaceCtx->cDownAlpha);
	#endif
	
	#if Patch_MM_INTERFACE_SHADOWS == true	
    OVERLAY_DISP = Gfx_DrawRect_DropShadow(OVERLAY_DISP, D_801BF9D4[EQUIP_SLOT_C_DOWN], D_801BF9DC[EQUIP_SLOT_C_DOWN],
                                           D_801BFAF4[EQUIP_SLOT_C_DOWN], D_801BFAF4[EQUIP_SLOT_C_DOWN],
                                           D_801BF9E4[EQUIP_SLOT_C_DOWN] * 2, D_801BF9E4[EQUIP_SLOT_C_DOWN] * 2, 255,
                                           240, 0, interfaceCtx->cDownAlpha);
	#elif Patch_MM_INTERFACE_SHADOWS == false									   
    gSPTextureRectangle(OVERLAY_DISP++, R_ITEM_BTN_X(2) << 2, R_ITEM_BTN_Y(2) << 2,
                        (R_ITEM_BTN_X(2) + R_ITEM_BTN_WIDTH(2)) << 2, (R_ITEM_BTN_Y(2) + R_ITEM_BTN_WIDTH(2)) << 2,
                        G_TX_RENDERTILE, 0, 0, R_ITEM_BTN_DD(2) << 1, R_ITEM_BTN_DD(2) << 1);
	#endif
	
    // C-Right Button Color & Texture
	#if Patch_INTERFACE_C_BUTTON_COLORS_MM == true
	gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR_MM0, R_C_BTN_COLOR_MM1, R_C_BTN_COLOR_MM2,
					interfaceCtx->cRightAlpha);
	#elif Patch_INTERFACE_C_BUTTON_COLORS_MM == false
	gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR(0), R_C_BTN_COLOR(1), R_C_BTN_COLOR(2),
                    interfaceCtx->cRightAlpha);
	#endif

	#if Patch_MM_INTERFACE_SHADOWS == true	
    OVERLAY_DISP = Gfx_DrawRect_DropShadow(OVERLAY_DISP, D_801BF9D4[EQUIP_SLOT_C_RIGHT], D_801BF9DC[EQUIP_SLOT_C_RIGHT],
                                           D_801BFAF4[EQUIP_SLOT_C_RIGHT], D_801BFAF4[EQUIP_SLOT_C_RIGHT],
                                           D_801BF9E4[EQUIP_SLOT_C_RIGHT] * 2, D_801BF9E4[EQUIP_SLOT_C_RIGHT] * 2, 255,
                                           240, 0, interfaceCtx->cRightAlpha);
	#elif Patch_MM_INTERFACE_SHADOWS == false										   
    gSPTextureRectangle(OVERLAY_DISP++, R_ITEM_BTN_X(3) << 2, R_ITEM_BTN_Y(3) << 2,
                        (R_ITEM_BTN_X(3) + R_ITEM_BTN_WIDTH(3)) << 2, (R_ITEM_BTN_Y(3) + R_ITEM_BTN_WIDTH(3)) << 2,
                        G_TX_RENDERTILE, 0, 0, R_ITEM_BTN_DD(3) << 1, R_ITEM_BTN_DD(3) << 1);
	#endif

    if ((pauseCtx->state < 8) || (pauseCtx->state >= 18)) {
        if ((play->pauseCtx.state != 0) || (play->pauseCtx.debugState != 0)) {
			
			
            // Start Button Texture, Color & Label
            gDPPipeSync(OVERLAY_DISP++);
			#if Patch_INTERFACE_BUTTON_COLORS == OOT
			#define R_PAUSE_BTN_COLOR1 200
			#define R_PAUSE_BTN_COLOR2 0
			#define R_PAUSE_BTN_COLOR3 0
			#elif Patch_INTERFACE_BUTTON_COLORS == MM
			#define R_PAUSE_BTN_COLOR1 255
			#define R_PAUSE_BTN_COLOR2 130
			#define R_PAUSE_BTN_COLOR3 60
			#elif Patch_INTERFACE_BUTTON_COLORS == GC
			#define R_PAUSE_BTN_COLOR1 120
			#define R_PAUSE_BTN_COLOR2 120
			#define R_PAUSE_BTN_COLOR3 120
			#endif

			#if Patch_MM_INTERFACE_BUTTONS_CORDS == true
				#if Patch_MM_INTERFACE_SHADOWS == true
				OVERLAY_DISP = Gfx_DrawRect_DropShadow(OVERLAY_DISP, 0x88, 0x11, 0x16, 0x16, 0x5B6, 0x5B6, R_PAUSE_BTN_COLOR1, R_PAUSE_BTN_COLOR2, R_PAUSE_BTN_COLOR3,
													   interfaceCtx->startAlpha);
				#elif Patch_MM_INTERFACE_SHADOWS == false
					#if Patch_INTERFACE_BUTTON_COLORS == OOT
					gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 0, 0, interfaceCtx->startAlpha);
					#elif Patch_INTERFACE_BUTTON_COLORS == MM
					gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 130, 60, interfaceCtx->startAlpha);
					#elif Patch_INTERFACE_BUTTON_COLORS == GC
					gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 120, 120, 120, interfaceCtx->startAlpha);
					#endif
				gSPTextureRectangle(OVERLAY_DISP++, startButtonLeftPos[gSaveContext.language] << 2, 17 << 2,
									(startButtonLeftPos[gSaveContext.language] + 22) << 2, 39 << 2, G_TX_RENDERTILE, 0, 0,
									(s32)(1.4277344 * (1 << 10)), (s32)(1.4277344 * (1 << 10)));
				#endif
			#elif Patch_MM_INTERFACE_BUTTONS_CORDS == false
				#if Patch_MM_INTERFACE_SHADOWS == true
				OVERLAY_DISP = Gfx_DrawRect_DropShadow(OVERLAY_DISP, 0x84, 0x11, 0x16, 0x16, 0x5B6, 0x5B6, R_PAUSE_BTN_COLOR1, R_PAUSE_BTN_COLOR2, R_PAUSE_BTN_COLOR3,
													   interfaceCtx->startAlpha);
				#elif Patch_MM_INTERFACE_SHADOWS == false
					#if Patch_INTERFACE_BUTTON_COLORS == OOT
					gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 0, 0, interfaceCtx->startAlpha);
					#elif Patch_INTERFACE_BUTTON_COLORS == MM
					gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 130, 60, interfaceCtx->startAlpha);
					#elif Patch_INTERFACE_BUTTON_COLORS == GC
					gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 120, 120, 120, interfaceCtx->startAlpha);
					#endif
				gSPTextureRectangle(OVERLAY_DISP++, startButtonLeftPos[gSaveContext.language] << 2, 17 << 2,
									(startButtonLeftPos[gSaveContext.language] + 22) << 2, 39 << 2, G_TX_RENDERTILE, 0, 0,
									(s32)(1.4277344 * (1 << 10)), (s32)(1.4277344 * (1 << 10)));
				#endif
			#endif
			
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->startAlpha);
            gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);
            gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

            gDPLoadTextureBlock_4b(OVERLAY_DISP++, interfaceCtx->doActionSegment + DO_ACTION_TEX_SIZE * 2, G_IM_FMT_IA,
                                   DO_ACTION_TEX_WIDTH, DO_ACTION_TEX_HEIGHT, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

            texCoordScale = (1 << 10) / (R_START_LABEL_DD(gSaveContext.language) / 100.0f);
            width = DO_ACTION_TEX_WIDTH / (R_START_LABEL_DD(gSaveContext.language) / 100.0f);
            height = DO_ACTION_TEX_HEIGHT / (R_START_LABEL_DD(gSaveContext.language) / 100.0f);
			#if Patch_MM_INTERFACE_BUTTONS_CORDS == true
            gSPTextureRectangle(OVERLAY_DISP++, 0x01F8, 0x0054, 0x02D4, 0x009C, G_TX_RENDERTILE, 0, 0, 0x04A6, 0x04A6);
			#elif Patch_MM_INTERFACE_BUTTONS_CORDS == false
            gSPTextureRectangle(OVERLAY_DISP++, R_START_LABEL_X(gSaveContext.language) << 2,
                                R_START_LABEL_Y(gSaveContext.language) << 2,
                                (R_START_LABEL_X(gSaveContext.language) + width) << 2,
                                (R_START_LABEL_Y(gSaveContext.language) + height) << 2, G_TX_RENDERTILE, 0, 0,
                                texCoordScale, texCoordScale);
			#endif
        }
    }

    if (interfaceCtx->naviCalling && (play->pauseCtx.state == 0) && (play->pauseCtx.debugState == 0) &&
        (play->csCtx.state == CS_STATE_IDLE)) {
        if (!sCUpInvisible) {
            // C-Up Button Texture, Color & Label (Navi Text)
            gDPPipeSync(OVERLAY_DISP++);

            if ((gSaveContext.unk_13EA == 1) || (gSaveContext.unk_13EA == 2) || (gSaveContext.unk_13EA == 5)) {
                temp = 0;
            } else if ((player->stateFlags1 & PLAYER_STATE1_21) || (func_8008F2F8(play) == 4) ||
                       (player->stateFlags2 & PLAYER_STATE2_18)) {
                temp = 70;
            } else {
                temp = interfaceCtx->healthAlpha;
            }
			
			#if Patch_MM_INTERFACE_SHADOWS == true
            OVERLAY_DISP =
            Gfx_DrawRect_DropShadow(OVERLAY_DISP, 0xFE, 0x10, 0x10, 0x10, 0x800, 0x800, 0xFF, 0xF0, 0, temp);
			#endif

			gDPPipeSync(OVERLAY_DISP++);
			#if Patch_INTERFACE_C_BUTTON_COLORS_MM == true
			gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR_MM0, R_C_BTN_COLOR_MM1, R_C_BTN_COLOR_MM2, temp);
			#elif Patch_INTERFACE_C_BUTTON_COLORS_MM == false
			gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR(0), R_C_BTN_COLOR(1), R_C_BTN_COLOR(2), temp);
			#endif
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
			#if Patch_INTERFACE_C_UP_TATL == true
            gSPTextureRectangle(OVERLAY_DISP++, R_C_UP_BTN_X << 2, R_C_UP_BTN_Y << 2, (R_C_UP_BTN_X + 16) << 2,
                                (R_C_UP_BTN_Y + 16) << 2, G_TX_RENDERTILE, 0, 0, 2 << 10, 2 << 10);
			#elif Patch_INTERFACE_C_UP_TATL == false
            gSPTextureRectangle(OVERLAY_DISP++, 2 + R_C_UP_BTN_X << 2, 2 + R_C_UP_BTN_Y << 2, 2 + (R_C_UP_BTN_X + 16) << 2,
                               2 + (R_C_UP_BTN_Y + 16) << 2, G_TX_RENDERTILE, 0, 0, 2 << 10, 2 << 10);
			#endif
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, temp);
            gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);
            gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
							  
			#if Patch_INTERFACE_C_UP_TATL == true
            gDPLoadTextureBlock_4b(OVERLAY_DISP++, cUpLabelTextures[gSaveContext.language], G_IM_FMT_IA, 32, 12,
                                   0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                   G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(OVERLAY_DISP++, 0x03DC, 0x0048, 0x045C, 0x0078, G_TX_RENDERTILE, 0, 0, 1 << 10,
                                1 << 10);
			#elif Patch_INTERFACE_C_UP_TATL == false
            gDPLoadTextureBlock_4b(OVERLAY_DISP++, cUpLabelTextures[gSaveContext.language], G_IM_FMT_IA, 32, 8, 0,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                   G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(OVERLAY_DISP++, R_C_UP_ICON_X << 2, R_C_UP_ICON_Y << 2, (R_C_UP_ICON_X + 32) << 2,
                                (R_C_UP_ICON_Y + 8) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
			#endif
        }

        sCUpTimer--;
        if (sCUpTimer == 0) {
            sCUpInvisible ^= 1;
            sCUpTimer = 10;
        }
    }

    gDPPipeSync(OVERLAY_DISP++);

    // Empty C Button Arrows
    for (temp = 1; temp < 4; temp++) {
        if (gSaveContext.equips.buttonItems[temp] > 0xF0) {
            if (temp == 1) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR(0), R_C_BTN_COLOR(1), R_C_BTN_COLOR(2),
                                interfaceCtx->cLeftAlpha);
            } else if (temp == 2) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR(0), R_C_BTN_COLOR(1), R_C_BTN_COLOR(2),
                                interfaceCtx->cDownAlpha);
            } else {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_C_BTN_COLOR(0), R_C_BTN_COLOR(1), R_C_BTN_COLOR(2),
                                interfaceCtx->cRightAlpha);
            }

            OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, ((u8*)gButtonBackgroundTex + ((32 * 32) * (temp + 1))), 32, 32,
                                          R_ITEM_BTN_X(temp), R_ITEM_BTN_Y(temp), R_ITEM_BTN_WIDTH(temp),
                                          R_ITEM_BTN_WIDTH(temp), R_ITEM_BTN_DD(temp) << 1, R_ITEM_BTN_DD(temp) << 1);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx, "../z_parameter.c", 3071);
}

void Interface_SetNaviCall(PlayState* play, u16 naviCallState) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    if (((naviCallState == 0x1D) || (naviCallState == 0x1E)) && !interfaceCtx->naviCalling &&
        (play->csCtx.state == CS_STATE_IDLE)) {
        // clang-format off
        if (naviCallState == 0x1E) { Audio_PlaySfxGeneral(NA_SE_VO_NAVY_CALL, &gSfxDefaultPos, 4,
                                                            &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                                            &gSfxDefaultReverb);
        }
        // clang-format on

        if (naviCallState == 0x1D) {
            func_800F4524(&gSfxDefaultPos, NA_SE_VO_NA_HELLO_2, 32);
        }

        interfaceCtx->naviCalling = false;
        sCUpInvisible = 0;
        sCUpTimer = 10;
    } else if ((naviCallState == 0x1F) && interfaceCtx->naviCalling) {
        interfaceCtx->naviCalling = false;
    }
}

void Interface_DrawItemIconTexture_B_Btn(PlayState* play, void* texture, s16 button) {
    OPEN_DISPS(play->state.gfxCtx, "../z_parameter.c", 3079);

    gDPLoadTextureBlock(OVERLAY_DISP++, texture, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
	#if Patch_MM_INTERFACE_BUTTONS_CORDS == true
    gSPTextureRectangle(OVERLAY_DISP++, 7 + R_ITEM_ICON_X(button) << 2, R_ITEM_ICON_Y(button) << 2,
                        7 + (R_ITEM_ICON_X(button) + R_ITEM_ICON_WIDTH(button)) << 2,
                        (R_ITEM_ICON_Y(button) + R_ITEM_ICON_WIDTH(button)) << 2, G_TX_RENDERTILE, 0, 0,
                        R_ITEM_ICON_DD(button) << 1, R_ITEM_ICON_DD(button) << 1);
	#elif Patch_MM_INTERFACE_BUTTONS_CORDS == false
    gSPTextureRectangle(OVERLAY_DISP++, R_ITEM_ICON_X(button) << 2, R_ITEM_ICON_Y(button) << 2,
                        (R_ITEM_ICON_X(button) + R_ITEM_ICON_WIDTH(button)) << 2,
                        (R_ITEM_ICON_Y(button) + R_ITEM_ICON_WIDTH(button)) << 2, G_TX_RENDERTILE, 0, 0,
                        R_ITEM_ICON_DD(button) << 1, R_ITEM_ICON_DD(button) << 1);
	#endif
	
    CLOSE_DISPS(play->state.gfxCtx, "../z_parameter.c", 3094);
}
     
void Interface_DrawAmmoCount(PlayState* play, s16 button, s16 alpha) {
    s16 i;
    s16 ammo;
    s16 rectX;
    s16 rectY;
    s16 aAlpha;

    OPEN_DISPS(play->state.gfxCtx, "../z_parameter.c", 3105);

    i = gSaveContext.equips.buttonItems[button];

    if ((i == ITEM_STICK) || (i == ITEM_NUT) || (i == ITEM_BOMB) || (i == ITEM_BOW) ||
        ((i >= ITEM_BOW_ARROW_FIRE) && (i <= ITEM_BOW_ARROW_LIGHT)) || (i == ITEM_SLINGSHOT) || (i == ITEM_BOMBCHU) ||
        (i == ITEM_BEAN)) {

        if ((i >= ITEM_BOW_ARROW_FIRE) && (i <= ITEM_BOW_ARROW_LIGHT)) {
            i = ITEM_BOW;
        }

        ammo = AMMO(i);

        gDPPipeSync(OVERLAY_DISP++);

        if ((button == 0) && (gSaveContext.minigameState == 1)) {
            ammo = play->interfaceCtx.hbaAmmo;
        } else if ((button == 0) && (play->shootingGalleryStatus > 1)) {
            ammo = play->shootingGalleryStatus - 1;
        } else if ((button == 0) && (play->sceneId == SCENE_BOWLING) && Flags_GetSwitch(play, 0x38)) {
            ammo = play->bombchuBowlingStatus;
            if (ammo < 0) {
                ammo = 0;
            }
        } else if (((i == ITEM_BOW) && (AMMO(i) == CUR_CAPACITY(UPG_QUIVER))) ||
                   ((i == ITEM_BOMB) && (AMMO(i) == CUR_CAPACITY(UPG_BOMB_BAG))) ||
                   ((i == ITEM_SLINGSHOT) && (AMMO(i) == CUR_CAPACITY(UPG_BULLET_BAG))) ||
                   ((i == ITEM_STICK) && (AMMO(i) == CUR_CAPACITY(UPG_STICKS))) ||
                   ((i == ITEM_NUT) && (AMMO(i) == CUR_CAPACITY(UPG_NUTS))) || ((i == ITEM_BOMBCHU) && (ammo == 50)) ||
                   ((i == ITEM_BEAN) && (ammo == 15))) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 120, 255, 0, alpha);
        }

        if (ammo == 0) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 100, 100, 100, alpha);
        }

        for (i = 0; ammo >= 10; i++) {
            ammo -= 10;
        }

    #if Patch_MM_INTERFACE_BUTTONS_CORDS == true
        if (i != 0) {
            OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, ((u8*)gAmmoDigit0Tex + ((8 * 8) * i)), 8, 8,
                                          R_ITEM_AMMO_X(button) + (button == 0 ? 2 : 0), R_ITEM_AMMO_Y(button) + (button == 0 ? 0.5 : 0) , 8, 8, 1 << 10, 1 << 10);
        }

        OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, ((u8*)gAmmoDigit0Tex + ((8 * 8) * ammo)), 8, 8,
                                      R_ITEM_AMMO_X(button) + 6 + (button == 0 ? 2 : 0), R_ITEM_AMMO_Y(button) + (button == 0 ? 0.5 : 0), 8, 8, 1 << 10, 1 << 10);
    #else
        if (i != 0) {
            OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, ((u8*)gAmmoDigit0Tex + ((8 * 8) * i)), 8, 8,
                                          R_ITEM_AMMO_X(button) + (button == 0 ? 2 : 0), R_ITEM_AMMO_Y(button) + (button == 0 ? 0.5 : 0) , 8, 8, 1 << 10, 1 << 10);
        }

        OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, ((u8*)gAmmoDigit0Tex + ((8 * 8) * ammo)), 8, 8,
                                      R_ITEM_AMMO_X(button) + 6 + (button == 0 ? 2 : 0), R_ITEM_AMMO_Y(button) + (button == 0 ? 0.5 : 0), 8, 8, 1 << 10, 1 << 10);
    #endif

    }

    CLOSE_DISPS(play->state.gfxCtx, "../z_parameter.c", 3158);
}	 

	Interface_DrawItemButtons(play);
		s16 aAlpha;

		aAlpha = interfaceCtx->aAlpha;

		if (aAlpha > 100) {
			aAlpha = 100;
		}
		
		#define R_A_BTN_Y_OFFSET                    XREG(33)
		#define Interface_SetPerspectiveView        func_8008A8B8
		
		#if Patch_MM_INTERFACE_BUTTONS_CORDS == true
		#define B_ACT_LABEL_X 480
		#define B_ACT_LABEL_Y 64
		#elif Patch_MM_INTERFACE_BUTTONS_CORDS == false
		#define B_ACT_LABEL_X 450
		#define B_ACT_LABEL_Y 70
		#endif

        gDPPipeSync(OVERLAY_DISP++);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
        //Fixs green outline on ammo counter and B Button Do Action label, commonly seen in inaccurate emulators with Majora's Mask
        gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
        if (!(interfaceCtx->unk_1FA)) {
            // B Button Icon & Ammo Count
            if (gSaveContext.equips.buttonItems[0] != ITEM_NONE) {
                Interface_DrawItemIconTexture_B_Btn(play, interfaceCtx->iconItemSegment, 0);

                if ((player->stateFlags1 & PLAYER_STATE1_23) || (play->shootingGalleryStatus > 1) ||
                    ((play->sceneId == SCENE_BOWLING) && Flags_GetSwitch(play, 0x38))) {
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE,
                                      0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
                    Interface_DrawAmmoCount(play, 0, interfaceCtx->bAlpha);
                }
            }
        } else {
            // B Button Do Action Label
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
							   PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
            
            gDPLoadTextureBlock_4b(OVERLAY_DISP++, interfaceCtx->doActionSegment + DO_ACTION_TEX_SIZE, G_IM_FMT_IA,
								   DO_ACTION_TEX_WIDTH, DO_ACTION_TEX_HEIGHT, 0, G_TX_NOMIRROR | G_TX_WRAP,
								   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            
            R_B_LABEL_DD = (1 << 10) / (WREG(37 + gSaveContext.language) / 100.0f);
            gSPTextureRectangle(OVERLAY_DISP++, R_B_LABEL_X(gSaveContext.language) + B_ACT_LABEL_X,
							   R_B_LABEL_Y(gSaveContext.language) + B_ACT_LABEL_Y,
							   (R_B_LABEL_X(gSaveContext.language) + DO_ACTION_TEX_WIDTH) << 2,
							   (R_B_LABEL_Y(gSaveContext.language) + DO_ACTION_TEX_HEIGHT) << 2, G_TX_RENDERTILE, 0, 0,
							   R_B_LABEL_DD, R_B_LABEL_DD
            );
        }
        
        gDPPipeSync(OVERLAY_DISP++);
        // C-Left Button Icon & Ammo Count
        if (gSaveContext.equips.buttonItems[1] < 0xF0) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->cLeftAlpha);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
            Interface_DrawItemIconTexture(play, interfaceCtx->iconItemSegment + 0x1000, 1);
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetCombineLERP(
                OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0
            );
            Interface_DrawAmmoCount(play, 1, interfaceCtx->cLeftAlpha);
        }
        
        gDPPipeSync(OVERLAY_DISP++);
        // C-Down Button Icon & Ammo Count
        if (gSaveContext.equips.buttonItems[2] < 0xF0) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->cDownAlpha);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
            Interface_DrawItemIconTexture(play, interfaceCtx->iconItemSegment + 0x2000, 2);
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetCombineLERP(
                OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0
            );
            Interface_DrawAmmoCount(play, 2, interfaceCtx->cDownAlpha);
        }
        
        gDPPipeSync(OVERLAY_DISP++);
        
        // C-Right Button Icon & Ammo Count
        if (gSaveContext.equips.buttonItems[3] < 0xF0) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->cRightAlpha);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
            Interface_DrawItemIconTexture(play, interfaceCtx->iconItemSegment + 0x3000, 3);
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetCombineLERP(
                OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0
            );
            Interface_DrawAmmoCount(play, 3, interfaceCtx->cRightAlpha);
        }
		
        Gfx_SetupDL_42Overlay(play->state.gfxCtx);
		
		gSPClearGeometryMode(OVERLAY_DISP++, G_CULL_BOTH);
		gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
		gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);

		Matrix_Translate(0.0f, 0.0f, -38.0f, MTXMODE_NEW);
		Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
		Matrix_RotateX(interfaceCtx->unk_1F4 / 10000.0f, MTXMODE_APPLY);
		
		aAlpha = interfaceCtx->aAlpha;

		if (aAlpha > 100) {
			aAlpha = 100;
		}
		
		// Draw A button Shadow
		#if Patch_MM_INTERFACE_BUTTONS_CORDS == true	
			#if Patch_MM_INTERFACE_SHADOWS == true
			gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
			gDPPipeSync(OVERLAY_DISP++);
			Interface_SetPerspectiveView(play, 23 + R_A_BTN_Y_OFFSET, 68 + R_A_BTN_Y_OFFSET, 194, 235);
			gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[4], 4, 0);
			gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, aAlpha);
			
			Interface_DrawActionButton(play);
			#endif
		#elif Patch_MM_INTERFACE_BUTTONS_CORDS == false	
			#if Patch_MM_INTERFACE_SHADOWS == true
			gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
			gDPPipeSync(OVERLAY_DISP++);
			Interface_SetPerspectiveView(play, 11 + R_A_BTN_Y_OFFSET, 53 + R_A_BTN_Y_OFFSET, 189, 235);
			gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[4], 4, 0);
			gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, aAlpha);	
			
			Interface_DrawActionButton(play);
			#endif
		#endif
		
			
		// Draw A Button Colored
		#if Patch_MM_INTERFACE_BUTTONS_CORDS == true	
		gDPPipeSync(OVERLAY_DISP++);
		Interface_SetPerspectiveView(play, 21 + R_A_BTN_Y_OFFSET, 66 + R_A_BTN_Y_OFFSET, 190, 235);
		gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[0], 4, 0);
        gDPSetPrimColor( OVERLAY_DISP++, 0, 0, R_A_BTN_COLOR(0), R_A_BTN_COLOR(1), R_A_BTN_COLOR(2), interfaceCtx->aAlpha);
		gSP1Quadrangle(OVERLAY_DISP++, 0, 2, 3, 1, 0);
		#elif Patch_MM_INTERFACE_BUTTONS_CORDS == false
        Gfx_SetupDL_42Overlay(play->state.gfxCtx);
        func_8008A8B8(play, R_A_BTN_Y, R_A_BTN_Y + 45, R_A_BTN_X, R_A_BTN_X + 45);
        gSPClearGeometryMode(OVERLAY_DISP++, G_CULL_BOTH);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_A_BTN_COLOR(0), R_A_BTN_COLOR(1), R_A_BTN_COLOR(2),
                        interfaceCtx->aAlpha);
		#endif
			
        Interface_DrawActionButton(play);
		
		#if Patch_MM_INTERFACE_BUTTONS_CORDS == true
        gDPPipeSync(OVERLAY_DISP++);
        func_8008A8B8(play, R_A_BTN_Y + 14, R_A_BTN_Y + 59, R_A_BTN_X, 8 + R_A_BTN_X + 45);
        gSPSetGeometryMode(OVERLAY_DISP++, G_CULL_BACK);
        gDPSetCombineLERP(
            OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
            PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
		#elif Patch_MM_INTERFACE_BUTTONS_CORDS == false
        gDPPipeSync(OVERLAY_DISP++);
        func_8008A8B8(play, R_A_ICON_Y, R_A_ICON_Y + 45, R_A_ICON_X, R_A_ICON_X + 45);
        gSPSetGeometryMode(OVERLAY_DISP++, G_CULL_BACK);
        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
		#endif
		
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->aAlpha);
        gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);
        gSPMatrix(
            OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx, "../z_parameter.c", 3701),
            G_MTX_MODELVIEW | G_MTX_LOAD
        );
        gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[4], 4, 0);
        
        if ((interfaceCtx->unk_1EC < 2) || (interfaceCtx->unk_1EC == 3)) {
            Interface_DrawActionLabel(play->state.gfxCtx, interfaceCtx->doActionSegment);
        } else {
            Interface_DrawActionLabel(play->state.gfxCtx, interfaceCtx->doActionSegment + DO_ACTION_TEX_SIZE);
        }
        
        gDPPipeSync(OVERLAY_DISP++);
        
        func_8008A994(interfaceCtx);
        
        if ((pauseCtx->state == 6) && (pauseCtx->unk_1E4 == 3)) {
            // Inventory Equip Effects
            gSPSegment(OVERLAY_DISP++, 0x08, pauseCtx->iconItemSegment);
            Gfx_SetupDL_42Overlay(play->state.gfxCtx);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
            gSPMatrix(OVERLAY_DISP++, &gMtxClear, G_MTX_MODELVIEW | G_MTX_LOAD);
            
            pauseCtx->cursorVtx[16].v.ob[0] = pauseCtx->cursorVtx[18].v.ob[0] = pauseCtx->equipAnimX / 10;
            pauseCtx->cursorVtx[17].v.ob[0] = pauseCtx->cursorVtx[19].v.ob[0] =
                pauseCtx->cursorVtx[16].v.ob[0] + WREG(90) / 10;
            pauseCtx->cursorVtx[16].v.ob[1] = pauseCtx->cursorVtx[17].v.ob[1] = pauseCtx->equipAnimY / 10;
            pauseCtx->cursorVtx[18].v.ob[1] = pauseCtx->cursorVtx[19].v.ob[1] =
                pauseCtx->cursorVtx[16].v.ob[1] - WREG(90) / 10;
            
            if (pauseCtx->equipTargetItem < 0xBF) {
                // Normal Equip (icon goes from the inventory slot to the C button when equipping it)
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, pauseCtx->equipAnimAlpha);
                gSPVertex(OVERLAY_DISP++, &pauseCtx->cursorVtx[16], 4, 0);
                
                gDPLoadTextureBlock(
                    OVERLAY_DISP++, gItemIcons[pauseCtx->equipTargetItem], G_IM_FMT_RGBA, G_IM_SIZ_32b,
                    32, 32, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                    G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD
                );
            } else {
                // Magic Arrow Equip Effect
                svar1 = pauseCtx->equipTargetItem - 0xBF;
                gDPSetPrimColor(
                    OVERLAY_DISP++, 0, 0, magicArrowEffectsR[svar1], magicArrowEffectsG[svar1],
                    magicArrowEffectsB[svar1], pauseCtx->equipAnimAlpha
                );
                
                if ((pauseCtx->equipAnimAlpha > 0) && (pauseCtx->equipAnimAlpha < 255)) {
                    svar1 = (pauseCtx->equipAnimAlpha / 8) / 2;
                    pauseCtx->cursorVtx[16].v.ob[0] = pauseCtx->cursorVtx[18].v.ob[0] =
                        pauseCtx->cursorVtx[16].v.ob[0] - svar1;
                    pauseCtx->cursorVtx[17].v.ob[0] = pauseCtx->cursorVtx[19].v.ob[0] =
                        pauseCtx->cursorVtx[16].v.ob[0] + svar1 * 2 + 32;
                    pauseCtx->cursorVtx[16].v.ob[1] = pauseCtx->cursorVtx[17].v.ob[1] =
                        pauseCtx->cursorVtx[16].v.ob[1] + svar1;
                    pauseCtx->cursorVtx[18].v.ob[1] = pauseCtx->cursorVtx[19].v.ob[1] =
                        pauseCtx->cursorVtx[16].v.ob[1] - svar1 * 2 - 32;
                }
                
                gSPVertex(OVERLAY_DISP++, &pauseCtx->cursorVtx[16], 4, 0);
                gDPLoadTextureBlock(
                    OVERLAY_DISP++, gMagicArrowEquipEffectTex, G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0,
                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                    G_TX_NOLOD, G_TX_NOLOD
                );
            }
            
            gSP1Quadrangle(OVERLAY_DISP++, 0, 2, 3, 1, 0);
        }
        
        Gfx_SetupDL_39Overlay(play->state.gfxCtx);
        //I IS HORSIE
        if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugState == 0)) {
            if (gSaveContext.minigameState != 1) {
                // Carrots rendering if the action corresponds to riding a horse
                if (interfaceCtx->unk_1EE == 8) {
                    // Load Carrot Icon
                    gDPLoadTextureBlock(
                        OVERLAY_DISP++, gCarrotIconTex, G_IM_FMT_RGBA, G_IM_SIZ_32b, 16, 16, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                        G_TX_NOLOD, G_TX_NOLOD
                    );
                    
                    // Draw 6 carrots
                    for (svar1 = 1, svar5 = ZREG(14); svar1 < 7; svar1++, svar5 += 16) {
                        // Carrot Color (based on availability)
                        if ((interfaceCtx->numHorseBoosts == 0) || (interfaceCtx->numHorseBoosts < svar1)) {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 150, 255, interfaceCtx->aAlpha);
                        } else {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->aAlpha);
                        }

                        gSPTextureRectangle(
                            OVERLAY_DISP++, svar5 << 2, ZREG(15) << 2, (svar5 + 16) << 2,
                                (ZREG(15) + 16) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10
                        );					
						// In future, this will be toggable with Patch_MM_INTERFACE_CARROTS to give one the ability to move these when a
						// timer is on screen
							//gSPTextureRectangle(OVERLAY_DISP++, svar5 << 2, ZREG(1) << 2, (svar5 + 16) << 2,
									//(ZREG(1) + 16) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
                    }
				}
            } else {
                // Score for the Horseback Archery
                svar5 = WREG(32);
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
                
                // Target Icon
                gDPLoadTextureBlock(
                    OVERLAY_DISP++, gArcheryScoreIconTex, G_IM_FMT_RGBA, G_IM_SIZ_16b, 24, 16, 0,
                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                    G_TX_NOLOD, G_TX_NOLOD
                );
                
                gSPTextureRectangle(
                    OVERLAY_DISP++, (svar5 + 28) << 2, ZREG(15) << 2, (svar5 + 52) << 2,
                        (ZREG(15) + 16) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10
                );
                
                // Score Counter
                gDPPipeSync(OVERLAY_DISP++);
                gDPSetCombineLERP(
                    OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE,
                    TEXEL0, 0, PRIMITIVE, 0
                );
                
                svar5 = WREG(32) + 6 * 9;
                
                for (svar1 = svar2 = 0; svar1 < 4; svar1++) {
                    if (sHBAScoreDigits[svar1] != 0 || (svar2 != 0) || (svar1 >= 3)) {
                        OVERLAY_DISP = Gfx_TextureI8(
                            OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * sHBAScoreDigits[svar1])), 8, 16, svar5,
                            (ZREG(15) - 2), digitWidth[0], VREG(42), VREG(43) << 1, VREG(43) << 1
                        );
                        svar5 += 9;
                        svar2++;
                    }
                }
                
                gDPPipeSync(OVERLAY_DISP++);
                gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM);
            }
        }
        
        if ((gSaveContext.timer2State == 5) && (Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT)) {
            // Trade quest timer reached 0
            D_8015FFE6 = 40;
            gSaveContext.cutsceneIndex = 0;
            play->transitionTrigger = TRANS_TRIGGER_START;
            play->transitionType = TRANS_TYPE_FADE_WHITE;
            gSaveContext.timer2State = 0;
            
            if (
                (gSaveContext.equips.buttonItems[0] != ITEM_SWORD_KOKIRI) &&
                (gSaveContext.equips.buttonItems[0] != ITEM_SWORD_MASTER) &&
                (gSaveContext.equips.buttonItems[0] != ITEM_SWORD_BGS) &&
                (gSaveContext.equips.buttonItems[0] != ITEM_SWORD_KNIFE)
            ) {
                if (gSaveContext.buttonStatus[0] != BTN_ENABLED) {
                    gSaveContext.equips.buttonItems[0] = gSaveContext.buttonStatus[0];
                } else {
                    gSaveContext.equips.buttonItems[0] = ITEM_NONE;
                }
            }
            
            // Revert any spoiling trade quest items
            for (svar1 = 0; svar1 < ARRAY_COUNT(gSpoilingItems); svar1++) {
                if (INV_CONTENT(ITEM_TRADE_ADULT) == gSpoilingItems[svar1]) {
                    gSaveContext.eventInf[EVENTINF_HORSES_INDEX] &=
                        (u16) ~(EVENTINF_HORSES_STATE_MASK | EVENTINF_HORSES_HORSETYPE_MASK | EVENTINF_HORSES_05_MASK |
                        EVENTINF_HORSES_06_MASK | EVENTINF_HORSES_0F_MASK);
                    osSyncPrintf("EVENT_INF=%x\n", gSaveContext.eventInf[EVENTINF_HORSES_INDEX]);
                    play->nextEntranceIndex = spoilingItemEntrances[svar1];
                    INV_CONTENT(gSpoilingItemReverts[svar1]) = gSpoilingItemReverts[svar1];
                    
                    for (svar2 = 1; svar2 < 4; svar2++) {
                        if (gSaveContext.equips.buttonItems[svar2] == gSpoilingItems[svar1]) {
                            gSaveContext.equips.buttonItems[svar2] = gSpoilingItemReverts[svar1];
                            Interface_LoadItemIcon1(play, svar2);
                        }
                    }
                }
            }
        }
        
        if (
            (play->pauseCtx.state == 0) && (play->pauseCtx.debugState == 0) &&
            (play->gameOverCtx.state == GAMEOVER_INACTIVE) && (msgCtx->msgMode == MSGMODE_NONE) &&
            !(player->stateFlags2 & PLAYER_STATE2_24) && (play->transitionTrigger == TRANS_TRIGGER_OFF) &&
            (play->transitionMode == TRANS_MODE_OFF) && !Play_InCsMode(play) && (gSaveContext.minigameState != 1) &&
            (play->shootingGalleryStatus <= 1) && !((play->sceneId == SCENE_BOWLING) && Flags_GetSwitch(play, 0x38))
        ) {
            svar6 = 0;
            switch (gSaveContext.timer1State) {
                case 1:
                    D_8015FFE2 = 20;
                    D_8015FFE0 = 20;
                    gSaveContext.timer1Value = gSaveContext.health >> 1;
                    gSaveContext.timer1State = 2;
                    break;
                case 2:
                    D_8015FFE2--;
                    if (D_8015FFE2 == 0) {
                        D_8015FFE2 = 20;
                        gSaveContext.timer1State = 3;
                    }
                    break;
                case 5:
                case 11:
                    D_8015FFE2 = 20;
                    D_8015FFE0 = 20;
                    if (gSaveContext.timer1State == 5) {
                        gSaveContext.timer1State = 6;
                    } else {
                        gSaveContext.timer1State = 12;
                    }
                    break;
                case 6:
                case 12:
                    D_8015FFE2--;
                    if (D_8015FFE2 == 0) {
                        D_8015FFE2 = 20;
                        if (gSaveContext.timer1State == 6) {
                            gSaveContext.timer1State = 7;
                        } else {
                            gSaveContext.timer1State = 13;
                        }
                    }
                    break;
                case 3:
                case 7:
                    svar1 = (gSaveContext.timerX[0] - 26) / D_8015FFE2;
                    gSaveContext.timerX[0] -= svar1;
                    
                    if (gSaveContext.healthCapacity > 0xA0) {
                        svar1 = (gSaveContext.timerY[0] - 54) / D_8015FFE2;
                    } else {
                        svar1 = (gSaveContext.timerY[0] - 46) / D_8015FFE2;
                    }
                    gSaveContext.timerY[0] -= svar1;
                    
                    D_8015FFE2--;
                    if (D_8015FFE2 == 0) {
                        D_8015FFE2 = 20;
                        gSaveContext.timerX[0] = 26;
                        
                        if (gSaveContext.healthCapacity > 0xA0) {
                            gSaveContext.timerY[0] = 54;
                        } else {
                            gSaveContext.timerY[0] = 46;
                        }
                        
                        if (gSaveContext.timer1State == 3) {
                            gSaveContext.timer1State = 4;
                        } else {
                            gSaveContext.timer1State = 8;
                        }
                    }
                    FALLTHROUGH;
                case 4:
                case 8:
                    if ((gSaveContext.timer1State == 4) || (gSaveContext.timer1State == 8)) {
                        if (gSaveContext.healthCapacity > 0xA0) {
                            gSaveContext.timerY[0] = 54;
                        } else {
                            gSaveContext.timerY[0] = 46;
                        }
                    }
                    
                    if ((gSaveContext.timer1State >= 3) && (msgCtx->msgLength == 0)) {
                        D_8015FFE0--;
                        if (D_8015FFE0 == 0) {
                            if (gSaveContext.timer1Value != 0) {
                                gSaveContext.timer1Value--;
                            }
                            
                            D_8015FFE0 = 20;
                            
                            if (gSaveContext.timer1Value == 0) {
                                gSaveContext.timer1State = 10;
                                if (D_80125A5C) {
                                    gSaveContext.health = 0;
                                    play->damagePlayer(play, -(gSaveContext.health + 2));
                                }
                                D_80125A5C = false;
                            } else if (gSaveContext.timer1Value > 60) {
                                if (timerDigits[4] == 1) {
                                    Audio_PlaySfxGeneral(
                                        NA_SE_SY_MESSAGE_WOMAN, &gSfxDefaultPos, 4,
                                        &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                        &gSfxDefaultReverb
                                    );
                                }
                            } else if (gSaveContext.timer1Value >= 11) {
                                if (timerDigits[4] & 1) {
                                    Audio_PlaySfxGeneral(
                                        NA_SE_SY_WARNING_COUNT_N, &gSfxDefaultPos, 4,
                                        &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                        &gSfxDefaultReverb
                                    );
                                }
                            } else {
                                Audio_PlaySfxGeneral(
                                    NA_SE_SY_WARNING_COUNT_E, &gSfxDefaultPos, 4,
                                    &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                    &gSfxDefaultReverb
                                );
                            }
                        }
                    }
                    break;
                case 13:
                    svar1 = (gSaveContext.timerX[0] - 26) / D_8015FFE2;
                    gSaveContext.timerX[0] -= svar1;
                    
                    if (gSaveContext.healthCapacity > 0xA0) {
                        svar1 = (gSaveContext.timerY[0] - 54) / D_8015FFE2;
                    } else {
                        svar1 = (gSaveContext.timerY[0] - 46) / D_8015FFE2;
                    }
                    gSaveContext.timerY[0] -= svar1;
                    
                    D_8015FFE2--;
                    if (D_8015FFE2 == 0) {
                        D_8015FFE2 = 20;
                        gSaveContext.timerX[0] = 26;
                        if (gSaveContext.healthCapacity > 0xA0) {
                            gSaveContext.timerY[0] = 54;
                        } else {
                            gSaveContext.timerY[0] = 46;
                        }
                        
                        gSaveContext.timer1State = 14;
                    }
                    FALLTHROUGH;
                case 14:
                    if (gSaveContext.timer1State == 14) {
                        if (gSaveContext.healthCapacity > 0xA0) {
                            gSaveContext.timerY[0] = 54;
                        } else {
                            gSaveContext.timerY[0] = 46;
                        }
                    }
                    
                    if (gSaveContext.timer1State >= 3) {
                        D_8015FFE0--;
                        if (D_8015FFE0 == 0) {
                            gSaveContext.timer1Value++;
                            D_8015FFE0 = 20;
                            
                            if (gSaveContext.timer1Value == 3599) {
                                D_8015FFE2 = 40;
                                gSaveContext.timer1State = 15;
                            } else {
                                Audio_PlaySfxGeneral(
                                    NA_SE_SY_WARNING_COUNT_N, &gSfxDefaultPos, 4,
                                    &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                    &gSfxDefaultReverb
                                );
                            }
                        }
                    }
                    break;
                case 10:
                    if (gSaveContext.timer2State != 0) {
                        D_8015FFE6 = 20;
                        D_8015FFE4 = 20;
                        gSaveContext.timerX[1] = 140;
                        gSaveContext.timerY[1] = 80;
                        
                        if (gSaveContext.timer2State < 7) {
                            gSaveContext.timer2State = 2;
                        } else {
                            gSaveContext.timer2State = 8;
                        }
                        
                        gSaveContext.timer1State = 0;
                    } else {
                        gSaveContext.timer1State = 0;
                    }
                case 15:
                    break;
                default:
                    svar6 = 1;
                    switch (gSaveContext.timer2State) {
                        case 1:
                        case 7:
                            D_8015FFE6 = 20;
                            D_8015FFE4 = 20;
                            gSaveContext.timerX[1] = 140;
                            gSaveContext.timerY[1] = 80;
                            if (gSaveContext.timer2State == 1) {
                                gSaveContext.timer2State = 2;
                            } else {
                                gSaveContext.timer2State = 8;
                            }
                            break;
                        case 2:
                        case 8:
                            D_8015FFE6--;
                            if (D_8015FFE6 == 0) {
                                D_8015FFE6 = 20;
                                if (gSaveContext.timer2State == 2) {
                                    gSaveContext.timer2State = 3;
                                } else {
                                    gSaveContext.timer2State = 9;
                                }
                            }
                            break;
                        case 3:
                        case 9:
                            osSyncPrintf(
                                "event_xp[1]=%d,  event_yp[1]=%d  TOTAL_EVENT_TM=%d\n",
                                ((void)0, gSaveContext.timerX[1]), ((void)0, gSaveContext.timerY[1]),
                                gSaveContext.timer2Value
                            );
                            svar1 = (gSaveContext.timerX[1] - 26) / D_8015FFE6;
                            gSaveContext.timerX[1] -= svar1;
                            if (gSaveContext.healthCapacity > 0xA0) {
                                svar1 = (gSaveContext.timerY[1] - 54) / D_8015FFE6;
                            } else {
                                svar1 = (gSaveContext.timerY[1] - 46) / D_8015FFE6;
                            }
                            gSaveContext.timerY[1] -= svar1;
                            
                            D_8015FFE6--;
                            if (D_8015FFE6 == 0) {
                                D_8015FFE6 = 20;
                                gSaveContext.timerX[1] = 26;
                                
                                if (gSaveContext.healthCapacity > 0xA0) {
                                    gSaveContext.timerY[1] = 54;
                                } else {
                                    gSaveContext.timerY[1] = 46;
                                }
                                
                                if (gSaveContext.timer2State == 3) {
                                    gSaveContext.timer2State = 4;
                                } else {
                                    gSaveContext.timer2State = 10;
                                }
                            }
                            FALLTHROUGH;
                        case 4:
                        case 10:
                            if ((gSaveContext.timer2State == 4) || (gSaveContext.timer2State == 10)) {
                                if (gSaveContext.healthCapacity > 0xA0) {
                                    gSaveContext.timerY[1] = 54;
                                } else {
                                    gSaveContext.timerY[1] = 46;
                                }
                            }
                            
                            if (gSaveContext.timer2State >= 3) {
                                D_8015FFE4--;
                                if (D_8015FFE4 == 0) {
                                    D_8015FFE4 = 20;
                                    if (gSaveContext.timer2State == 4) {
                                        gSaveContext.timer2Value--;
                                        osSyncPrintf("TOTAL_EVENT_TM=%d\n", gSaveContext.timer2Value);
                                        
                                        if (gSaveContext.timer2Value <= 0) {
                                            if (
                                                !Flags_GetSwitch(play, 0x37) ||
                                                ((play->sceneId != SCENE_GANON_DEMO) &&
                                                (play->sceneId != SCENE_GANON_FINAL) &&
                                                (play->sceneId != SCENE_GANON_SONOGO) &&
                                                (play->sceneId != SCENE_GANONTIKA_SONOGO))
                                            ) {
                                                D_8015FFE6 = 40;
                                                gSaveContext.timer2State = 5;
                                                gSaveContext.cutsceneIndex = 0;
                                                Message_StartTextbox(play, 0x71B0, NULL);
                                                func_8002DF54(play, NULL, 8);
                                            } else {
                                                D_8015FFE6 = 40;
                                                gSaveContext.timer2State = 6;
                                            }
                                        } else if (gSaveContext.timer2Value > 60) {
                                            if (timerDigits[4] == 1) {
                                                Audio_PlaySfxGeneral(
                                                    NA_SE_SY_MESSAGE_WOMAN, &gSfxDefaultPos, 4,
                                                    &gSfxDefaultFreqAndVolScale,
                                                    &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb
                                                );
                                            }
                                        } else if (gSaveContext.timer2Value > 10) {
                                            if (timerDigits[4] & 1) {
                                                Audio_PlaySfxGeneral(
                                                    NA_SE_SY_WARNING_COUNT_N, &gSfxDefaultPos, 4,
                                                    &gSfxDefaultFreqAndVolScale,
                                                    &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb
                                                );
                                            }
                                        } else {
                                            Audio_PlaySfxGeneral(
                                                NA_SE_SY_WARNING_COUNT_E, &gSfxDefaultPos, 4,
                                                &gSfxDefaultFreqAndVolScale,
                                                &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb
                                            );
                                        }
                                    } else {
                                        gSaveContext.timer2Value++;
                                        if (GET_EVENTINF(EVENTINF_10)) {
                                            if (gSaveContext.timer2Value == 240) {
                                                Message_StartTextbox(play, 0x6083, NULL);
                                                CLEAR_EVENTINF(EVENTINF_10);
                                                gSaveContext.timer2State = 0;
                                            }
                                        }
                                    }
                                    
                                    if ((gSaveContext.timer2Value % 60) == 0) {
                                        Audio_PlaySfxGeneral(
                                            NA_SE_SY_WARNING_COUNT_N, &gSfxDefaultPos, 4,
                                            &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                            &gSfxDefaultReverb
                                        );
                                    }
                                }
                            }
                            break;
                        case 6:
                            D_8015FFE6--;
                            if (D_8015FFE6 == 0) {
                                gSaveContext.timer2State = 0;
                            }
                            break;
                    }
                    break;
            }
            
            if (((gSaveContext.timer1State != 0) && (gSaveContext.timer1State != 10)) ||
                (gSaveContext.timer2State != 0)) {
                u32 remainderFrames = 0;
                static u32 milliseconds = 0;
                u8 isCountingUp = false;
                u8 isFrozen = false;
                #if Patch_MM_TIMER == true
                static u8 wow = 0;
                #endif
                for (int i = 0; i < TIMER_DIGITS; ++i)
                    timerDigits[i] = 0;
                timerDigits[2] = 10; // digit 10 is used as ':' (colon)

                if (gSaveContext.timer1State != 0) {
                    timerDigits[4] = gSaveContext.timer1Value;
                    remainderFrames = sTimerNextSecondTimer;
                    isCountingUp = gSaveContext.timer1State == TIMER_STATE_UP_INIT
                        || gSaveContext.timer1State == TIMER_STATE_UP_PREVIEW
                        || gSaveContext.timer1State == TIMER_STATE_UP_MOVE
                        || gSaveContext.timer1State == TIMER_STATE_UP_TICK
                        || gSaveContext.timer1State == TIMER_STATE_UP_FREEZE
                    ;
                    
                    if (gSaveContext.timer1State == TIMER_STATE_UP_INIT
                        || gSaveContext.timer1State == TIMER_STATE_UP_PREVIEW
                        || gSaveContext.timer1State == TIMER_STATE_UP_MOVE
                        || gSaveContext.timer1State == TIMER_STATE_DOWN_INIT
                        || gSaveContext.timer1State == TIMER_STATE_DOWN_PREVIEW
                        || gSaveContext.timer1State == TIMER_STATE_DOWN_MOVE
                        || gSaveContext.timer1State == TIMER_STATE_ENV_HAZARD_INIT
                        || gSaveContext.timer1State == TIMER_STATE_ENV_HAZARD_PREVIEW
                        || gSaveContext.timer1State == TIMER_STATE_ENV_HAZARD_MOVE
                    )
						#if Patch_MM_TIMER == true
                        isFrozen = true, wow = 0, milliseconds = 0;
						#endif
                    
                    if (gSaveContext.timer1State == TIMER_STATE_UP_FREEZE)
                        isFrozen = true;
                } else {
                    timerDigits[4] = gSaveContext.timer2Value;
                    remainderFrames = sSubTimerNextSecondTimer;
                    isCountingUp = gSaveContext.timer2State == SUBTIMER_STATE_UP_INIT
                        || gSaveContext.timer2State == SUBTIMER_STATE_UP_PREVIEW
                        || gSaveContext.timer2State == SUBTIMER_STATE_UP_MOVE
                        || gSaveContext.timer2State == SUBTIMER_STATE_UP_TICK
                    ;
                    
                    if (gSaveContext.timer2State == SUBTIMER_STATE_UP_INIT
                        || gSaveContext.timer2State == SUBTIMER_STATE_UP_PREVIEW
                        || gSaveContext.timer2State == SUBTIMER_STATE_UP_MOVE
                        || gSaveContext.timer2State == SUBTIMER_STATE_DOWN_INIT
                        || gSaveContext.timer2State == SUBTIMER_STATE_DOWN_PREVIEW
                        || gSaveContext.timer2State == SUBTIMER_STATE_DOWN_MOVE
                    )
						#if Patch_MM_TIMER == true
                        isFrozen = true, wow = 0, milliseconds = 0;
						#endif
                    
                    if (gSaveContext.timer2State == SUBTIMER_STATE_STOP)
                        isFrozen = true;
                }
                
                if (isFrozen == false)
                {
                    milliseconds = CLAMP_MAX((u32)(((remainderFrames % 20) / 20.0f) * 100.0f), 100) % 100;
                    
                    if (isCountingUp)
                        milliseconds = 99 - milliseconds;
                }
                    
                if (isCountingUp && remainderFrames > 17 && timerDigits[4] < 1)
                    milliseconds = 0;
                // when timer is counting down, make sure it stops at 00:00:00, not 00:01:00
                else if (isCountingUp == false && timerDigits[4] >= 1)
                    timerDigits[4] -= 1;

                while (timerDigits[4] >= 60) {
                    timerDigits[1]++;
                    if (timerDigits[1] >= 10) {
                        timerDigits[0]++;
                        timerDigits[1] -= 10;
                    }
                    timerDigits[4] -= 60;
                }
                
                #if Patch_MM_TIMER == true
                {
                    static u8 wowArr[] = { 0, 4, 7, 2, 1, 8, 3, 0, 9, 5, 6, 2, 7, 4, 0, 8, 1, 3, 9, 5, 2, 6, 4, 7, 0, 8, 1, 3, 9, 5, 2, 6 };
                    
                    timerDigits[5] = 10;
                    timerDigits[6] = milliseconds / 10;
                    timerDigits[7] = wowArr[wow % ARRAY_COUNT(wowArr)];
                    
                    if (isFrozen == false)
                        ++wow;
                }
                #endif

                while (timerDigits[4] >= 10) {
                    timerDigits[3]++;
                    timerDigits[4] -= 10;
                }
                
                // Clock Icon
                gDPPipeSync(OVERLAY_DISP++);
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
                gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);
                OVERLAY_DISP =
                    Gfx_TextureIA8(OVERLAY_DISP, gClockIconTex, 16, 16, ((void)0, gSaveContext.timerX[svar6]),
                                   ((void)0, gSaveContext.timerY[svar6]) + 2, 16, 16, 1 << 10, 1 << 10);

                // Timer Counter
                gDPPipeSync(OVERLAY_DISP++);
                gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE,
                                  TEXEL0, 0, PRIMITIVE, 0);


                if (gSaveContext.timer1State != 0) {
                    if ((gSaveContext.timer1Value <= 10) && (gSaveContext.timer1State < 11)) {
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 50, 0, 255);
                    } else {
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
                    }
                } else {
                    if ((gSaveContext.timer2Value <= 10) && (gSaveContext.timer2State < 6)) {
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 50, 0, 255);
                    } else {
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 0, 255);
                    }
                }

                for (svar1 = 0; svar1 < TIMER_DIGITS; svar1++) {
                    OVERLAY_DISP =
                        Gfx_TextureI8(OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * timerDigits[svar1])), 8, 16,
                                      ((void)0, gSaveContext.timerX[svar6]) + timerDigitLeftPos[svar1],
                                      ((void)0, gSaveContext.timerY[svar6]), digitWidth[svar1], VREG(42), VREG(43) << 1,
                                      VREG(43) << 1);
                }
            }
        }
    }
    
    if (pauseCtx->debugState == 3) {
        FlagSet_Update(play);
    }
    
    if (interfaceCtx->unk_244 != 0) {
        gDPPipeSync(OVERLAY_DISP++);
        gSPDisplayList(OVERLAY_DISP++, sSetupDL_80125A60);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->unk_244);
        gDPFillRectangle(OVERLAY_DISP++, 0, 0, gScreenWidth - 1, gScreenHeight - 1);
    }
    
    CLOSE_DISPS(play->state.gfxCtx, "../z_parameter.c", 4269);
}

Asm_VanillaHook(Interface_LoadActionLabel);
void Interface_LoadActionLabel(InterfaceContext* interfaceCtx, u16 action, s16 loadOffset) {
    static void* sDoActionTextures[] = { gAttackDoActionENGTex, gCheckDoActionENGTex };
    
    if (action >= DO_ACTION_MAX) {
        action = DO_ACTION_NONE;
    }
    
#if 0                          // RIP other languages
    if (gSaveContext.language != LANGUAGE_ENG) {
        action += DO_ACTION_MAX;
    }
    
    if (gSaveContext.language == LANGUAGE_FRA) {
        action += DO_ACTION_MAX;
    }
#endif
    
    if ((action != DO_ACTION_NONE) && (action != DO_ACTION_MAX + DO_ACTION_NONE) && (action != 2 * DO_ACTION_MAX + DO_ACTION_NONE)) {
        osCreateMesgQueue(&interfaceCtx->loadQueue, &interfaceCtx->loadMsg, OS_MESG_BLOCK);
        DmaMgr_SendRequest2(
            &interfaceCtx->dmaRequest_160,
            (u32)interfaceCtx->doActionSegment + (loadOffset * DO_ACTION_TEX_SIZE),
            gDmaDataTable[17].vromStart + (action * DO_ACTION_TEX_SIZE),
            DO_ACTION_TEX_SIZE,
            0,
            &interfaceCtx->loadQueue,
            NULL,
            NULL,
            0
        );
        osRecvMesg(&interfaceCtx->loadQueue, NULL, OS_MESG_BLOCK);
    } else {
        gSegments[7] = VIRTUAL_TO_PHYSICAL(interfaceCtx->doActionSegment);
        func_80086D5C(SEGMENTED_TO_VIRTUAL(sDoActionTextures[loadOffset]), DO_ACTION_TEX_SIZE / 4);
    }
}

Asm_VanillaHook(Interface_LoadActionLabelB);
void Interface_LoadActionLabelB(PlayState* playState, u16 action) {
    InterfaceContext* interfaceCtx = &playState->interfaceCtx;
    
#if 0
    if (gSaveContext.language != LANGUAGE_ENG) {
        action += DO_ACTION_MAX;
    }
    
    if (gSaveContext.language == LANGUAGE_FRA) {
        action += DO_ACTION_MAX;
    }
#endif
    
    interfaceCtx->unk_1FC = action;
    
    osCreateMesgQueue(&interfaceCtx->loadQueue, &interfaceCtx->loadMsg, OS_MESG_BLOCK);
    DmaMgr_SendRequest2(
        &interfaceCtx->dmaRequest_160,
        (u32)interfaceCtx->doActionSegment + DO_ACTION_TEX_SIZE,
        gDmaDataTable[17].vromStart + (action * DO_ACTION_TEX_SIZE),
        DO_ACTION_TEX_SIZE,
        0,
        &interfaceCtx->loadQueue,
        NULL,
        NULL,
        0
    );
    osRecvMesg(&interfaceCtx->loadQueue, NULL, OS_MESG_BLOCK);
    
    interfaceCtx->unk_1FA = 1;
}

Asm_VanillaHook(Magic_Fill);
void Magic_Fill(PlayState* play) {
    if (gSaveContext.isMagicAcquired) {
        gSaveContext.prevMagicState = gSaveContext.magicState;
        gSaveContext.magicFillTarget = (gSaveContext.isDoubleMagicAcquired + 1) * MAGIC_NORMAL_METER;
        gSaveContext.magicState = MAGIC_STATE_FILL;
    }
}

Asm_VanillaHook(Magic_Reset);
void Magic_Reset(PlayState* play) {
    if ((gSaveContext.magicState != MAGIC_STATE_STEP_CAPACITY) && (gSaveContext.magicState != MAGIC_STATE_FILL)) {
        if (gSaveContext.magicState == MAGIC_STATE_ADD) {
            gSaveContext.prevMagicState = gSaveContext.magicState;
        }
        gSaveContext.magicState = MAGIC_STATE_RESET;
    }
}

/**
 * Request to either increase or consume magic.
 * @param amount the positive-valued amount to either increase or decrease magic by
 * @param type how the magic is increased or consumed.
 * @return false if the request failed
 */
Asm_VanillaHook(Magic_RequestChange);
s32 Magic_RequestChange(PlayState* play, s16 amount, s16 type) {
    if (!gSaveContext.isMagicAcquired) {
        return false;
    }

    if ((type != MAGIC_ADD) && (gSaveContext.magic - amount) < 0) {
        if (gSaveContext.magicCapacity != 0) {
            Audio_PlaySfxGeneral(NA_SE_SY_ERROR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                 &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        }
        return false;
    }

    switch (type) {
        case MAGIC_CONSUME_NOW:
        case MAGIC_CONSUME_NOW_ALT:
            // Consume magic immediately
            if ((gSaveContext.magicState == MAGIC_STATE_IDLE) ||
                (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS)) {
                if (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS) {
                    play->actorCtx.lensActive = false;
                }
                gSaveContext.magicTarget = gSaveContext.magic - amount;
                gSaveContext.magicState = MAGIC_STATE_CONSUME_SETUP;
                return true;
            } else {
                Audio_PlaySfxGeneral(NA_SE_SY_ERROR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                return false;
            }

        case MAGIC_CONSUME_WAIT_NO_PREVIEW:
            // Sets consume target but waits to consume.
            // No yellow magic to preview target consumption.
            // Unused
            if ((gSaveContext.magicState == MAGIC_STATE_IDLE) ||
                (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS)) {
                if (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS) {
                    play->actorCtx.lensActive = false;
                }
                gSaveContext.magicTarget = gSaveContext.magic - amount;
                gSaveContext.magicState = MAGIC_STATE_METER_FLASH_3;
                return true;
            } else {
                Audio_PlaySfxGeneral(NA_SE_SY_ERROR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                return false;
            }

        case MAGIC_CONSUME_LENS:
            if (gSaveContext.magicState == MAGIC_STATE_IDLE) {
                if (gSaveContext.magic != 0) {
                    play->interfaceCtx.lensMagicConsumptionTimer = 80;
                    gSaveContext.magicState = MAGIC_STATE_CONSUME_LENS;
                    return true;
                } else {
                    return false;
                }
            } else if (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS) {
                return true;
            } else {
                return false;
            }

        case MAGIC_CONSUME_WAIT_PREVIEW:
            // Sets consume target but waits to consume.
            // Preview consumption with a yellow bar
            if ((gSaveContext.magicState == MAGIC_STATE_IDLE) ||
                (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS)) {
                if (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS) {
                    play->actorCtx.lensActive = false;
                }
                gSaveContext.magicTarget = gSaveContext.magic - amount;
                gSaveContext.magicState = MAGIC_STATE_METER_FLASH_2;
                return true;
            } else {
                Audio_PlaySfxGeneral(NA_SE_SY_ERROR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                return false;
            }

        case MAGIC_ADD:
            // Sets target for magic to increase to
            if (gSaveContext.magic <= gSaveContext.magicCapacity) {
                gSaveContext.magicTarget = gSaveContext.magic + amount;

                if (gSaveContext.magicTarget >= gSaveContext.magicCapacity) {
                    gSaveContext.magicTarget = gSaveContext.magicCapacity;
                }

                gSaveContext.magicState = MAGIC_STATE_ADD;
                return true;
            }
            break;
    }

    return false;
}

Asm_VanillaHook(Magic_Update);
void Magic_Update(PlayState* play) {
    static s16 sMagicBorderColors[][3] = {
        { 255, 255, 255 },
        { 150, 150, 150 },
        { 255, 255, 150 }, // unused
        { 255, 255, 50 },  // unused
    };
    static s16 sMagicBorderIndices[] = { 0, 1, 1, 0 };
    static s16 sMagicBorderRatio = 2;
    static s16 sMagicBorderStep = 1;
    MessageContext* msgCtx = &play->msgCtx;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 borderChangeR;
    s16 borderChangeG;
    s16 borderChangeB;
    s16 temp; // target for magicCapacity, or magicBorderIndex

    switch (gSaveContext.magicState) {
        case MAGIC_STATE_STEP_CAPACITY:
            // Step magicCapacity to the capacity determined by magicLevel
            // This changes the width of the magic meter drawn
            temp = gSaveContext.magicLevel * MAGIC_NORMAL_METER;
            if (gSaveContext.magicCapacity != temp) {
                if (gSaveContext.magicCapacity < temp) {
                    gSaveContext.magicCapacity += 8;
                    if (gSaveContext.magicCapacity > temp) {
                        gSaveContext.magicCapacity = temp;
                    }
                } else {
                    gSaveContext.magicCapacity -= 8;
                    if (gSaveContext.magicCapacity <= temp) {
                        gSaveContext.magicCapacity = temp;
                    }
                }
            } else {
                // Once the capacity has reached its target,
                // follow up by filling magic to magicFillTarget
                gSaveContext.magicState = MAGIC_STATE_FILL;
            }
            break;

        case MAGIC_STATE_FILL:
            // Add magic until magicFillTarget is reached
            gSaveContext.magic += 4;

            if (gSaveContext.gameMode == GAMEMODE_NORMAL && !IS_CUTSCENE_LAYER) {
                Audio_PlaySfxGeneral(NA_SE_SY_GAUGE_UP - SFX_FLAG, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            }

            // "Storage  MAGIC_NOW=%d (%d)"
            osSyncPrintf("蓄電  MAGIC_NOW=%d (%d)\n", gSaveContext.magic, gSaveContext.magicFillTarget);

            if (gSaveContext.magic >= gSaveContext.magicFillTarget) {
                gSaveContext.magic = gSaveContext.magicFillTarget;
                gSaveContext.magicState = gSaveContext.prevMagicState;
                gSaveContext.prevMagicState = MAGIC_STATE_IDLE;
            }
            break;

        case MAGIC_STATE_CONSUME_SETUP:
            // Sets the speed at which magic border flashes
            sMagicBorderRatio = 2;
            gSaveContext.magicState = MAGIC_STATE_CONSUME;
            break;

        case MAGIC_STATE_CONSUME:
            // Consume magic until target is reached or no more magic is available
            gSaveContext.magic -= 2;
            if (gSaveContext.magic <= 0) {
                gSaveContext.magic = 0;
                gSaveContext.magicState = MAGIC_STATE_METER_FLASH_1;
                sMagicBorderR = sMagicBorderG = sMagicBorderB = 255;
            } else if (gSaveContext.magic == gSaveContext.magicTarget) {
                gSaveContext.magicState = MAGIC_STATE_METER_FLASH_1;
                sMagicBorderR = sMagicBorderG = sMagicBorderB = 255;
            }
            FALLTHROUGH; // Flash border while magic is being consumed
        case MAGIC_STATE_METER_FLASH_1:
        case MAGIC_STATE_METER_FLASH_2:
        case MAGIC_STATE_METER_FLASH_3:
            temp = sMagicBorderIndices[sMagicBorderStep];
            borderChangeR = ABS(sMagicBorderR - sMagicBorderColors[temp][0]) / sMagicBorderRatio;
            borderChangeG = ABS(sMagicBorderG - sMagicBorderColors[temp][1]) / sMagicBorderRatio;
            borderChangeB = ABS(sMagicBorderB - sMagicBorderColors[temp][2]) / sMagicBorderRatio;

            if (sMagicBorderR >= sMagicBorderColors[temp][0]) {
                sMagicBorderR -= borderChangeR;
            } else {
                sMagicBorderR += borderChangeR;
            }

            if (sMagicBorderG >= sMagicBorderColors[temp][1]) {
                sMagicBorderG -= borderChangeG;
            } else {
                sMagicBorderG += borderChangeG;
            }

            if (sMagicBorderB >= sMagicBorderColors[temp][2]) {
                sMagicBorderB -= borderChangeB;
            } else {
                sMagicBorderB += borderChangeB;
            }

            sMagicBorderRatio--;
            if (sMagicBorderRatio == 0) {
                sMagicBorderR = sMagicBorderColors[temp][0];
                sMagicBorderG = sMagicBorderColors[temp][1];
                sMagicBorderB = sMagicBorderColors[temp][2];
                sMagicBorderRatio = YREG(40 + sMagicBorderStep);
                sMagicBorderStep++;
                if (sMagicBorderStep >= 4) {
                    sMagicBorderStep = 0;
                }
            }
            break;

        case MAGIC_STATE_RESET:
            sMagicBorderR = sMagicBorderG = sMagicBorderB = 255;
            gSaveContext.magicState = MAGIC_STATE_IDLE;
            break;

        case MAGIC_STATE_CONSUME_LENS:
            // Slowly consume magic while lens is on
            if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugState == 0) && (msgCtx->msgMode == MSGMODE_NONE) &&
                (play->gameOverCtx.state == GAMEOVER_INACTIVE) && (play->transitionTrigger == TRANS_TRIGGER_OFF) &&
                (play->transitionMode == TRANS_MODE_OFF) && !Play_InCsMode(play)) {
                if ((gSaveContext.magic == 0) || ((func_8008F2F8(play) >= 2) && (func_8008F2F8(play) < 5)) ||
                    ((gSaveContext.equips.buttonItems[1] != ITEM_LENS) &&
                     (gSaveContext.equips.buttonItems[2] != ITEM_LENS) &&
                     (gSaveContext.equips.buttonItems[3] != ITEM_LENS)) ||
                    !play->actorCtx.lensActive) {
                    // Force lens off and set magic meter state to idle
                    play->actorCtx.lensActive = false;
                    Audio_PlaySfxGeneral(NA_SE_SY_GLASSMODE_OFF, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                         &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                    gSaveContext.magicState = MAGIC_STATE_IDLE;
                    sMagicBorderR = sMagicBorderG = sMagicBorderB = 255;
                    break;
                }

                interfaceCtx->lensMagicConsumptionTimer--;
                if (interfaceCtx->lensMagicConsumptionTimer == 0) {
                    gSaveContext.magic--;
                    interfaceCtx->lensMagicConsumptionTimer = 80;
                }
            }

            temp = sMagicBorderIndices[sMagicBorderStep];
            borderChangeR = ABS(sMagicBorderR - sMagicBorderColors[temp][0]) / sMagicBorderRatio;
            borderChangeG = ABS(sMagicBorderG - sMagicBorderColors[temp][1]) / sMagicBorderRatio;
            borderChangeB = ABS(sMagicBorderB - sMagicBorderColors[temp][2]) / sMagicBorderRatio;

            if (sMagicBorderR >= sMagicBorderColors[temp][0]) {
                sMagicBorderR -= borderChangeR;
            } else {
                sMagicBorderR += borderChangeR;
            }

            if (sMagicBorderG >= sMagicBorderColors[temp][1]) {
                sMagicBorderG -= borderChangeG;
            } else {
                sMagicBorderG += borderChangeG;
            }

            if (sMagicBorderB >= sMagicBorderColors[temp][2]) {
                sMagicBorderB -= borderChangeB;
            } else {
                sMagicBorderB += borderChangeB;
            }

            sMagicBorderRatio--;
            if (sMagicBorderRatio == 0) {
                sMagicBorderR = sMagicBorderColors[temp][0];
                sMagicBorderG = sMagicBorderColors[temp][1];
                sMagicBorderB = sMagicBorderColors[temp][2];
                sMagicBorderRatio = YREG(40 + sMagicBorderStep);
                sMagicBorderStep++;
                if (sMagicBorderStep >= 4) {
                    sMagicBorderStep = 0;
                }
            }
            break;

        case MAGIC_STATE_ADD:
            // Add magic until target is reached
            gSaveContext.magic += 4;
            Audio_PlaySfxGeneral(NA_SE_SY_GAUGE_UP - SFX_FLAG, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                 &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            if (gSaveContext.magic >= gSaveContext.magicTarget) {
                gSaveContext.magic = gSaveContext.magicTarget;
                gSaveContext.magicState = gSaveContext.prevMagicState;
                gSaveContext.prevMagicState = MAGIC_STATE_IDLE;
            }
            break;

        default:
            gSaveContext.magicState = MAGIC_STATE_IDLE;
            break;
    }
}

Asm_VanillaHook(Magic_DrawMeter);
void Magic_DrawMeter(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 magicMeterY;
	s16 sMagicMeterOutlinePrimRed = 255;
	s16 sMagicMeterOutlinePrimGreen = 255;
	s16 sMagicMeterOutlinePrimBlue = 255;
	#define magicBarY magicMeterY

    OPEN_DISPS(play->state.gfxCtx, "../z_parameter.c", 2650);

    if (gSaveContext.magicLevel != 0) {
        if (gSaveContext.healthCapacity > 0xA0) {
            magicMeterY = R_MAGIC_METER_Y_LOWER; // two rows of hearts
        } else {
            magicMeterY = R_MAGIC_METER_Y_HIGHER; // one row of hearts
        }

        Gfx_SetupDL_39Overlay(play->state.gfxCtx);

        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sMagicBorderR, sMagicBorderG, sMagicBorderB, interfaceCtx->magicAlpha);
        gDPSetEnvColor(OVERLAY_DISP++, 100, 50, 50, 255);
		
		#if Patch_MM_INTERFACE_SHADOWS == true
        OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(
            OVERLAY_DISP, gMagicMeterEndTex, 8, 16, 18, magicBarY, 8, 16, 1 << 10, 1 << 10, sMagicMeterOutlinePrimRed,
            sMagicMeterOutlinePrimGreen, sMagicMeterOutlinePrimBlue, interfaceCtx->magicAlpha);
			
        OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(OVERLAY_DISP, gMagicMeterMidTex, 24, 16, 26, magicBarY,
                                                     ((void)0, gSaveContext.magicCapacity), 16, 1 << 10, 1 << 10,
                                                     sMagicMeterOutlinePrimRed, sMagicMeterOutlinePrimGreen,
                                                     sMagicMeterOutlinePrimBlue, interfaceCtx->magicAlpha);
        OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadowOffset(
            OVERLAY_DISP, gMagicMeterEndTex, 8, 16, ((void)0, gSaveContext.magicCapacity) + 26, magicBarY, 8, 16,
            1 << 10, 1 << 10, sMagicMeterOutlinePrimRed, sMagicMeterOutlinePrimGreen, sMagicMeterOutlinePrimBlue,
            interfaceCtx->magicAlpha, 3, 0x100);

        OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, gMagicMeterEndTex, 8, 16, R_MAGIC_METER_X, magicMeterY, 8, 16,
                                      1 << 10, 1 << 10);
		#elif Patch_MM_INTERFACE_SHADOWS == false
        OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, gMagicMeterEndTex, 8, 16, R_MAGIC_METER_X, magicMeterY, 8, 16,
                                      1 << 10, 1 << 10);

        OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, gMagicMeterMidTex, 24, 16, R_MAGIC_METER_X + 8, magicMeterY,
                                      gSaveContext.magicCapacity, 16, 1 << 10, 1 << 10);

        gDPLoadTextureBlock(OVERLAY_DISP++, gMagicMeterEndTex, G_IM_FMT_IA, G_IM_SIZ_8b, 8, 16, 0,
                            G_TX_MIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 3, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gSPTextureRectangle(OVERLAY_DISP++, (R_MAGIC_METER_X + gSaveContext.magicCapacity + 8) << 2, magicMeterY << 2,
                            (R_MAGIC_METER_X + gSaveContext.magicCapacity + 16) << 2, (magicMeterY + 16) << 2,
                            G_TX_RENDERTILE, 256, 0, 1 << 10, 1 << 10);
		#endif
		
        gDPPipeSync(OVERLAY_DISP++);
        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, PRIMITIVE, PRIMITIVE,
                          ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, PRIMITIVE);
        gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);

        if (gSaveContext.magicState == MAGIC_STATE_METER_FLASH_2) {
            // Yellow part of the meter indicating the amount of magic to be subtracted
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 250, 250, 0, interfaceCtx->magicAlpha);

            gDPLoadMultiBlock_4b(OVERLAY_DISP++, gMagicMeterFillTex, 0x0000, G_TX_RENDERTILE, G_IM_FMT_I, 16, 16, 0,
                                 G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                 G_TX_NOLOD, G_TX_NOLOD);

            gSPTextureRectangle(OVERLAY_DISP++, R_MAGIC_FILL_X << 2, (magicMeterY + 3) << 2,
                                (R_MAGIC_FILL_X + gSaveContext.magic) << 2, (magicMeterY + 10) << 2, G_TX_RENDERTILE, 0,
                                0, 1 << 10, 1 << 10);

            // Fill the rest of the meter with the normal magic color
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_MAGIC_FILL_COLOR(0), R_MAGIC_FILL_COLOR(1), R_MAGIC_FILL_COLOR(2),
                            interfaceCtx->magicAlpha);

            gSPTextureRectangle(OVERLAY_DISP++, R_MAGIC_FILL_X << 2, (magicMeterY + 3) << 2,
                                (R_MAGIC_FILL_X + gSaveContext.magicTarget) << 2, (magicMeterY + 10) << 2,
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        } else {
            // Fill the whole meter with the normal magic color
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, R_MAGIC_FILL_COLOR(0), R_MAGIC_FILL_COLOR(1), R_MAGIC_FILL_COLOR(2),
                            interfaceCtx->magicAlpha);

            gDPLoadMultiBlock_4b(OVERLAY_DISP++, gMagicMeterFillTex, 0x0000, G_TX_RENDERTILE, G_IM_FMT_I, 16, 16, 0,
                                 G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                 G_TX_NOLOD, G_TX_NOLOD);

            gSPTextureRectangle(OVERLAY_DISP++, R_MAGIC_FILL_X << 2, (magicMeterY + 3) << 2,
                                (R_MAGIC_FILL_X + gSaveContext.magic) << 2, (magicMeterY + 10) << 2, G_TX_RENDERTILE, 0,
                                0, 1 << 10, 1 << 10);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx, "../z_parameter.c", 2731);
}

Asm_VanillaHook(Interface_LoadItemIcon1);
void Interface_LoadItemIcon1(PlayState* playState, u16 button) {
    InterfaceContext* interfaceCtx = &playState->interfaceCtx;
    
    osCreateMesgQueue(&interfaceCtx->loadQueue, &interfaceCtx->loadMsg, OS_MESG_BLOCK);
    DmaMgr_SendRequest2(
        &interfaceCtx->dmaRequest_160,
        (u32)interfaceCtx->iconItemSegment + button * 0x1000,
        gDmaDataTable[7].vromStart + (gSaveContext.equips.buttonItems[button] * 0x1000),
        0x1000,
        0,
        &interfaceCtx->loadQueue,
        NULL,
        NULL,
        0
    );
    osRecvMesg(&interfaceCtx->loadQueue, NULL, OS_MESG_BLOCK);
}

Asm_VanillaHook(Interface_LoadItemIcon2);
void Interface_LoadItemIcon2(PlayState* playState, u16 button) {
    osCreateMesgQueue(&playState->interfaceCtx.loadQueue, &playState->interfaceCtx.loadMsg, OS_MESG_BLOCK);
    DmaMgr_SendRequest2(
        &playState->interfaceCtx.dmaRequest_180,
        (u32)playState->interfaceCtx.iconItemSegment + button * 0x1000,
        gDmaDataTable[7].vromStart + (gSaveContext.equips.buttonItems[button] * 0x1000),
        0x1000,
        0,
        &playState->interfaceCtx.loadQueue,
        NULL,
        NULL,
        0
    );
    osRecvMesg(&playState->interfaceCtx.loadQueue, NULL, OS_MESG_BLOCK);
}

Asm_VanillaHook(Minimap_Draw);
void Minimap_Draw(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s32 mapIndex = gSaveContext.mapIndex;
    
    OPEN_DISPS(play->state.gfxCtx, "../z_map_exp.c", 626);
    
    if (play->pauseCtx.state < 4) {
        switch (play->sceneId) {
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
                if (!R_MINIMAP_DISABLED) {
                    Gfx_SetupDL_39Overlay(play->state.gfxCtx);
                    gDPSetCombineLERP(
                        OVERLAY_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0,
                        TEXEL0, 0, PRIMITIVE, 0
                    );
                    
                    if (CHECK_DUNGEON_ITEM(DUNGEON_MAP, mapIndex)) {
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 100, 255, 255, interfaceCtx->minimapAlpha);
                        
                        gDPLoadTextureBlock_4b(
                            OVERLAY_DISP++, interfaceCtx->mapSegment, G_IM_FMT_I, 96, 85, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                            G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD
                        );
                        
                        gSPTextureRectangle(
                            OVERLAY_DISP++, R_DGN_MINIMAP_X << 2, R_DGN_MINIMAP_Y << 2,
                                (R_DGN_MINIMAP_X + 96) << 2, (R_DGN_MINIMAP_Y + 85) << 2, G_TX_RENDERTILE,
                                0, 0, 1 << 10, 1 << 10
                        );
                    }
                    
                    if (CHECK_DUNGEON_ITEM(DUNGEON_COMPASS, mapIndex)) {
                        Minimap_DrawCompassIcons(play); // Draw icons for the player spawn and current position
                        Gfx_SetupDL_39Overlay(play->state.gfxCtx);
                        MapMark_Draw(play);
                    }
                }
                
                if (play->frameAdvCtx.enabled == false && CHECK_BTN_ALL(play->state.input[0].press.button, BTN_L) && !Play_InCsMode(play)) {
                    if (!R_MINIMAP_DISABLED) {
                        Audio_PlaySfxGeneral(
                            NA_SE_SY_CAMERA_ZOOM_UP, &gSfxDefaultPos, 4,
                            &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                            &gSfxDefaultReverb
                        );
                    } else {
                        Audio_PlaySfxGeneral(
                            NA_SE_SY_CAMERA_ZOOM_DOWN, &gSfxDefaultPos, 4,
                            &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                            &gSfxDefaultReverb
                        );
                    }
                    
                    R_MINIMAP_DISABLED ^= 1;
                }
                
                break;
            case SCENE_SPOT00:
            case SCENE_SPOT01:
            case SCENE_SPOT02:
            case SCENE_SPOT03:
            case SCENE_SPOT04:
            case SCENE_SPOT05:
            case SCENE_SPOT06:
            case SCENE_SPOT07:
            case SCENE_SPOT08:
            case SCENE_SPOT09:
            case SCENE_SPOT10:
            case SCENE_SPOT11:
            case SCENE_SPOT12:
            case SCENE_SPOT13:
            case SCENE_SPOT15:
            case SCENE_SPOT16:
            case SCENE_SPOT17:
            case SCENE_SPOT18:
            case SCENE_SPOT20:
            case SCENE_GANON_TOU:
                if (!R_MINIMAP_DISABLED) {
                    Gfx_SetupDL_39Overlay(play->state.gfxCtx);
                    
                    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                    gDPSetPrimColor(
                        OVERLAY_DISP++, 0, 0, R_MINIMAP_COLOR(0), R_MINIMAP_COLOR(1), R_MINIMAP_COLOR(2),
                        interfaceCtx->minimapAlpha
                    );
                    
                    gDPLoadTextureBlock_4b(
                        OVERLAY_DISP++, interfaceCtx->mapSegment, G_IM_FMT_IA,
                        gMapData->owMinimapWidth[mapIndex], gMapData->owMinimapHeight[mapIndex], 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD
                    );
                    
                    gSPTextureRectangle(
                        OVERLAY_DISP++, R_OW_MINIMAP_X << 2, R_OW_MINIMAP_Y << 2,
                            (R_OW_MINIMAP_X + gMapData->owMinimapWidth[mapIndex]) << 2,
                            (R_OW_MINIMAP_Y + gMapData->owMinimapHeight[mapIndex]) << 2, G_TX_RENDERTILE, 0,
                            0, 1 << 10, 1 << 10
                    );
                    
                    if (
                        ((play->sceneId != SCENE_SPOT01) && (play->sceneId != SCENE_SPOT04) &&
                        (play->sceneId != SCENE_SPOT08)) ||
                        (LINK_AGE_IN_YEARS != YEARS_ADULT)
                    ) {
                        if (
                            (gMapData->owEntranceFlag[sEntranceIconMapIndex] == 0xFFFF) ||
                            ((gMapData->owEntranceFlag[sEntranceIconMapIndex] != 0xFFFF) &&
                            (gSaveContext.infTable[INFTABLE_1AX_INDEX] &
                            gBitFlags[gMapData->owEntranceFlag[mapIndex]]))
                        ) {
                            
                            gDPLoadTextureBlock(
                                OVERLAY_DISP++, gMapDungeonEntranceIconTex, G_IM_FMT_RGBA, G_IM_SIZ_16b,
                                8, 8, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                                G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD
                            );
                            
                            gSPTextureRectangle(
                                OVERLAY_DISP++,
                                gMapData->owEntranceIconPosX[sEntranceIconMapIndex] << 2,
                                    gMapData->owEntranceIconPosY[sEntranceIconMapIndex] << 2,
                                    (gMapData->owEntranceIconPosX[sEntranceIconMapIndex] + 8) << 2,
                                    (gMapData->owEntranceIconPosY[sEntranceIconMapIndex] + 8) << 2,
                                    G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10
                            );
                        }
                    }
                    
                    if (
                        (play->sceneId == SCENE_SPOT08) &&
                        (gSaveContext.infTable[INFTABLE_1AX_INDEX] & gBitFlags[INFTABLE_1A9_SHIFT])
                    ) {
                        gDPLoadTextureBlock(
                            OVERLAY_DISP++, gMapDungeonEntranceIconTex, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8,
                            8, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                            G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD
                        );
                        
                        gSPTextureRectangle(
                            OVERLAY_DISP++, 270 << 2, 154 << 2, 278 << 2, 162 << 2, G_TX_RENDERTILE, 0,
                                0, 1 << 10, 1 << 10
                        );
                    }
                    
                    Minimap_DrawCompassIcons(play); // Draw icons for the player spawn and current position
                }
                
                if (play->frameAdvCtx.enabled == false && CHECK_BTN_ALL(play->state.input[0].press.button, BTN_L) && !Play_InCsMode(play)) {
                    
                    if (!R_MINIMAP_DISABLED) {
                        Audio_PlaySfxGeneral(
                            NA_SE_SY_CAMERA_ZOOM_UP, &gSfxDefaultPos, 4,
                            &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                            &gSfxDefaultReverb
                        );
                    } else {
                        Audio_PlaySfxGeneral(
                            NA_SE_SY_CAMERA_ZOOM_DOWN, &gSfxDefaultPos, 4,
                            &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                            &gSfxDefaultReverb
                        );
                    }
                    
                    R_MINIMAP_DISABLED ^= 1;
                }
                
                break;
        }
    }
    
    CLOSE_DISPS(play->state.gfxCtx, "../z_map_exp.c", 782);
}
