#ifndef __ULIB_H__
#define __ULIB_H__

// # # # # # # # # # # # # # # # # # # # #
// # User Library Patches                #
// # # # # # # # # # # # # # # # # # # # #

// New save patches
#define Patch_SaveStartEntrance 0xBB
#define Patch_SaveStartAge      LINK_AGE_CHILD
#define Patch_SaveStartTime     0x6AAB
#define Patch_SaveStartCsIndex  0xFFF1

#define OOT 0
#define MM  1
#define GC  2

// Wield Hylian shield like Deku shield
#define Patch_WieldHylianShieldLikeKokiriShield true

// Play cutscene after obtaining Silver Gauntlets
#define Patch_SilverGauntletObtainCS false

// Flush current textbox by pressing B
#define Patch_QuickText true

// MM Timer (faked)
#define Patch_MM_TIMER false

// When Bunny Hood is equipped, you run fast like in MM
#define MM_BUNNYHOOD true

// Save anywhere but a grotto
#define SAVE_ANYWHERE true

// Use MM style titlecards
#define MM_TITLECARD false

// Enable motion blur + motion blur cutscene command
#define MOTION_BLUR true

// Interface Stuff
// MM-Styled Interface (Corrdinates)
#define Patch_MM_INTERFACE_BUTTONS_CORDS true

// MM-Styled Interface (Shadows)
#define Patch_MM_INTERFACE_SHADOWS true

// Interface Colors for A and B buttons (OOT/MM/GC)
#define Patch_INTERFACE_BUTTON_COLORS MM

// Interface Colors for C buttons (OOT/MM)
#define Patch_INTERFACE_C_BUTTON_COLORS_MM true

// Interface TATL height (c-up, requires texture patch)
#define Patch_INTERFACE_C_UP_TATL true

// Interface Colors for the Rupee counter, according to wallet upgrades 
#define Patch_MM_INTERFACE_RUPEE_UPGRADES true

// Space allocated for GI models, expand if needed
#define Patch_GetItem_SegmentSize 0x3008

// Extension, these can be adjusted if necessary
#define EXT_DMA_MAX    3800
#define EXT_ACTOR_MAX  1000
#define EXT_OBJECT_MAX 1000
#define EXT_SCENE_MAX  256
#define EXT_EFFECT_MAX 64

// # # # # # # # # # # # # # # # # # # # #
// # UserLibrary                         #
// # # # # # # # # # # # # # # # # # # # #

#include <z64hdr.h>

#include "uLib_macros.h"
#include "uLib_types.h"
#include "sfx_enum.h"

extern DmaEntry __ext_gDmaDataTable[EXT_DMA_MAX];
extern ActorOverlay __ext_gActorOverlayTable[EXT_ACTOR_MAX];
extern RomFile __ext_gObjectTable[EXT_OBJECT_MAX];
extern SceneTableEntry __ext_gSceneTable[EXT_SCENE_MAX];
extern EffectSsOverlay __ext_gEffectSsOverlayTable[EXT_EFFECT_MAX];
extern u8 gFontOrdering[];
extern Vec3f gZeroVec;

#ifndef NO_Z64ROM_EXT_TABLES
#define gDmaDataTable         __ext_gDmaDataTable
#define gActorOverlayTable    __ext_gActorOverlayTable
#define gObjectTable          __ext_gObjectTable
#define gSceneTable           __ext_gSceneTable
#define gEffectSsOverlayTable __ext_gEffectSsOverlayTable
#endif

extern u32 osMemSize;
extern PlayState gPlayState;
extern LibContext gLibCtx;
extern GraphicsContext* __gfxCtx;
extern ExitParam gExitParam;
extern f32 EPSILON;
asm ("gPlayState = 0x80212020 - 0x38000;");
asm ("__gfxCtx = gPlayState;");
asm ("gExitParam = gPlayState + 0x11E18;"); // 801EBE38
asm ("osMemSize = 0x80000318");

void uLib_Update(GameState* gameState);
void* memset(void* m, int v, unsigned int s);
f32 fmodf(f32, f32);

extern char gBlankString[1];
asm("gBlankString = 0x8012602C;");

#define DEFAULT_REVERB 0.35f
#define DEFAULT_RANGE  800.0f

void SoundEffect_PlayOneshot(SoundFile id, f32 vol, f32 pitch, Vec3f* pos, f32 reverb, f32 range, void* origin);
void SoundEffect_PlayHeld(SoundFile id, f32 vol, f32 pitch, f32 stepRelease, Vec3f* pos, f32 reverb, f32 range, void* origin);
void SoundEffect_PlayDefaultOneshot(SoundFile id, Vec3f* pos, void* origin);
void SoundEffect_PlayDefaultHeld(SoundFile id, f32 stepRelease, Vec3f* pos, void* origin);
void SoundEffect_StopOneshot(SoundFile id, Vec3f* pos, void* origin);
typedef struct SoundEntity SoundEntity;
SoundEntity* SoundEntity_Register(void* origin);
void SoundEntity_Unregister(SoundEntity* sen);
void SoundEntity_SetFilter(SoundEntity* sen, s8 filter);

#ifdef DEV_BUILD

#define Assert(cond)  if (!(cond)) { char buffer[82]; sprintf(buffer, "%s\nline: %d", __FILE__, __LINE__); Fault_AddHungupAndCrashImpl("Assert("#cond ");", buffer); }
#define osInfo(title) "" PRNT_GRAY "[" PRNT_REDD "%s" PRNT_GRAY "::" PRNT_YELW "%d" PRNT_GRAY "]" PRNT_RSET ": " PRNT_REDD title, __FUNCTION__, __LINE__

void Debug_Text(u8 r, u8 g, u8 b, s32 x, s32 y, char* fmt, ...);
void Debug_DmaLog(DmaRequest* req);
void DebugMenu_Update(PlayState* playState);
s32 DebugMenu_CineCamera(Camera* camera, Normal1* norm1, Player* player);
void Profiler_Start(DebugProfiler* profiler);
void Profiler_End(DebugProfiler* profiler);
void osLibPrintf(const char* fmt, ...);
void osLibHex(const char* txt, const void* data, u32 size, u32 dispOffset);

#else /* RELEASE_BUILD */

#define Debug_Text(...)           do {} while (0)
#define Debug_DmaLog(...)         do {} while (0)
#define DebugMenu_Update(...)     do {} while (0)
#define DebugMenu_CineCamera(...) do {} while (0)
#define Profiler_Start(...)       do {} while (0)
#define Profiler_End(...)         do {} while (0)
#define Assert(cond)              do {} while (0)
#define osInfo(title)             do {} while (0)
#define osLibPrintf(...)          do {} while (0)
#define osLibHex(...)             do {} while (0)
#endif

void Audio_PlaySys(u16 flag);
Color_HSL Color_RgbToHsl(f32 r, f32 g, f32 b);
Color_RGB8 Color_HslToRgb(f32 h, f32 s, f32 l);
void Cutscene_PlaySegment(PlayState* play, void* segment);
Vtx* Gfx_AllocQuad(PlayState* playState, s16 x, s16 y, s16 width, s16 height, s16 u, s16 v);
s32 Lights_SortLightList(PlayState* play, LightInfo* sortedLightList[7]);
void Lights_SetPointlight(PlayState* play, Lights* lights, LightParams* params, bool isWiiVC);
void Lights_RebindActor(PlayState* play, Actor* actor, Vec3f* bindPos);
void Lights_RebindPointlightsActor(PlayState* play, Actor* actor, bool isWiiVC);

f32 MaxF(f32 a, f32 b);
f32 MinF(f32 a, f32 b);
s32 MaxS(s32 a, s32 b);
s32 MinS(s32 a, s32 b);
s32 WrapS(s32 x, s32 min, s32 max);
f32 WrapF(f32 x, f32 min, f32 max);
f32 Math_Spline1(f32 k, f32 xm1, f32 x0, f32 x1, f32 x2);
f32 Math_Spline2(f32 k, f32 xm1, f32 x0, f32 x1, f32 x2);
Vec3f Math_Vec3f_Spline1(f32 k, Vec3f xm1, Vec3f x0, Vec3f x1, Vec3f x2);
Vec3f Math_Vec3f_Spline2(f32 k, Vec3f xm1, Vec3f x0, Vec3f x1, Vec3f x2);
Vec3f Math_Vec3f_YawDist(f32 dist, s16 yaw);
Vec3f Math_Vec3f_YawPitchDist(f32 dist, s16 yaw, s16 pitch);
Vec3f Math_Vec3f_PosRelativeTo(Vec3f* target, Vec3f* origin, s16 originYaw);
f32 Math_Vec3f_Length(Vec3f* a);
f32 PowF(f32 a, f32 b);

void Matrix_RotateX_s(s16 binang, MatrixMode mtxMode);
void Matrix_RotateY_s(s16 binang, MatrixMode mtxMode);
void Matrix_RotateZ_s(s16 binang, MatrixMode mtxMode);
void Matrix_RotateX_f(f32 deg, MatrixMode mtxMode);
void Matrix_RotateY_f(f32 deg, MatrixMode mtxMode);
void Matrix_RotateZ_f(f32 deg, MatrixMode mtxMode);
void Matrix_MultX(f32 x, Vec3f* dst);
void Matrix_MultY(f32 y, Vec3f* dst);
void Matrix_MultZ(f32 z, Vec3f* dst);

s32 Actor_FocusPlayer(PlayState* play, Actor* this, Vec3s* headVector, f32 dist);

Particle Particle_New(Vec3f pos, f32 mass);
void Particle_Update(Particle* particle, f32 gravity, Vec3f addForce, f32 c);
Chain Chain_New(Particle* p1, Particle* p2, f32 length);
void Chain_UpdateStep(Chain* chain, f32 step, f32 max);
void Chain_UpdateAverage(Chain* chain);
void Chain_Update(Chain* chain);

void* Segment_Scene_GetHeader(void* segment, s32 setupIndex);
void* Segment_Scene_GetCutscene(void* segment, s32 setupIndex);
CollisionHeader* Segment_Scene_GetCollision(void* segment, s32 setupIndex);
void Segment_Scene_PlayCutscene(void* segment, s32 setupIndex);
u8 PathList_GetNum(u8 index, s8 header);
void* PathList_GetList(u8 index, s8 header);

void SceneAnim_Update(PlayState* playState);

void SkelAnime_InterpFrameTable_Quat(s32 limbCount, Vec3s* dst, Vec3s* start, Vec3s* target, f32 weight);

void Play_SetFadeOut(PlayState* play);
struct Time Play_GetTime(void);
void NewRoom_Draw(PlayState* play, Room* room, u32 flags);

// please read the examples and 'important things to know' sections
// of ActorPayload.c before using Actor_GetPayload() in your actors
void *(Actor_GetPayload)(Actor *actor, PlayState *play);
void *Actor_GetPayloadAndDoRelocs(Actor *actor, PlayState *play, u8 *relocs);

// i wish there was a better way to do this sort of thing in c
#define Actor_GetPayload_ARRAY(...) (u8[]){__VA_ARGS__, 0xff}
#define Actor_GetPayload_EXPAND(x) x
#define Actor_GetPayload_DICT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, NAME, ...) NAME
#define Actor_GetPayload2(ACTOR, PLAY)                                                                               Actor_GetPayload(ACTOR, PLAY)
#define Actor_GetPayload3(ACTOR, PLAY, A0)                                                                           Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4))
#define Actor_GetPayload4(ACTOR, PLAY, A0, A1)                                                                       Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4))
#define Actor_GetPayload5(ACTOR, PLAY, A0, A1, A2)                                                                   Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4))
#define Actor_GetPayload6(ACTOR, PLAY, A0, A1, A2, A3)                                                               Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4))
#define Actor_GetPayload7(ACTOR, PLAY, A0, A1, A2, A3, A4)                                                           Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4))
#define Actor_GetPayload8(ACTOR, PLAY, A0, A1, A2, A3, A4, A5)                                                       Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4))
#define Actor_GetPayload9(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6)                                                   Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4))
#define Actor_GetPayload10(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7)                                              Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4))
#define Actor_GetPayload11(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7, A8)                                          Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4, A8/4))
#define Actor_GetPayload12(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)                                      Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4, A8/4, A9/4))
#define Actor_GetPayload13(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)                                 Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4, A8/4, A9/4, A10/4))
#define Actor_GetPayload14(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)                            Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4, A8/4, A9/4, A10/4, A11/4))
#define Actor_GetPayload15(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)                       Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4, A8/4, A9/4, A10/4, A11/4, A12/4))
#define Actor_GetPayload16(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13)                  Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4, A8/4, A9/4, A10/4, A11/4, A12/4, A13/4))
#define Actor_GetPayload17(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14)             Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4, A8/4, A9/4, A10/4, A11/4, A12/4, A13/4, A14/4))
#define Actor_GetPayload18(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15)        Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4, A8/4, A9/4, A10/4, A11/4, A12/4, A13/4, A14/4, A15/4))
#define Actor_GetPayload19(ACTOR, PLAY, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16)   Actor_GetPayloadAndDoRelocs(ACTOR, PLAY, Actor_GetPayload_ARRAY(A0/4, A1/4, A2/4, A3/4, A4/4, A5/4, A6/4, A7/4, A8/4, A9/4, A10/4, A11/4, A12/4, A13/4, A14/4, A15/4, A16/4))
#define Actor_GetPayload(...)    Actor_GetPayload_EXPAND( Actor_GetPayload_DICT(__VA_ARGS__, Actor_GetPayload19, Actor_GetPayload18, Actor_GetPayload17, Actor_GetPayload16, Actor_GetPayload15, Actor_GetPayload14, Actor_GetPayload13, Actor_GetPayload12, Actor_GetPayload11, Actor_GetPayload10, Actor_GetPayload9, Actor_GetPayload8, Actor_GetPayload7, Actor_GetPayload6, Actor_GetPayload5, Actor_GetPayload4, Actor_GetPayload3, Actor_GetPayload2)(__VA_ARGS__) )

#define EASYTALK_OPENED      100   // opened textbox
#define EASYTALK_CLOSED      200   // closed textbox
#define EASYTALK_CHOICE_1    1     // made choice 1, 2, or 3
#define EASYTALK_CHOICE_2    2
#define EASYTALK_CHOICE_3    3
#define EASYTALK_NO_EVENT    0     // no event has taken place

// feed this distance into EasyTalkNpc() to activate the textbox instantly
#define EASYTALK_DISTANCE_ACTIVATE_INSTANTLY -123456.0f

void EasyTalkOverrideString(PlayState *play, const char *string);
void EasyTalkFlush(PlayState *play);
int (EasyTalkNpc)(Actor *actor, PlayState *play, const EasyTalkNpcArgs *args);
void EasyTalkQueueOverrideString(const char *text);
void EasyTalkApplyQueuedNaviActorDescription(void);
void EasyTalkSetNaviActorDescriptionString(Actor *actor, PlayState *play, const char *text);
#define EasyTalkNpc(ACTOR, PLAY, ...) \
	(EasyTalkNpc)(ACTOR, PLAY, &(EasyTalkNpcArgs) { \
		EasyTalkNpcArgsDefaults __VA_ARGS__ \
	})
_Pragma("GCC diagnostic ignored \"-Woverride-init\"") // for functions w/ optional arguments

typedef enum {
    OVL_MSG_TALK,
    OVL_MSG_CHECK,
} OvlMsgType;

typedef enum {
    MSGBOX_TYPE_BLACK,
    MSGBOX_TYPE_WOODEN          =  1 << 4,
    MSGBOX_TYPE_BLUE            =  2 << 4,
    MSGBOX_TYPE_OCARINA         =  3 << 4,
    MSGBOX_TYPE_NONE_BOTTOM     =  4 << 4,
    MSGBOX_TYPE_NONE_NO_SHADOW  =  5 << 4,
    MSGBOX_TYPE_CREDITS         = 11 << 4
} MsgBoxType;

typedef enum {
    MSGBOX_POS_VARIABLE,
    MSGBOX_POS_TOP,
    MSGBOX_POS_MIDDLE,
    MSGBOX_POS_BOTTOM
} MsgBoxPos;

#define MSG_COLOR_DEFAULT   "\x05\x00"
#define MSG_COLOR_RED       "\x05\x01"
#define MSG_COLOR_GREEN     "\x05\x02"
#define MSG_COLOR_BLUE      "\x05\x03"
#define MSG_COLOR_LIGHTBLUE "\x05\x04"
#define MSG_COLOR_PURPLE    "\x05\x05"
#define MSG_COLOR_YELLOW    "\x05\x06"
#define MSG_COLOR_BLACK     "\x05\x07"
#define MSG_COLOR_REG_0     "\x05\x08"
#define MSG_COLOR_REG_1     "\x05\x09"
#define MSG_COLOR_REG_2     "\x05\x0A"
#define MSG_COLOR_REG_3     "\x05\x0B"
#define MSG_COLOR_REG_4     "\x05\x0C"
#define MSG_COLOR_REG_5     "\x05\x0D"
#define MSG_COLOR_REG_6     "\x05\x0E"
#define MSG_COLOR_REG_7     "\x05\x0F"

#define MSG_SHIFT(x)             "\x06" x // 1
#define MSG_BOX_BREAK_DELAYED(x) "\x0C" x // 1
#define MSG_FADE(x)              "\x0E" x // 1
#define MSG_FADE2(x, y)          "\x11" x y // 2
#define MSG_SFX(x, y)            "\x12" x y // 2
#define MSG_ITEM_ICON(x)         "\x13" x // 1
#define MSG_TEXT_SPEED(x)        "\x14" x // 1
#define MSG_HIGHSCORE(x)         "\x1E" x // 1

#define MSG_END           "\x02"
#define MSG_INSTANT_ON    "\x08"
#define MSG_INSTANT_OFF   "\x09"
#define MSG_PERSISTENT    "\xFA"
#define MSG_EVENT         "\x0B"
#define MSG_NAME          "\x0F"
#define MSG_OCARINA       "\x10"
#define MSG_MARATHON_TIME "\x16"
#define MSG_RACE_TIME     "\x17"
#define MSG_POINTS        "\x18"
#define MSG_TOKENS        "\x19"
#define MSG_TWO_CHOICE    "\x1B"
#define MSG_THREE_CHOICE  "\x1C"
#define MSG_FISH_INFO     "\x1D"
#define MSG_TIME          "\x1F"

typedef struct OvlMessage {
    MsgBoxType type;
    MsgBoxPos  pos;
    char*      txt;
} OvlMessage;

void OvlMessage_Init(Actor* actor, OvlMsgType type);
void OvlMessage_RegisterColor(u8 id, u8 r, u8 g, u8 b);
s8 OvlMessage_Prompt(Actor* actor, OvlMessage* msg, f32 radius, u32 exchangeItemId);
void OvlMessage_Start(Actor* actor, OvlMessage* msg);
void OvlMessage_Continue(Actor* actor, OvlMessage* msg);
s8 OvlMessage_SwapFocus(Actor* actor);
s8 OvlMessage_IsClosed(Actor* actor);
s8 OvlMessage_IsCurrentMessage(OvlMessage* msg);
s8 OvlMessage_SetBranch(OvlMessage* branch);
s8 OvlMessage_GetChoice(Actor* actor);
s8 OvlMessage_GetBoxNum(void);

//Draw helpers
#define DRAW_ANCHOR_C 0
#define DRAW_ANCHOR_U (1 << 0)
#define DRAW_ANCHOR_R (1 << 1)
#define DRAW_ANCHOR_D (1 << 2)
#define DRAW_ANCHOR_L (1 << 3)
#define DRAW_ANCHOR_UR DRAW_ANCHOR_U | DRAW_ANCHOR_R
#define DRAW_ANCHOR_DR DRAW_ANCHOR_D | DRAW_ANCHOR_R
#define DRAW_ANCHOR_DL DRAW_ANCHOR_D | DRAW_ANCHOR_L
#define DRAW_ANCHOR_UL DRAW_ANCHOR_U | DRAW_ANCHOR_L
void Text_DrawShadowColor(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf, Color_RGBA8* color);
void Text_Finish(Gfx* gfx);

#if MOTION_BLUR
    #define motionBlurAlpha unk_12428[0]
#endif

#endif
