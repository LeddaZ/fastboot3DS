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
#include "fb_assert.h"
#include "types.h"
#include "arm9/hardware/crypto.h"
#include "arm9/hardware/cfg9.h"
#include "arm9/hardware/interrupt.h"
#include "arm9/hardware/ndma.h"
#include "arm.h"
#include "mmio.h"



//////////////////////////////////
//             AES              //
//////////////////////////////////

#define AES_REGS_BASE        (IO_MEM_ARM9_ONLY + 0x9000)
#define REG_AESCNT           *((vu32*)(AES_REGS_BASE + 0x000))

#define REG_AESBLKCNT        *((vu32*)(AES_REGS_BASE + 0x004))
#define REG_AES_BLKCNT_LOW   *((vu16*)(AES_REGS_BASE + 0x004))
#define REG_AES_BLKCNT_HIGH  *((vu16*)(AES_REGS_BASE + 0x006))
#define REG_AESWRFIFO         (        AES_REGS_BASE + 0x008)
#define REG_AESRDFIFO         (        AES_REGS_BASE + 0x00C)
#define REG_AESKEYSEL        *((vu8* )(AES_REGS_BASE + 0x010))
#define REG_AESKEYCNT        *((vu8* )(AES_REGS_BASE + 0x011))
#define REG_AESCTR            ((vu32*)(AES_REGS_BASE + 0x020))
#define REG_AESMAC            ((vu32*)(AES_REGS_BASE + 0x030))

#define REG_AESKEY0           ((vu32*)(AES_REGS_BASE + 0x040))
#define REG_AESKEYX0          ((vu32*)(AES_REGS_BASE + 0x050))
#define REG_AESKEYY0          ((vu32*)(AES_REGS_BASE + 0x060))
#define REG_AESKEY1           ((vu32*)(AES_REGS_BASE + 0x070))
#define REG_AESKEYX1          ((vu32*)(AES_REGS_BASE + 0x080))
#define REG_AESKEYY1          ((vu32*)(AES_REGS_BASE + 0x090))
#define REG_AESKEY2           ((vu32*)(AES_REGS_BASE + 0x0A0))
#define REG_AESKEYX2          ((vu32*)(AES_REGS_BASE + 0x0B0))
#define REG_AESKEYY2          ((vu32*)(AES_REGS_BASE + 0x0C0))
#define REG_AESKEY3           ((vu32*)(AES_REGS_BASE + 0x0D0))
#define REG_AESKEYX3          ((vu32*)(AES_REGS_BASE + 0x0E0))
#define REG_AESKEYY3          ((vu32*)(AES_REGS_BASE + 0x0F0))

#define REG_AESKEYFIFO        ((vu32*)(AES_REGS_BASE + 0x100))
#define REG_AESKEYXFIFO       ((vu32*)(AES_REGS_BASE + 0x104))
#define REG_AESKEYYFIFO       ((vu32*)(AES_REGS_BASE + 0x108))



static void setupKeys(void)
{
	// Setup TWL unit info and console ID
	const bool isDevUnit = REG_CFG9_UNITINFO != 0;
	const u64 twlConsoleId = (isDevUnit ? (*((vu64*)0x10012000)) :
	                                      ((*((vu64*)0x01FFB808) ^ 0x8C267B7B358A6AFULL) | 0x80000000ULL));
	*((vu8*)0x10010014) = REG_CFG9_UNITINFO;
	*((vu64*)0x10012100) = twlConsoleId;


	// TWL key init
	REG_AESCNT = 0;
	REG_AESKEYX1[2] = (u32)(twlConsoleId>>32);
	REG_AESKEYX1[3] = (u32)twlConsoleId;

	AES_setKey(0x02, AES_KEY_X, AES_INPUT_LITTLE | AES_INPUT_REVERSED, false, (u32*)0x01FFD398);

	u32 key3X[4] = {(u32)twlConsoleId, 0, 0, (u32)(twlConsoleId>>32)};
	if(isDevUnit)
	{
		key3X[1] = 0xEE7A4B1E;
		key3X[2] = 0xAF42C08B;
		AES_setKey(0x03, AES_KEY_X, AES_INPUT_LITTLE | AES_INPUT_REVERSED, false, key3X);

		alignas(4) static const u8 key2YDev[16] = {
		0x3B, 0x06, 0x86, 0x57, 0x33, 0x04, 0x88, 0x11, 0x49, 0x04, 0x6B, 0x33, 0x12, 0x02, 0xAC, 0xF3};
		alignas(4) static const u8 key3YDev[16] = {
		0xAA, 0xBF, 0x76, 0xF1, 0x7A, 0xB8, 0xE8, 0x66, 0x97, 0x64, 0x6A, 0x26, 0x05, 0x00, 0xA0, 0xE1};
		AES_setKey(0x02, AES_KEY_Y, AES_INPUT_LITTLE | AES_INPUT_REVERSED, false, (const u32*)key2YDev);
		AES_setKey(0x03, AES_KEY_Y, AES_INPUT_LITTLE | AES_INPUT_REVERSED, false, (const u32*)key3YDev);
	}
	else
	{
		key3X[1] = 0x544E494E; // "NINT"
		key3X[2] = 0x4F444E45; // "ENDO"
		AES_setKey(0x03, AES_KEY_X, AES_INPUT_LITTLE | AES_INPUT_REVERSED, false, key3X);

		AES_setKey(0x02, AES_KEY_Y, AES_INPUT_LITTLE | AES_INPUT_REVERSED, false, (u32*)0x01FFD220);

		u32 key3YRetail[4];
		key3YRetail[0] = ((u32*)0x01FFD3C8)[0];
		key3YRetail[1] = ((u32*)0x01FFD3C8)[1];
		key3YRetail[2] = ((u32*)0x01FFD3C8)[2];
		key3YRetail[3] = 0xE1A00005;
		AES_setKey(0x03, AES_KEY_Y, AES_INPUT_LITTLE | AES_INPUT_REVERSED, false, key3YRetail);
	}


	// 3DS key init
	if(REG_CFG9_SOCINFO & 2) // New 3DS
	{
		alignas(4) static const u8 keyY0x05[16] = {
		0x4D, 0x80, 0x4F, 0x4E, 0x99, 0x90, 0x19, 0x46, 0x13, 0xA2, 0x04, 0xAC, 0x58, 0x44, 0x60, 0xBE};
		AES_setKey(0x05, AES_KEY_Y, AES_INPUT_BIG | AES_INPUT_NORMAL, false, (const u32*)keyY0x05);
	}

	alignas(4) static const u8 keyY0x24[16] = {
	0x74, 0xCA, 0x07, 0x48, 0x84, 0xF4, 0x22, 0x8D, 0xEB, 0x2A, 0x1C, 0xA7, 0x2D, 0x28, 0x77, 0x62};
	AES_setKey(0x24, AES_KEY_Y, AES_INPUT_BIG | AES_INPUT_NORMAL, false, (const u32*)keyY0x24);

	alignas(4) static const u8 keyX0x25s[2][16] = {
	 {0xCE, 0xE7, 0xD8, 0xAB, 0x30, 0xC0, 0x0D, 0xAE, 0x85, 0x0E, 0xF5, 0xE3, 0x82, 0xAC, 0x5A, 0xF3},
	 {0x81, 0x90, 0x7A, 0x4B, 0x6F, 0x1B, 0x47, 0x32, 0x3A, 0x67, 0x79, 0x74, 0xCE, 0x4A, 0xD7, 0x1B}};
	AES_setKey(0x25, AES_KEY_X, AES_INPUT_BIG | AES_INPUT_NORMAL, false, (const u32*)keyX0x25s[isDevUnit]);

	alignas(4) static const u8 keyY0x2Fs[2][16] = {
	 {0xC3, 0x69, 0xBA, 0xA2, 0x1E, 0x18, 0x8A, 0x88, 0xA9, 0xAA, 0x94, 0xE5, 0x50, 0x6A, 0x9F, 0x16},
	 {0x73, 0x25, 0xC4, 0xEB, 0x14, 0x3A, 0x0D, 0x5F, 0x5D, 0xB6, 0xE5, 0xC5, 0x7A, 0x21, 0x95, 0xAC}};
	AES_setKey(0x2F, AES_KEY_Y, AES_INPUT_BIG | AES_INPUT_NORMAL, false, (const u32*)keyY0x2Fs[isDevUnit]);

	// Set 0x11 keyslot
	alignas(4) static const u8 key1s[2][16] = {
	 {0x07, 0x29, 0x44, 0x38, 0xF8, 0xC9, 0x75, 0x93, 0xAA, 0x0E, 0x4A, 0xB4, 0xAE, 0x84, 0xC1, 0xD8},
	 {0xA2, 0xF4, 0x00, 0x3C, 0x7A, 0x95, 0x10, 0x25, 0xDF, 0x4E, 0x9E, 0x74, 0xE3, 0x0C, 0x92, 0x99}};
	alignas(4) static const u8 key2s[2][16] = {
	 {0x42, 0x3F, 0x81, 0x7A, 0x23, 0x52, 0x58, 0x31, 0x6E, 0x75, 0x8E, 0x3A, 0x39, 0x43, 0x2E, 0xD0},
	 {0xFF, 0x77, 0xA0, 0x9A, 0x99, 0x81, 0xE9, 0x48, 0xEC, 0x51, 0xC9, 0x32, 0x5D, 0x14, 0xEC, 0x25}};


	alignas(4) u8 keyBlocks[2][16] = {
	 {0xA4, 0x8D, 0xE4, 0xF1, 0x0B, 0x36, 0x44, 0xAA, 0x90, 0x31, 0x28, 0xFF, 0x4D, 0xCA, 0x76, 0xDF},
	 {0xDD, 0xDA, 0xA4, 0xC6, 0x2C, 0xC4, 0x50, 0xE9, 0xDA, 0xB6, 0x9B, 0x0D, 0x9D, 0x2A, 0x21, 0x98}};
	u32 decKey[4];

	AES_ctx ctx;
	AES_setCryptParams(&ctx, AES_INPUT_BIG | AES_INPUT_NORMAL, AES_OUTPUT_BIG | AES_OUTPUT_NORMAL);

	// key 0x18
	AES_setKey(0x11, AES_KEY_NORMAL, AES_INPUT_BIG | AES_INPUT_NORMAL, false, (const u32*)key1s[isDevUnit]);
	AES_selectKeyslot(0x11);
	AES_ecb(&ctx, (const u32*)keyBlocks[0], decKey, 1, false, false);
	AES_setKey(0x18, AES_KEY_X, AES_INPUT_BIG | AES_INPUT_NORMAL, false, decKey);

	AES_setKey(0x11, AES_KEY_NORMAL, AES_INPUT_BIG | AES_INPUT_NORMAL, false, (const u32*)key2s[isDevUnit]);
	AES_selectKeyslot(0x11);
	for(u8 slot = 0x19; slot < 0x20; slot++, keyBlocks[1][0xF]++)
	{
		AES_ecb(&ctx, (const u32*)keyBlocks[1], decKey, 1, false, false);
		AES_setKey(slot, AES_KEY_X, AES_INPUT_BIG | AES_INPUT_NORMAL, false, decKey);
	}
}

void AES_init(void)
{
	REG_AESCNT = AES_MAC_SIZE(4) | AES_FLUSH_WRITE_FIFO | AES_FLUSH_READ_FIFO;
	*((vu8*)0x10000008) |= 0xCu; // ??

	REG_NDMA0_DST_ADDR = REG_AESWRFIFO;
	REG_NDMA0_INT_CNT = NDMA_INT_SYS_FREQ;
	REG_NDMA0_CNT = NDMA_TOTAL_CNT_MODE | NDMA_STARTUP_AES_IN | NDMA_BURST_WORDS(4) |
	                NDMA_SRC_UPDATE_INC | NDMA_DST_UPDATE_FIXED;

	REG_NDMA1_SRC_ADDR = REG_AESRDFIFO;
	REG_NDMA1_INT_CNT = NDMA_INT_SYS_FREQ;
	REG_NDMA1_CNT = NDMA_TOTAL_CNT_MODE | NDMA_STARTUP_AES_OUT | NDMA_BURST_WORDS(4) |
	                NDMA_SRC_UPDATE_FIXED | NDMA_DST_UPDATE_INC;

	IRQ_registerHandler(IRQ_AES, NULL);

	setupKeys();
}

void AES_setKey(u8 keyslot, AesKeyType type, u8 orderEndianess, bool twlScrambler, const u32 key[4])
{
	fb_assert(keyslot < 0x40);
	fb_assert(key != NULL);


	REG_AESCNT = (u32)orderEndianess<<23;
	if(keyslot > 3)
	{
		REG_AESKEYCNT = 0x80u | (type > AES_KEY_NORMAL && twlScrambler ? 1u : 0u)<<6 | keyslot;
		REG_AESKEYFIFO[type] = key[0];
		REG_AESKEYFIFO[type] = key[1];
		REG_AESKEYFIFO[type] = key[2];
		REG_AESKEYFIFO[type] = key[3];
	}
	else
	{
		u32 lastu32;
		vu32 *twlKeyNReg = &REG_AESKEY0[12u * keyslot + type * 4u];
		if(orderEndianess & AES_INPUT_NORMAL)
		{
			twlKeyNReg[0] = key[3];
			twlKeyNReg[1] = key[2];
			twlKeyNReg[2] = key[1];
			lastu32 = key[0];
		}
		else
		{
			twlKeyNReg[0] = key[0];
			twlKeyNReg[1] = key[1];
			twlKeyNReg[2] = key[2];
			lastu32 = key[3];
		}
		twlKeyNReg[3] = lastu32;
	}
}

void AES_selectKeyslot(u8 keyslot)
{
	fb_assert(keyslot < 0x40);

	REG_AESKEYSEL = keyslot;
	REG_AESCNT |= AES_UPDATE_KEYSLOT;
}

void AES_setNonce(AES_ctx *const ctx, u8 orderEndianess, const u32 nonce[3])
{
	fb_assert(ctx != NULL);
	fb_assert(nonce != NULL);


	ctx->ctrIvNonceParams = (u32)orderEndianess<<23;
	u32 *const ctrIvNonce = ctx->ctrIvNonce;
	u32 lastu32;
	if(orderEndianess & AES_INPUT_NORMAL)
	{
		ctrIvNonce[0] = nonce[2];
		ctrIvNonce[1] = nonce[1];
		lastu32 = nonce[0];
	}
	else
	{
		ctrIvNonce[0] = nonce[0];
		ctrIvNonce[1] = nonce[1];
		lastu32 = nonce[2];
	}
	ctrIvNonce[2] = lastu32;
}

void AES_setCtrIv(AES_ctx *const ctx, u8 orderEndianess, const u32 ctrIv[4])
{
	fb_assert(ctx != NULL);
	fb_assert(ctrIv != NULL);


	ctx->ctrIvNonceParams = (u32)orderEndianess<<23;
	u32 *const ctrIvNonce = ctx->ctrIvNonce;
	u32 lastu32;
	if(orderEndianess & AES_INPUT_NORMAL)
	{
		ctrIvNonce[0] = ctrIv[3];
		ctrIvNonce[1] = ctrIv[2];
		ctrIvNonce[2] = ctrIv[1];
		lastu32 = ctrIv[0];
	}
	else
	{
		ctrIvNonce[0] = ctrIv[0];
		ctrIvNonce[1] = ctrIv[1];
		ctrIvNonce[2] = ctrIv[2];
		lastu32 = ctrIv[3];
	}
	ctrIvNonce[3] = lastu32;
}

// TODO: Handle endianess!
NAKED void AES_addCounter(u32 ctr[4], u32 val)
{
    __asm__
	(
		"stmfd sp!, {r4, lr}\n\t"
		"ldm r0, {r2-r4, lr}\n\t"
		"adds r2, r1, lsr #4\n\t"
		"addcss r3, r3, #1\n\t"
		"addcss r4, r4, #1\n\t"
		"addcs lr, lr, #1\n\t"
		"stm r0, {r2-r4, lr}\n\t"
		"ldmfd sp!, {r4, pc}\n\t"
		: : "r" (ctr), "r" (val) :
	);
}

void AES_setCryptParams(AES_ctx *const ctx, u8 inEndianessOrder, u8 outEndianessOrder)
{
	fb_assert(ctx != NULL);

	ctx->aesParams = (u32)inEndianessOrder<<23 | (u32)outEndianessOrder<<22;
}

static void aesProcessBlocksCpu(const u32 *in, u32 *out, u32 blocks)
{
	REG_AES_BLKCNT_HIGH = blocks;
	REG_AESCNT |= AES_ENABLE | 3<<12 | AES_FLUSH_READ_FIFO | AES_FLUSH_WRITE_FIFO;

	for(u32 i = 0; i < blocks * 4; i += 4)
	{
		_u128 tmp = *((const _u128*)&in[i]);
		*((vu32*)REG_AESWRFIFO) = tmp.data[0];
		*((vu32*)REG_AESWRFIFO) = tmp.data[1];
		*((vu32*)REG_AESWRFIFO) = tmp.data[2];
		*((vu32*)REG_AESWRFIFO) = tmp.data[3];

		while(AES_READ_FIFO_COUNT == 0);

		tmp.data[0] = *((vu32*)REG_AESRDFIFO);
		tmp.data[1] = *((vu32*)REG_AESRDFIFO);
		tmp.data[2] = *((vu32*)REG_AESRDFIFO);
		tmp.data[3] = *((vu32*)REG_AESRDFIFO);
		*((_u128*)&out[i]) = tmp;
	}
}

// AES_init() must be called before this works
static void aesProcessBlocksDma(const u32 *in, u32 *out, u32 blocks)
{
	// DMA can't reach TCMs
	fb_assert(((u32)in >= ITCM_BOOT9_MIRROR + ITCM_SIZE) && (((u32)in < DTCM_BASE) || ((u32)in >= DTCM_BASE + DTCM_SIZE)));
	fb_assert(((u32)out >= ITCM_BOOT9_MIRROR + ITCM_SIZE) && (((u32)out < DTCM_BASE) || ((u32)out >= DTCM_BASE + DTCM_SIZE)));


	// Check block alignment
	u32 aesFifoSize;
	if(!(blocks & 1)) aesFifoSize = 1; // 32 bytes
	else              aesFifoSize = 0; // 16 bytes

	REG_NDMA0_SRC_ADDR = (u32)in;
	REG_NDMA0_TOTAL_CNT = blocks<<2;
	REG_NDMA0_LOG_BLK_CNT = aesFifoSize * 4 + 4;
	REG_NDMA0_CNT |= NDMA_ENABLE;

	REG_NDMA1_DST_ADDR = (u32)out;
	REG_NDMA1_TOTAL_CNT = blocks<<2;
	REG_NDMA1_LOG_BLK_CNT = aesFifoSize * 4 + 4;
	REG_NDMA1_CNT |= NDMA_ENABLE;

	REG_AES_BLKCNT_HIGH = blocks;
	REG_AESCNT |= AES_ENABLE | AES_IRQ_ENABLE | aesFifoSize<<14 | (3 - aesFifoSize)<<12 |
	              AES_FLUSH_READ_FIFO | AES_FLUSH_WRITE_FIFO;
	while(REG_AESCNT & AES_ENABLE) __wfi();
}

void AES_ctr(AES_ctx *const ctx, const u32 *in, u32 *out, u32 blocks, bool dma)
{
	fb_assert(ctx != NULL);
	fb_assert(in != NULL);
	fb_assert(out != NULL);

	const u32 ctrParams = ctx->ctrIvNonceParams;
	u32 *const ctr = ctx->ctrIvNonce;
	const u32 aesParams = AES_MODE_CTR | ctx->aesParams;


	while(blocks)
	{
		REG_AESCNT = ctrParams;
		REG_AESCTR[0] = ctr[0];
		REG_AESCTR[1] = ctr[1];
		REG_AESCTR[2] = ctr[2];
		REG_AESCTR[3] = ctr[3];

		REG_AESCNT = aesParams;
		u32 blockNum = ((blocks > AES_MAX_BLOCKS) ? AES_MAX_BLOCKS : blocks);
		if(dma) aesProcessBlocksDma(in, out, blockNum);
		else aesProcessBlocksCpu(in, out, blockNum);

		AES_addCounter(ctr, blockNum<<4);
		in += blockNum<<2;
		out += blockNum<<2;
		blocks -= blockNum;
	}
}

/*void AES_cbc(AES_ctx *const ctx, const u32 *in, u32 *out, u32 blocks, bool enc, bool dma)
{
	fb_assert(ctx != NULL);
	fb_assert(in != NULL);
	fb_assert(out != NULL);

	const u32 ivParams = ctx->ctrIvNonceParams;
	u32 *const iv = ctx->ctrIvNonce;
	const u32 aesParams = (enc ? AES_MODE_CBC_ENCRYPT : AES_MODE_CBC_DECRYPT) | ctx->aesParams;


	while(blocks)
	{
		REG_AESCNT = ivParams;
		REG_AESCTR[0] = iv[0];
		REG_AESCTR[1] = iv[1];
		REG_AESCTR[2] = iv[2];
		REG_AESCTR[3] = iv[3];

		u32 blockNum = ((blocks > AES_MAX_BLOCKS) ? AES_MAX_BLOCKS : blocks);

		if(!enc)
		{
			// Save last 16 bytes of the input blocks as next IV
			const u32 *const nextIv = in + (blockNum<<2) - 4;
			if(aesParams>>23 & AES_INPUT_NORMAL)
			{
				iv[0] = nextIv[3];
				iv[1] = nextIv[2];
				iv[2] = nextIv[1];
				iv[3] = nextIv[0];
			}
			else
			{
				iv[0] = nextIv[0];
				iv[1] = nextIv[1];
				iv[2] = nextIv[2];
				iv[3] = nextIv[3];
			}
		}

		REG_AESCNT = aesParams;
		if(dma) aesProcessBlocksDma(in, out, blockNum);
		else aesProcessBlocksCpu(in, out, blockNum);

		if(enc)
		{
			// Save last 16 bytes of the output blocks as next IV
			const u32 *const nextIv = out + (blockNum<<2) - 4;
			if(aesParams>>23 & AES_INPUT_NORMAL)
			{
				iv[0] = nextIv[3];
				iv[1] = nextIv[2];
				iv[2] = nextIv[1];
				iv[3] = nextIv[0];
			}
			else
			{
				iv[0] = nextIv[0];
				iv[1] = nextIv[1];
				iv[2] = nextIv[2];
				iv[3] = nextIv[3];
			}
		}

		in += blockNum<<2;
		out += blockNum<<2;
		blocks -= blockNum;
	}
}*/

void AES_ecb(AES_ctx *const ctx, const u32 *in, u32 *out, u32 blocks, bool enc, bool dma)
{
	fb_assert(ctx != NULL);
	fb_assert(in != NULL);
	fb_assert(out != NULL);

	const u32 aesParams =  (enc ? AES_MODE_ECB_ENCRYPT : AES_MODE_ECB_DECRYPT) | ctx->aesParams;


	while(blocks)
	{
		REG_AESCNT = aesParams;
		u32 blockNum = ((blocks > AES_MAX_BLOCKS) ? AES_MAX_BLOCKS : blocks);
		if(dma) aesProcessBlocksDma(in, out, blockNum);
		else aesProcessBlocksCpu(in, out, blockNum);

		in += blockNum<<2;
		out += blockNum<<2;
		blocks -= blockNum;
	}
}

bool AES_ccm(const AES_ctx *const ctx, const u32 *const in, u32 *const out, u32 macSize,
             u32 mac[4], u16 blocks, bool enc)
{
	fb_assert(ctx != NULL);
	fb_assert(in != NULL);
	fb_assert(out != NULL);
	fb_assert(macSize != 0);
	fb_assert(mac != NULL);
	fb_assert(blocks != 0);


	REG_AESCNT = ctx->ctrIvNonceParams;
	REG_AESCTR[0] = ctx->ctrIvNonce[0];
	REG_AESCTR[1] = ctx->ctrIvNonce[1];
	REG_AESCTR[2] = ctx->ctrIvNonce[2];

	REG_AES_BLKCNT_LOW = 0;
	REG_AESCNT = (enc ? AES_MODE_CCM_ENCRYPT : AES_MODE_CCM_DECRYPT) |
	             AES_MAC_SIZE(macSize) | ctx->aesParams;
	aesProcessBlocksCpu(in, out, blocks);

	// This is broken right now with DMA due to a (AES engine?) bug.
	if(!enc)
	{
		*((vu32*)REG_AESWRFIFO) = mac[0];
		*((vu32*)REG_AESWRFIFO) = mac[1];
		*((vu32*)REG_AESWRFIFO) = mac[2];
		*((vu32*)REG_AESWRFIFO) = mac[3];
		while(REG_AESCNT & AES_ENABLE);
	}
	else
	{
		mac[0] = *((vu32*)REG_AESRDFIFO);
		mac[1] = *((vu32*)REG_AESRDFIFO);
		mac[2] = *((vu32*)REG_AESRDFIFO);
		mac[3] = *((vu32*)REG_AESRDFIFO);
	}

	if(enc) return true;
	else return AES_IS_MAC_VALID;
}



//////////////////////////////////
//             SHA              //
//////////////////////////////////

#define SHA_REGS_BASE    (IO_MEM_ARM9_ONLY + 0xA000)
#define REG_SHA_CNT      *((vu32*)(SHA_REGS_BASE + 0x00))
#define REG_SHA_BLKCNT   *((vu32*)(SHA_REGS_BASE + 0x04))
#define REGs_SHA_HASH     ((vu32*)(SHA_REGS_BASE + 0x40))
#define REGs_SHA_INFIFO   ((vu32*)(SHA_REGS_BASE + 0x80))


void SHA_start(u8 params)
{
	REG_SHA_CNT = params | SHA_IN_DMA_ENABLE | SHA_ENABLE;
}

void SHA_update(const u32 *data, u32 size)
{
	while(size >= 64)
	{
		*((volatile _u512*)REGs_SHA_INFIFO) = *((const _u512*)data);
		data += 64 / 4;
		size -= 64;
		while(REG_SHA_CNT & SHA_ENABLE);
	}

	if(size) iomemcpy(REGs_SHA_INFIFO, data, size);
}

void SHA_finish(u32 *const hash, u8 endianess)
{
	REG_SHA_CNT = (REG_SHA_CNT & SHA_MODE_MASK) | endianess | SHA_FINAL_ROUND;
	while(REG_SHA_CNT & SHA_ENABLE);

	SHA_getState(hash);
}

void SHA_getState(u32 *const out)
{
	u32 size;
	switch(REG_SHA_CNT & SHA_MODE_MASK)
	{
		case SHA_MODE_256:
			size = 32;
			break;
		case SHA_MODE_224:
			size = 28;
			break;
		case SHA_MODE_1:
		default:           // 2 and 3 are both SHA1
			size = 20;
	}

	iomemcpy(out, REGs_SHA_HASH, size);
}

void sha(const u32 *data, u32 size, u32 *const hash, u8 params, u8 hashEndianess)
{
	SHA_start(params);
	SHA_update(data, size);
	SHA_finish(hash, hashEndianess);
}

/*void sha_dma(const u32 *data, u32 size, u32 *const hash, u8 params, u8 hashEndianess)
{
	REG_NDMA2_SRC_ADDR = (u32)data;
	REG_NDMA2_DST_ADDR = REG_SHA_INFIFO;
	REG_NDMA2_TOTAL_CNT = size / 4;
	REG_NDMA2_LOG_BLK_CNT = 64 / 4;
	REG_NDMA2_INT_CNT = NDMA_INT_SYS_FREQ;
	REG_NDMA2_CNT = NDMA_DST_UPDATE_INC | NDMA_DST_ADDR_RELOAD | NDMA_SRC_UPDATE_INC |
	                NDMA_BURST_WORDS(64 / 4) | NDMA_TOTAL_CNT_MODE | NDMA_STARTUP_SHA | NDMA_ENABLE;
	SHA_start(params);
	while(REG_NDMA2_CNT & NDMA_ENABLE);
	while(REG_SHA_CNT & SHA_ENABLE);
	SHA_finish(hash, hashEndianess);
}*/

//////////////////////////////////
//             RSA              //
//////////////////////////////////

#define RSA_REGS_BASE   (IO_MEM_ARM9_ONLY + 0xB000)
#define REG_RSA_CNT     *((vu32*)(RSA_REGS_BASE + 0x000))
#define REG_RSA_UNK_F0  *((vu32*)(RSA_REGS_BASE + 0x0F0))
#define REGs_RSA_SLOT0   ((vu32*)(RSA_REGS_BASE + 0x100))
#define REGs_RSA_SLOT1   ((vu32*)(RSA_REGS_BASE + 0x110))
#define REGs_RSA_SLOT2   ((vu32*)(RSA_REGS_BASE + 0x120))
#define REGs_RSA_SLOT3   ((vu32*)(RSA_REGS_BASE + 0x130))
#define rsaSlots         ((RsaSlot*)(RSA_REGS_BASE + 0x100))
#define REGs_RSA_EXP     ((vu32*)(RSA_REGS_BASE + 0x200))
#define REGs_RSA_MOD     ((vu32*)(RSA_REGS_BASE + 0x400))
#define REGs_RSA_TXT     ((vu32*)(RSA_REGS_BASE + 0x800))

typedef struct
{
	vu32 REG_RSA_SLOTCNT;
	vu32 REG_RSA_SLOTSIZE;
	vu32 REG_RSA_SLOT_UNK_0x8;
	vu32 REG_RSA_SLOT_UNK_0xC;
} RsaSlot;


void RSA_init(void)
{
	REG_RSA_UNK_F0 = 0;
}

static void rsaWaitBusy(void)
{
	while(REG_RSA_CNT & RSA_ENABLE);
}

void RSA_selectKeyslot(u8 keyslot)
{
	fb_assert(keyslot < 4);

	rsaWaitBusy();
	REG_RSA_CNT = (REG_RSA_CNT & ~RSA_KEYSLOT(0xFu)) | RSA_KEYSLOT(keyslot);
}

bool RSA_setKey2048(u8 keyslot, const u32 *const mod, u32 exp)
{
	fb_assert(keyslot < 4);
	fb_assert(mod != NULL);

	RsaSlot *slot = &rsaSlots[keyslot];
	rsaWaitBusy();
	if(slot->REG_RSA_SLOTCNT & RSA_KEY_WR_PROT) return false;
	// Unset key if bit 31 is not set. No idea why but boot9 does this.
	if(!(slot->REG_RSA_SLOTCNT & RSA_KEY_UNK_BIT31)) slot->REG_RSA_SLOTCNT &= ~RSA_KEY_STAT_SET;

	REG_RSA_CNT = RSA_INPUT_NORMAL | RSA_INPUT_BIG | RSA_KEYSLOT(keyslot);
	iomemset(REGs_RSA_EXP, 0, 0x100 - 4);
	REGs_RSA_EXP[(0x100>>2) - 1] = exp;

	if(slot->REG_RSA_SLOTSIZE != RSA_SLOTSIZE_2048) return false;
	iomemcpy(REGs_RSA_MOD, mod, 0x100);

	return true;
}

bool RSA_decrypt2048(u32 *const decSig, const u32 *const encSig)
{
	fb_assert(decSig != NULL);
	fb_assert(encSig != NULL);

	const u8 keyslot = RSA_GET_KEYSLOT;
	rsaWaitBusy();
	if(!(rsaSlots[keyslot].REG_RSA_SLOTCNT & RSA_KEY_STAT_SET)) return false;

	REG_RSA_CNT |= RSA_INPUT_NORMAL | RSA_INPUT_BIG;
	iomemcpy(REGs_RSA_TXT, encSig, 0x100);

	REG_RSA_CNT |= RSA_ENABLE;
	rsaWaitBusy();
	iomemcpy(decSig, REGs_RSA_TXT, 0x100);

	return true;
}

bool RSA_verify2048(const u32 *const encSig, const u32 *const data, u32 size)
{
	fb_assert(encSig != NULL);
	fb_assert(data != NULL);

	alignas(4) u8 decSig[0x100];
	if(!RSA_decrypt2048((u32*)decSig, encSig)) return false;

	if(decSig[0] != 0x00 || decSig[1] != 0x01) return false;

	u32 read = 2;
	while(read < 0x100)
	{
		if(decSig[read] != 0xFF) break;
		read++;
	}
	if(read != 0xCC || decSig[read] != 0x00) return false;

	// ASN.1 is a clusterfuck so we skip parsing the remaining headers
	// and hardcode the hash location.

	alignas(4) u8 hash[32];
	sha(data, size, (u32*)hash, SHA_INPUT_BIG | SHA_MODE_256, SHA_OUTPUT_BIG);

	// Compare hash
	u8 res = 0;
	for(u32 i = 0; i < 32; i++)
	{
		u8 tmp;
		if(decSig[0xE0 + i] == hash[i]) tmp = 0;
		else tmp = 1;

		res |= tmp;
	}

	return res == 0;
}
