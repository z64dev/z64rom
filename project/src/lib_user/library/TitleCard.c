#include <uLib.h>
#include <code/game.h>
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

u64 gSceneTitleCardGradientTex[] = {
    0xfcfbf9f8f6f5f3f1, 0xefedebe9e7e5e3e0, 0xdedbd9d6d4d1cfcc, 0xc9c7c4c1bebbb9b6, 0xb3b0adaaa7a4a29f, 0x9c999694918e8b89, 0x8684817f766b6055, 0x493e33281e150d06, 
};

////////////////////////////////////////////////////////////////////////////
//
// prefix data and general how-to
//
//

// select a gradient color by adding \x01\xYY to your titlecard.txt prefix,
// where YY represents a value like 00 for mm, 01 for oot, 02 for orange, etc
// e.g. if your titlecard.txt contains \\\x01\x02\\Hello World, you get
// the text 'Hello World' on an orange gradient
static u32 presetGradientColors[] = {
    0x8c28a0ff, // 00 = mm titlecard color, default
    0x3250e6ff, // 01 = blue titlecard color, in the spirit of oot
    0xff821eff, // 02 = orange, as an example and b/c orange is nice
    // z64rom users can add extra colors here
};
// select a text color by adding \x03\xYY to your titlecard.txt prefix,
// where YY represents a value like 00 for white, 01 for black, etc
static u32 presetTextColors[] = {
    0xffffffff, // 00 = white, default
    0x000000ff, // 01 = black
    0xff821eff, // 02 = orange, as an example and b/c orange is nice
    // z64rom users can add extra colors here
};

// totally custom gradient colors can be used by adding
// \x02\xRR\xGG\xBB\xAA to your titlecard.txt prefix, where
// RR GG BB AA represent the RGBA of the gradient color you want
// e.g. \x02\xff\x00\x00\xff for a very bright red gradient

// similarly, totally custom text colors can be used by adding
// \x04\xRR\xGG\xBB\xAA to your titlecard.txt prefix, where
// RR GG BB AA represent the RGBA of the text color you want
// e.g. \x04\xff\x00\x00\xff for a very bright red text color

/*
 * quick titlecard prefix command reference
 *
 *  \x01 - preset gradient color
 *  \x02 - custom gradient color
 *  \x03 - preset text color
 *  \x04 - custom text color
 *  \x05 - play a sound
 */

/*
 * lastly, these prefix commands can be combined
 * for example:
 *  - prefix start          \\
//  - builtin orange text   \x03\x02
//  - custom red gradient   \x02\xff\x00\x00\xff
//  - mooing cow sound      \x05\x28\xdf
//  - prefix end            \\
//  - text to display       Hello World
// result: \\\x03\x02\x02\xff\x00\x00\xff\x05\x28\xdf\\Hello World
*/

//
//
////////////////////////////////////////////////////////////////////////////

static struct
{
	u32 gradientColor;
	u32 textColor;
	u16 printer; // frames elapsed since init
	u16 sfxId;
	u8 length; // length of source string
	u8 oldIndex;
	u8 oldChar;
	bool isText;
} titleCardText;


Asm_VanillaHook(TitleCard_InitPlaceName);
void TitleCard_InitPlaceName(PlayState* play, TitleCardContext* titleCtx, void* texture, s32 x, s32 y, s32 width,
                             s32 height, s32 delay) {
    SceneTableEntry* loadedScene = play->loadedScene;
    u32 size = loadedScene->titleFile.vromEnd - loadedScene->titleFile.vromStart;
    
    if ((size != 0) && (size <= 0x3000)) {
        DmaMgr_SendRequest1(texture, loadedScene->titleFile.vromStart, size, "", 0);
    }
    
    titleCtx->texture = texture;
    titleCtx->x = x;
    titleCtx->y = y;
    titleCtx->width = width;
    titleCtx->height = height;
    titleCtx->durationTimer = 80;
    titleCtx->delayTimer = delay;
    titleCardText.isText = size < 256 && size != 0;
    
    if (titleCardText.isText && texture != NULL)
    {
        // prepare titlecard vars
        u8 *tex = texture;
        int length;
        
        titleCardText.gradientColor = presetGradientColors[0];
        titleCardText.textColor = presetTextColors[0];
        titleCardText.sfxId = 0xffff;
        
        /* handle prefixes for extended features like colors, sounds, etc
           - starts and ends with a backslash (\), allowing any value between
           - in the titlecard.txt, that could look like this: \\\x01\x00\\
        */
        if (tex[0] == '\\')
        {
            int prefixBytes = 1;
            
            for (int i = 1; tex[i] != '\\'; prefixBytes = ++i)
            {
                switch (tex[i])
                {
                    case 0x01: // use preset gradient color from array
                        titleCardText.gradientColor = presetGradientColors[tex[i + 1]];
                        i += 1;
                        break;
                    
                    case 0x02: // custom gradient color
                        titleCardText.gradientColor =
                            (tex[i + 1] << 24)
                            | (tex[i + 2] << 16)
                            | (tex[i + 3] << 8)
                            | (tex[i + 4] << 0);
                        i += 4;
                        break;
                    
                    case 0x03: // use preset text color from array
                        titleCardText.textColor = presetTextColors[tex[i + 1]];
                        i += 1;
                        break;
                    
                    case 0x04: // custom text color
                        titleCardText.textColor =
                            (tex[i + 1] << 24)
                            | (tex[i + 2] << 16)
                            | (tex[i + 3] << 8)
                            | (tex[i + 4] << 0);
                        i += 4;
                        break;
                    
                    case 0x05: // play sound
                        // sfxId is stored and played later, b/c playing
                        // during titlecard setup is not ideal (the game
                        // silences sound effects played on this frame)
                        titleCardText.sfxId = (tex[i + 1] << 8) | tex[i + 2];
                        i += 2;
                        break;
                }
            }
            
            // now erase the prefix
            // (this is safe; titleCtx->texture is not used for freeing the block it points to)
            tex += prefixBytes + 1;
            titleCtx->texture = tex;
        }
        
        // oldChar gets read *after* reading prefix b/c tex pointer may have moved
        titleCardText.printer = 0;
        titleCardText.oldIndex = 0;
        titleCardText.oldChar = tex[0];
        
        // get byte size of string (exluding zero terminator)
        for (length = 0; tex[length]; ++length)
        {
            switch (tex[length])
            {
                case '\x05': length += 4; break; // 05 rr gg bb aa
            }
        }
        titleCardText.length = length;
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
    
    if (titleCtx->alpha != 0 && titleCtx->texture != NULL)
    {
        OPEN_DISPS(play->state.gfxCtx, "", 0);
        
        // draw MM-style text titlecard
        if (titleCardText.isText)
        {
            u8 *tex = titleCtx->texture;
            Color_RGBA8 col = {titleCardText.textColor >> 24, titleCardText.textColor >> 16, titleCardText.textColor >> 8,(u8)titleCtx->alpha};
            u8 curIndex = -1;
            
            // draw the MM banner gradient across the screen
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
                
                textureLanguageOffset = width * height * gSaveContext.language;
                height = (width * height > 0x1000) ? 0x1000 / width : height;
                titleSecondY = titleYBoss + (height * 4);
                
                OVERLAY_DISP = Gfx_SetupDL_52NoCD(OVERLAY_DISP);
                
                gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                                  ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
                
                              
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, (u8)titleCtx->alpha);
                gDPSetEnvColor(OVERLAY_DISP++, titleCardText.gradientColor >> 24, titleCardText.gradientColor >> 16, titleCardText.gradientColor >> 8, titleCardText.gradientColor);
                
                gDPLoadTextureBlock(OVERLAY_DISP++, gSceneTitleCardGradientTex, G_IM_FMT_I, G_IM_SIZ_8b, 64, 1, 0, G_TX_NOMIRROR | G_TX_WRAP, 
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                gSPTextureRectangle(OVERLAY_DISP++, 0x1000, 0x00F0, (0 + gradtitleX), gradtitleY + (height + 80),
                                    G_TX_RENDERTILE, 0, 0, 204, 1 << 10);
                
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, (u8)titleCtx->intensity, (u8)titleCtx->intensity, (u8)titleCtx->intensity,
                                (u8)titleCtx->alpha);
                gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
                
                gDPLoadTextureBlock(OVERLAY_DISP++, (s32)titleCtx->texture + textureLanguageOffset, G_IM_FMT_IA, G_IM_SIZ_8b,
                                    width, height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                                    G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            }
            
            // frames elapsed since init
            // (this is used for timings besides printing, such as playing sfx)
            titleCardText.printer += 1;
            
            // typewriter effect (the text gets 'printed' to the screen letter by letter)
            // (done in-place by modifying a single string on the fly)
            // (change to if (false) if you don't want the typewriter effect)
            if (true)
            {
                // curIndex indicates the temporary end-of-string marker
                curIndex = titleCardText.printer / 1; // divide by 2 or 3 etc for slower printing if you want
                
                // restore the old string before the current frame
                tex[titleCardText.oldIndex] = titleCardText.oldChar;
                
                // back up the current string for restoring on the next frame
                titleCardText.oldChar = tex[curIndex];
                titleCardText.oldIndex = curIndex;
                
                // prematurely terminate the string here for one frame
                if (curIndex < titleCardText.length)
                    tex[curIndex] = '\0';
            }
            
            // play deferred sfx
            if (titleCardText.sfxId != 0xffff && titleCardText.printer == titleCardText.length)
            {
                func_80078884(titleCardText.sfxId);
                titleCardText.sfxId = 0xffff;
            }
            
            Gfx *gfx = Text_Begin();
            
            if (titleCardText.length < TITLECARD_TYPEWRITER_MINIMUM || curIndex >= TITLECARD_TYPEWRITER_MINIMUM)
                Text_DrawShadowColor(&gfx, tex, 12.0f, 30, 72, DRAW_ANCHOR_L, &play->msgCtx.font, &col);
            
            Text_Finish(gfx);
        }
        // draw OoT-style texture titlecard (is also used for boss fight titlecards)
        else
        {
            width = titleCtx->width;
            height = titleCtx->height;
            titleX = (titleCtx->x * 4) - (width * 2);
            titleY = (titleCtx->y * 4) - (height * 2);
            doubleWidth = width * 2;
            
            textureLanguageOffset = width * height * gSaveContext.language;
            height = (width * height > 0x1000) ? 0x1000 / width : height;
            titleSecondY = titleY + (height * 4);
            
            OVERLAY_DISP = Gfx_SetupDL_52NoCD(OVERLAY_DISP);
            
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, (u8)titleCtx->intensity, (u8)titleCtx->intensity, (u8)titleCtx->intensity,
                            (u8)titleCtx->alpha);
            
            gDPLoadTextureBlock(OVERLAY_DISP++, (s32)titleCtx->texture + textureLanguageOffset, G_IM_FMT_IA, G_IM_SIZ_8b,
                                width, height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                                G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            
            gSPTextureRectangle(OVERLAY_DISP++, titleX, titleY, ((doubleWidth * 2) + titleX) - 4, titleY + (height * 4) - 1,
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            
            height = titleCtx->height - height;
            
            // If texture is bigger than 0x1000, display the rest
            if (height > 0) {
                gDPLoadTextureBlock(OVERLAY_DISP++, (s32)titleCtx->texture + textureLanguageOffset + 0x1000, G_IM_FMT_IA,
                                    G_IM_SIZ_8b, width, height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                                    G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                
                gSPTextureRectangle(OVERLAY_DISP++, titleX, titleSecondY, ((doubleWidth * 2) + titleX) - 4,
                                    titleSecondY + (height * 4) - 1, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            }
        }
        
        CLOSE_DISPS(play->state.gfxCtx, "", 0);
    }
}

#endif

