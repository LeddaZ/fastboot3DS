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

#include <stdlib.h>
#include "types.h"
#include "ipc_handler.h"
#include "hardware/cache.h"
#include "arm11/debug.h"



u32 IPC_handleCmd(u8 cmdId, u32 inBufs, u32 outBufs, UNUSED const u32 *const buf)
{
	for(u32 i = 0; i < inBufs; i++)
	{
		const IpcBuffer *const inBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(inBuf->ptr && inBuf->size) invalidateDCacheRange(inBuf->ptr, inBuf->size);
	}

	u32 result = 0;
	switch(cmdId)
	{
		case IPC_CMD_ID_MASK(IPC_CMD11_PRINT_MSG):
		case IPC_CMD_ID_MASK(IPC_CMD11_PANIC):
		case IPC_CMD_ID_MASK(IPC_CMD11_EXCEPTION):
			break;
		default:
			panic();
	}

	for(u32 i = inBufs; i < inBufs + outBufs; i++)
	{
		const IpcBuffer *const outBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(outBuf->ptr && outBuf->size) flushInvalidateDCacheRange(outBuf->ptr, outBuf->size);
	}

	return result;
}
