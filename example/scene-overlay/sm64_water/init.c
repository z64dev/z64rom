//
// SM64 Clockwise/Counter Clockwise water
// Used in all cases of water throught Super Mario 64
//

#include "global.h"
#include "room_0.h"
#include <uLib.h>

// function name must be SceneFunc()
void SceneFunc(PlayState* play)
{
    static_UvRotateConfig(moatWater, MoatWater_Vtx, 4, 20);
    static_UvRotateConfig(lakeWater, Water_Vtx, 4, -15);
    
    UvRotate(play, &moatWater);
    UvRotate(play, &lakeWater);
}