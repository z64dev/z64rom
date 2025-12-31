#include <z64hdr.h>
#include "uLib.h"

void UvRotate(PlayState *play, UvRotateConfig *config)
{
    u32 gameplayFrames = play->gameplayFrames;
    
    if (!config->data)
    {
        config->data = SEGMENTED_TO_VIRTUAL(config->segAddr);
        
        for (int i = 0; i < config->numVerts; ++i)
            memcpy(&config->uvData[i], config->data[i].v.tc, sizeof(Vec2s));
    }
    
    for (int i = 0; i < config->numVerts; ++i)
    {
        const Vec2s *src = &config->uvData[i];
              Vec2s *dst = (Vec2s*)(config->data[i].v.tc);

        // Angle advances every frame
        s16 angle = gameplayFrames * config->speed;

        // OoT trig returns values in s16 fixed-point (1.0 = 0x7FFF)
        s16 sinA = sins(angle);
        s16 cosA = coss(angle);

        // Original texture coordinates
        s32 u = src->x;
        s32 v = src->y;

        // Clockwise rotation around (0, 0)
        // u' =  u*cosθ + v*sinθ
        // v' = -u*sinθ + v*cosθ
        s32 uRot = ( u * cosA + v * sinA) >> 15;
        s32 vRot = (-u * sinA + v * cosA) >> 15;

        dst->x = (s16)uRot;
        dst->y = (s16)vRot;
    }
}

Gfx* Gfx_QuickTwoTexScroll(PlayState* play, TextureScroll* params) {
    u32 frames = play->gameplayFrames;

    return Gfx_TwoTexScroll(play->state.gfxCtx, 0, params[0].xSpeed * frames, -(params[0].ySpeed * frames),
                            params[0].width, params[0].height, 1, params[1].xSpeed * frames,
                            -(params[1].ySpeed * frames), params[1].width, params[1].height);
}