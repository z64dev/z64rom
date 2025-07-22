#include <uLib.h>
#include <code/z_rcp.h>

#ifndef DRAWSTRING_H
#define DRAWSTRING_H


#define MONO_DIST 0.9f

static s32 sStickTexIndex = 0;

extern u16 _gfxTextureLoadWidth;
extern u16 _gfxTextureLoadHeight;

#define _gfxTextureLoad_4b(pkt, timg, fmt, width, height, empty)                                                     \
    _gfxTextureLoadWidth = width;                                                                                    \
    _gfxTextureLoadHeight = height;                                                                                  \
    gDPLoadTextureBlock_4b(pkt++, timg, fmt, width, height, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, \
                           G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

Gfx* Gfx_TextureLoadI4(Gfx* head, void* texture, s16 width, s16 height) {
    _gfxTextureLoad_4b(head, texture, G_IM_FMT_I, width, height, );
    return head;
}

u16 _gfxTextureLoadWidth = 0;
u16 _gfxTextureLoadHeight = 0;

static f32 sFontWidths[144] = {
    8.0f,  // ' '
    8.0f,  // '!'
    6.0f,  // '"'
    9.0f,  // '#'
    9.0f,  // '$'
    14.0f, // '%'
    12.0f, // '&'
    3.0f,  // '''
    7.0f,  // '('
    7.0f,  // ')'
    7.0f,  // '*'
    9.0f,  // '+'
    4.0f,  // ','
    6.0f,  // '-'
    4.0f,  // '.'
    9.0f,  // '/'
    10.0f, // '0'
    5.0f,  // '1'
    9.0f,  // '2'
    9.0f,  // '3'
    10.0f, // '4'
    9.0f,  // '5'
    9.0f,  // '6'
    9.0f,  // '7'
    9.0f,  // '8'
    9.0f,  // '9'
    6.0f,  // ':'
    6.0f,  // ';'
    9.0f,  // '<'
    11.0f, // '='
    9.0f,  // '>'
    11.0f, // '?'
    13.0f, // '@'
    12.0f, // 'A'
    9.0f,  // 'B'
    11.0f, // 'C'
    11.0f, // 'D'
    8.0f,  // 'E'
    8.0f,  // 'F'
    12.0f, // 'G'
    10.0f, // 'H'
    4.0f,  // 'I'
    8.0f,  // 'J'
    10.0f, // 'K'
    8.0f,  // 'L'
    13.0f, // 'M'
    11.0f, // 'N'
    13.0f, // 'O'
    9.0f,  // 'P'
    13.0f, // 'Q'
    10.0f, // 'R'
    10.0f, // 'S'
    9.0f,  // 'T'
    10.0f, // 'U'
    11.0f, // 'V'
    15.0f, // 'W'
    11.0f, // 'X'
    10.0f, // 'Y'
    10.0f, // 'Z'
    7.0f,  // '['
    10.0f, // '\'
    7.0f,  // ']'
    10.0f, // '^'
    9.0f,  // '_'
    5.0f,  // '`'
    8.0f,  // 'a'
    9.0f,  // 'b'
    8.0f,  // 'c'
    9.0f,  // 'd'
    9.0f,  // 'e'
    6.0f,  // 'f'
    9.0f,  // 'g'
    8.0f,  // 'h'
    4.0f,  // 'i'
    6.0f,  // 'j'
    8.0f,  // 'k'
    4.0f,  // 'l'
    12.0f, // 'm'
    9.0f,  // 'n'
    9.0f,  // 'o'
    9.0f,  // 'p'
    9.0f,  // 'q'
    7.0f,  // 'r'
    8.0f,  // 's'
    7.0f,  // 't'
    8.0f,  // 'u'
    9.0f,  // 'v'
    12.0f, // 'w'
    8.0f,  // 'x'
    9.0f,  // 'y'
    8.0f,  // 'z'
    7.0f,  // '{'
    5.0f,  // '|'
    7.0f,  // '}'
    10.0f, // '~'
    10.0f, // '‾'
    12.0f, // 'À'
    6.0f,  // 'î'
    12.0f, // 'Â'
    12.0f, // 'Ä'
    11.0f, // 'Ç'
    8.0f,  // 'È'
    8.0f,  // 'É'
    8.0f,  // 'Ê'
    6.0f,  // 'Ë'
    6.0f,  // 'Ï'
    13.0f, // 'Ô'
    13.0f, // 'Ö'
    10.0f, // 'Ù'
    10.0f, // 'Û'
    10.0f, // 'Ü'
    9.0f,  // 'ß'
    8.0f,  // 'à'
    8.0f,  // 'á'
    8.0f,  // 'â'
    8.0f,  // 'ä'
    8.0f,  // 'ç'
    9.0f,  // 'è'
    9.0f,  // 'é'
    9.0f,  // 'ê'
    9.0f,  // 'ë'
    6.0f,  // 'ï'
    9.0f,  // 'ô'
    9.0f,  // 'ö'
    9.0f,  // 'ù'
    9.0f,  // 'û'
    9.0f,  // 'ü'
    14.0f, // '[A]'
    14.0f, // '[B]'
    14.0f, // '[C]'
    14.0f, // '[L]'
    14.0f, // '[R]'
    14.0f, // '[Z]'
    14.0f, // '[C-Up]'
    14.0f, // '[C-Down]'
    14.0f, // '[C-Left]'
    14.0f, // '[C-Right]'
    14.0f, // '▼'
    14.0f, // '[Control-Pad]'
    14.0f, // '[D-Pad]'
    14.0f, // ?
    14.0f, // ?
    14.0f, // ?
    14.0f, // ?
};



Gfx* Gfx_TextureRectangle(Gfx* head, f32 fX, f32 fY, f32 fWidth, f32 fHeight, u8 anchor) {
    s16 x = qs102(fX);
    s16 y = qs102(fY);
    u16 width = qu102(fWidth);
    u16 height = qu102(fHeight);
    u16 s;
    u16 t;
    u16 dsdx;
    u16 dtdy;
    s16 x2;
    s16 y2;

    if ((anchor & DRAW_ANCHOR_L) == 0) {
        if (anchor & DRAW_ANCHOR_R) {
            x -= width; // subtract full texture width from X position as it is anchored to right
        } else {
            x -= width >> 1; // subtract half texture width from X position as it is anchored to center
        }
    }

    if ((anchor & DRAW_ANCHOR_U) == 0) {
        if (anchor & DRAW_ANCHOR_D) {
            y -= height; // subtract full texture height from Y position as it is anchored to bottom
        } else {
            y -= height >> 1; // subtract half texture height from Y position as it is anchored to center
        }
    }

    if (x >= qu102(320) || y > qu102(240)) {
        return head; // Texture is to the right or below the screen, do not render
    }

    x2 = x + width;
    y2 = y + height;

    if (x2 < 0 || y2 < 0) {
        return head; // Texture is to the left or above the screen, do not render
    }

    dsdx = qu510((f32)_gfxTextureLoadWidth / fWidth);
    dtdy = qu510((f32)_gfxTextureLoadHeight / fHeight);

    if (x < 0) {
        // Texture starts left of the screen, so we offset s and set x to 0
        s = ((-x << 3) * dsdx) >> 10;
        x = 0;
    } else {
        s = 0;
    }

    if (y < 0) {
        // Texture starts above the screen, so we offset t and set y to 0
        t = ((-y << 3) * dtdy) >> 10;
        y = 0;
    } else {
        t = 0;
    }

    gSPTextureRectangle(head++, x, y, x2, y2, G_TX_RENDERTILE, s, t, dsdx, dtdy);
    gDPPipeSync(head++);
    return head;
}


/**
 * Assigns the index for the stick icon animation
 */
void Text_SetStickTexIndex(s32 newIndex) {
    sStickTexIndex = newIndex;
}

/**
 * Calculates the width in pixels of a single line of a string (stops after terminator or new line)
 */
void Text_CalcLineWidthHeight(char const* fmt, f32 fontSize, f32* width, f32* height, s32 useMono) {
    s32 i = 0;
    char chr;
    s32 adjWidth = true;

    *height = fontSize;
    *width = 0;

    while (true) {
        chr = fmt[i];
        i++;
        switch (chr) {
            case '\0': // Terminator character
                return;
            case '\n': // New line
                adjWidth = false;
                *height += fontSize;
                break;
            case '\x1': // A Button Icon
            case '\x2': // B Button Icon
            case '\x3': // R Button Icon
                if (adjWidth) {
                    *width += fontSize;
                }
                break;
            case '\x4': // Stick icon
                if (adjWidth) {
                    *width += fontSize * (32.0f / 24.0f) * 1.3f;
                }
                break;
            case '\x5': // Color
                i += 4;
                break;
            case '\x6': // Disable color
                break;
            default: // ASCII character
                if (useMono) {
                    if (adjWidth) {
                        *width += fontSize * MONO_DIST;
                    }
                } else {
                    if (adjWidth) {
                        *width += (sFontWidths[chr - ' '] / (f32)FONT_CHAR_TEX_WIDTH) * fontSize;
                    }
                }
                break;
        }
    }
}

/**
 * Finds the upper left coordinate to print a line of a string at
 */
void Text_AnchorCoords(f32* x, f32* y, char const* fmt, f32 fontSize, u8 anchor, s32 useMono) {
    u8 alignHorizontal = (anchor & DRAW_ANCHOR_L) == 0;
    u8 alignVertical = (anchor & DRAW_ANCHOR_U) == 0;

    if (alignHorizontal || alignVertical) {
        f32 width;
        f32 height;

        Text_CalcLineWidthHeight(fmt, fontSize, &width, &height, useMono);

        if (x != NULL && alignHorizontal) {
            *x -= (anchor & DRAW_ANCHOR_R) ? width : width / 2.0f;
        }
        if (y != NULL && alignVertical) {
            *y -= (anchor & DRAW_ANCHOR_D) ? height : height / 2.0f;
        }
    }
}

/**
 * Functions for setting the combiner to various presets for text drawing
 */
static inline void Text_SetTextCombiner(Gfx** glistp) {
    // Set combiner to use the primitive color for color, and the texture for alpha
    gDPSetCombineLERP((*glistp)++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                      PRIMITIVE, 0);
}

static inline void Text_SetShadowCombiner(Gfx** glistp) {
    // Set the combiner to use black for color and texture for alpha
    gDPSetCombineLERP((*glistp)++, 0, 0, 0, 0, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, 0, TEXEL0, 0, PRIMITIVE, 0);
}

static inline void Text_SetIconCombiner(Gfx** glistp) {
    // Set the combiner to use the texture color and alpha
    gDPSetCombineLERP((*glistp)++, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE, 0);
}

/**
 * Main text drawer
 */
void Text_DrawEx(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf, u8 r, u8 g,
                 u8 b, u8 a, s32 useShadow, s32 useMono) {
    f32 const xOrigin = x;
    char prevChr = 0, chr = 0;
    s32 i = 0;


    Text_SetTextCombiner(glistp);
    gDPSetPrimColor((*glistp)++, 0, 0, r, g, b, a);

    Text_AnchorCoords(&x, &y, fmt, fontSize, anchor, useMono);

    while (true) {
        chr = fmt[i];
        i++;

        switch (chr) {
            case '\0': // Terminator character
                return;
            case '\n': // New line
                x = xOrigin;
                Text_AnchorCoords(&x, NULL, &fmt[i], fontSize, anchor, useMono);
                y += fontSize;
                break;
            case '\x5': // Color
                gDPSetPrimColor((*glistp)++, 0, 0, fmt[i + 0], fmt[i + 1], fmt[i + 2], fmt[i + 3]);
                i += 4;
                break;
            case '\x6': // Disable color
                gDPSetPrimColor((*glistp)++, 0, 0, r, g, b, a);
                break;
            default: // ASCII character
                if (chr != ' ') {
                    // Load the character texture
                    char cha = chr - ' ';
                    u16 texID = cha * FONT_CHAR_TEX_SIZE;
                    Font_LoadChar(fontBuf, cha, texID);

                    if (chr != prevChr) {
                        (*glistp) = Gfx_TextureLoadI4((*glistp), &fontBuf->charTexBuf[texID],
                                                      FONT_CHAR_TEX_WIDTH, FONT_CHAR_TEX_HEIGHT);
                        prevChr = chr;
                    }
                    if (useShadow) {
                        // Draw character shadow
                        Text_SetShadowCombiner(glistp);
                        (*glistp) = Gfx_TextureRectangle((*glistp), x + 1, y + 1, fontSize, fontSize, DRAW_ANCHOR_UL);
                        Text_SetTextCombiner(glistp);
                    }

                    // Draw character
                    (*glistp) = Gfx_TextureRectangle((*glistp), x, y, fontSize, fontSize, DRAW_ANCHOR_UL);
                }
                if (useMono) {
                    x += fontSize * MONO_DIST;
                } else {
                    x += (sFontWidths[chr - ' '] / (f32)FONT_CHAR_TEX_WIDTH) * fontSize;
                }
                break;
        }
    }
}

/**
 * Wrappers for the main text drawer
 */
void Text_DrawShadowMonoColor(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf,
                              Color_RGBA8* color) {
    Text_DrawEx(glistp, fmt, fontSize, x, y, anchor, fontBuf, color->r, color->g, color->b, color->a, true, true);
}

void Text_DrawShadowMono(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf) {
    Text_DrawEx(glistp, fmt, fontSize, x, y, anchor, fontBuf, 255, 255, 255, 255, true, true);
}

void Text_DrawMonoColor(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf,
                        Color_RGBA8* color) {
    Text_DrawEx(glistp, fmt, fontSize, x, y, anchor, fontBuf, color->r, color->g, color->b, color->a, false, true);
}

void Text_DrawMono(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf) {
    Text_DrawEx(glistp, fmt, fontSize, x, y, anchor, fontBuf, 255, 255, 255, 255, false, true);
}

void Text_DrawShadowColor(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf,
                          Color_RGBA8* color) {
    Text_DrawEx(glistp, fmt, fontSize, x, y, anchor, fontBuf, color->r, color->g, color->b, color->a, true, false);
}

void Text_DrawShadow(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf) {
    Text_DrawEx(glistp, fmt, fontSize, x, y, anchor, fontBuf, 255, 255, 255, 255, true, false);
}

void Text_DrawColor(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf,
                    Color_RGBA8* color) {
    Text_DrawEx(glistp, fmt, fontSize, x, y, anchor, fontBuf, color->r, color->g, color->b, color->a, false, false);
}

void Text_Draw(Gfx** glistp, char const* fmt, f32 fontSize, f32 x, f32 y, u8 anchor, Font* fontBuf) {
    Text_DrawEx(glistp, fmt, fontSize, x, y, anchor, fontBuf, 255, 255, 255, 255, false, false);
}

Gfx* Text_Begin()
{

    Gfx* gfx = POLY_OPA_DISP + 1;
    gSPDisplayList(OVERLAY_DISP++, gfx);

    Gfx_SetupDL_39Ptr(&gfx);

    return gfx;
}

void Text_Finish(Gfx* gfx)
{
    gSPEndDisplayList(gfx++);
    gSPBranchList(POLY_OPA_DISP, gfx);
    POLY_OPA_DISP = gfx;
}

#endif