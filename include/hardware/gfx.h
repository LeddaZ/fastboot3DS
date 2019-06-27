#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mem_map.h"
#include "types.h"


#define SCREEN_TOP          (1)
#define SCREEN_SUB          (0)

#define SCREEN_WIDTH_TOP    (400)
#define SCREEN_HEIGHT_TOP   (240)
#define SCREEN_SIZE_TOP     (SCREEN_WIDTH_TOP * SCREEN_HEIGHT_TOP * 2)
#define SCREEN_WIDTH_SUB    (320)
#define SCREEN_HEIGHT_SUB   (240)
#define SCREEN_SIZE_SUB     (SCREEN_WIDTH_SUB * SCREEN_HEIGHT_SUB * 2)

#define FRAMEBUF_TOP_A_1    (VRAM_BASE)
#define FRAMEBUF_SUB_A_1    (FRAMEBUF_TOP_A_1 + SCREEN_SIZE_TOP)
#define FRAMEBUF_TOP_A_2    (VRAM_BASE + 0x100000)
#define FRAMEBUF_SUB_A_2    (FRAMEBUF_TOP_A_2 + SCREEN_SIZE_TOP)

#define RENDERBUF_TOP       (VRAM_BASE + 0x200000 - SCREEN_SIZE_TOP - SCREEN_SIZE_SUB)
#define RENDERBUF_SUB       (VRAM_BASE + 0x200000 - SCREEN_SIZE_SUB)

#define DEFAULT_BRIGHTNESS  (0x30)

/// Converts packed RGB8 to packed RGB565.
#define RGB8_to_565(r,g,b)  (((b)>>3)&0x1f)|((((g)>>2)&0x3f)<<5)|((((r)>>3)&0x1f)<<11)


#ifdef ARM11
typedef enum
{
	GFX_EVENT_PSC0   = 0u,
	GFX_EVENT_PSC1   = 1u,
	GFX_EVENT_PDC0   = 2u,
	GFX_EVENT_PDC1   = 3u,
	GFX_EVENT_PPF    = 4u,
	GFX_EVENT_P3D    = 5u
} GfxEvent;



void GX_memoryFill(u64 *buf0a, u32 buf0v, u32 buf0Sz, u32 val0, u64 *buf1a, u32 buf1v, u32 buf1Sz, u32 val1);
void GX_displayTransfer(u64 *in, u32 indim, u64 *out, u32 outdim, u32 flags);
void GX_textureCopy(u64 *in, u32 indim, u64 *out, u32 outdim, u32 size);
void GFX_setBrightness(u32 top, u32 sub);
void* GFX_getFramebuffer(u8 screen);
void GFX_swapFramebufs(void);
void GFX_waitForEvent(GfxEvent event, bool discard);
void GFX_init(bool clearScreens);
void GFX_enterLowPowerState(void);
void GFX_returnFromLowPowerState(void);
void GFX_deinit(bool keepLcdsOn);

#endif
