//
// MM Texture Scroll
// Easy to use helper written by /zel/
//

#include "global.h"
#include <uLib.h>

// function name must be SceneFunc()
void SceneFunc(PlayState* play)
{
    // Transparent waterfall texture scroll
    // xSpeed, ySpeed, width, height
    static TextureScroll WaterfallTexScroll[] = {
        { -10, 0, 64, 64 },
        { 0, 0, 64, 64 },
    };
    gSPSegment(POLY_XLU_DISP++, 0x0A, Gfx_QuickTwoTexScroll(play, WaterfallTexScroll));
}