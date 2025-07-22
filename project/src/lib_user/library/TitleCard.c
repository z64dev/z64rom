#include <uLib.h>
#include <code/game.h>
#define SCANLINE_HEIGHT 1
//#include "scene_names.h"

/*
   z64ram = 0x8002CFAC
   z64rom = 0xAA414C

 */
 


// (MM starts displaying letters only after there are at least 10 to display)
// (change from 0 to 10 if you want your titlecard to behave like MM)
#define TITLECARD_TYPEWRITER_MINIMUM 0

typedef struct {
    Actor actor;
} Entity;

#if MM_TITLECARD

extern void Text_DrawShadowColor(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf,
                          Color_RGBA8* color);
extern void Text_Finish(Gfx* gfx);

u64 gSceneTitleCardGradientTex[] = {
    0xfcfbf9f8f6f5f3f1, 0xefedebe9e7e5e3e0, 0xdedbd9d6d4d1cfcc, 0xc9c7c4c1bebbb9b6, 0xb3b0adaaa7a4a29f, 0x9c999694918e8b89, 0x8684817f766b6055, 0x493e33281e150d06, 
};
static u8 length; // length of source string
static u8 printer;
static u8 oldIndex;
static u8 oldChar;


Asm_VanillaHook(TitleCard_InitPlaceName);
void TitleCard_InitPlaceName(PlayState* play, TitleCardContext* titleCtx, void* texture, s32 x, s32 y, s32 width,
                             s32 height, s32 delay) {
    SceneTableEntry* loadedScene = play->loadedScene;
    u32 size = loadedScene->titleFile.vromEnd - loadedScene->titleFile.vromStart;

    if ((size != 0) && (size <= 0x3000)) {
        DmaMgr_SendRequest1(texture, loadedScene->titleFile.vromStart, size, "../z_actor.c", 2765);
    }

    titleCtx->texture = texture;
    titleCtx->x = x;
    titleCtx->y = y;
    titleCtx->width = width;
    titleCtx->height = height;
    titleCtx->durationTimer = 80;
    titleCtx->delayTimer = delay;

    if (titleCtx->texture != NULL)
    {
	//prepare titlecard vars
    u8 *tex = titleCtx->texture;
	printer = 0;
	oldIndex = 0;
	oldChar = tex[0];
	tex = titleCtx->texture;
	// get byte size of string (exluding zero terminator)
	for (length = 0; tex[length]; ++length)
	{
	    switch (tex[length])
	    {
	        case '\x05': length += 4; break; // 05 rr gg bb aa
	    }
	}
	}
}


Asm_VanillaHook(TitleCard_Draw);
void TitleCard_Draw(PlayState* play, TitleCardContext* titleCtx, Gfx** gfxp) {
    s32 width;
    s32 height;
    s32 titleX;
    s32 titleXBoss;
    s32 doubleWidth;
    s32 titleY;
    s32 titleYBoss;
    s32 titleSecondY;
    s32 textureLanguageOffset;
    s32 gradtitleX;
    s32 gradtitleY;
    
    if (titleCtx->alpha != 0)
    {
        width = titleCtx->width;
        height = titleCtx->height;
        titleX = (titleCtx->x + 180) - (width * 2 + 23);
        titleY = (titleCtx->y + 184) - (height * 2);
        titleXBoss = (titleCtx->x * 4) - (width * 2);
        titleYBoss = (titleCtx->y * 4) - (height * 2);
        doubleWidth = width * 2;
        gradtitleX = (titleCtx->x + 140) - (width * 2 + 13);
        gradtitleY = (titleCtx->y + 174) - (height * 2);

        OPEN_DISPS(play->state.gfxCtx, "../z_actor.c", 2824);
        
        textureLanguageOffset = width * height * gSaveContext.language;
        height = (width * height > 0x1000) ? 0x1000 / width : height;
        titleSecondY = titleYBoss + (height * 4);
        
        OVERLAY_DISP = Gfx_SetupDL_52NoCD(OVERLAY_DISP);

        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);            

                      
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, (u8)titleCtx->alpha);
    #if USE_MM_GRADCOLOR == true
        gDPSetEnvColor(OVERLAY_DISP++, 140, 40, 160, 255);
    #else
        gDPSetEnvColor(OVERLAY_DISP++, 50, 80, 230, 255);
    #endif

                        
        gDPLoadTextureBlock(OVERLAY_DISP++, gSceneTitleCardGradientTex, G_IM_FMT_I, G_IM_SIZ_8b, 64, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, 
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                        
        switch (play->sceneId)
        {
            case SCENE_YDAN_BOSS:
            case SCENE_DDAN_BOSS:
            case SCENE_BDAN_BOSS:
            case SCENE_MORIBOSSROOM:
            case SCENE_FIRE_BS:
            case SCENE_MIZUSIN_BS:
            case SCENE_HAKADAN_BS:
            case SCENE_JYASINBOSS:
            case SCENE_GANON_BOSS:
            case SCENE_GANON_DEMO:
            case 0x00DF:
            case 0x00F3:
                gSPTextureRectangle(OVERLAY_DISP++, 0x0000, 0x0000, 0, 0,
                                    G_TX_RENDERTILE, 0, 0, 0, 0);
                break;
            
            default:
                gSPTextureRectangle(OVERLAY_DISP++, 0x1000, 0x00F0, (0 + gradtitleX), gradtitleY + (height + 80),
                                    G_TX_RENDERTILE, 0, 0, 204, 1 << 10);
            	break;
        }
        
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, (u8)titleCtx->intensity, (u8)titleCtx->intensity, (u8)titleCtx->intensity,
                        (u8)titleCtx->alpha);
        gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
        
        gDPLoadTextureBlock(OVERLAY_DISP++, (s32)titleCtx->texture + textureLanguageOffset, G_IM_FMT_IA, G_IM_SIZ_8b,
                            width, height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                            G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                    
        Gfx* gfx = Text_Begin();
        Color_RGBA8 col = {255,255,255,(u8)titleCtx->alpha};
        switch (play->sceneId)
        {
            case SCENE_YDAN_BOSS:
            case SCENE_DDAN_BOSS:
            case SCENE_BDAN_BOSS:
            case SCENE_MORIBOSSROOM:
            case SCENE_FIRE_BS:
            case SCENE_MIZUSIN_BS:
            case SCENE_HAKADAN_BS:
            case SCENE_JYASINBOSS:
            case SCENE_GANON_BOSS:
            case SCENE_GANON_DEMO:
            case 0x00DF:
            case 0x00F3:
                gSPTextureRectangle(OVERLAY_DISP++, titleXBoss, titleYBoss, ((doubleWidth * 2) + titleXBoss) - 4, titleYBoss + (height * 4) - 1,
                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
                break;
            
            default:
                if (titleCtx->texture != NULL)
                {
                    u8 *tex = titleCtx->texture;

                    u8 curIndex = -1;
                    
                    // typewriter effect (the text gets 'printed' to the screen letter by letter)
                    // (done in-place by modifying a single string on the fly)
                    if (true)
                    {

                        
                        
                        // curIndex indicates the temporary end-of-string marker
                        curIndex = ++printer;
                        
                        // restore the old string before the current frame
                        tex[oldIndex] = oldChar;
                        
                        // back up the current string for restoring on the next frame
                        oldChar = tex[curIndex];
                        oldIndex = curIndex;
                        
                        // prematurely terminate the string here for one frame
                        if (curIndex < length)
                            tex[curIndex] = '\0';
                    }
                    
                    if (length < TITLECARD_TYPEWRITER_MINIMUM || curIndex >= TITLECARD_TYPEWRITER_MINIMUM)
                        Text_DrawShadowColor(&gfx, tex, 12.0f, 30, 72, DRAW_ANCHOR_L, &play->msgCtx.font, &col);
                }
                else
                {
                    gSPTextureRectangle(
                        OVERLAY_DISP++,
                        titleX,
                        titleY,
                        ((doubleWidth * 2 ) + titleX)
                        , titleY + (height * 3) + 7
                        , G_TX_RENDERTILE,
                        0,
                        0,
                        0x488,
                        0x4d5
                    );
                }
                break;
        }
        Text_Finish(gfx);
        height = titleCtx->height - height;

        // If texture is bigger than 0x1000, display the rest
        if (height > 0) {
            gDPLoadTextureBlock(OVERLAY_DISP++, (s32)titleCtx->texture + textureLanguageOffset + 0x1000, G_IM_FMT_IA,
                                G_IM_SIZ_8b, width, height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                                G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

            gSPTextureRectangle(OVERLAY_DISP++, titleXBoss, titleSecondY, ((doubleWidth * 2) + titleXBoss) - 4,
                                titleSecondY + (height * 4) - 1, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        }
        CLOSE_DISPS(play->state.gfxCtx, "../z_actor.c", 2880);
    }
}

#endif

