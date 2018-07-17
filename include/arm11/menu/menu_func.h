#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200, d0k3
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

#include "types.h"
#include "mem_map.h"
#include "arm11/hardware/cfg11.h"
#include "arm11/console.h"


#define NAND_BACKUP_PATH	"sdmc:/3DS" // NAND backups standard path
#define DEVICE_BUFSIZE		(((REG_CFG11_SOCINFO & 2) ? 1024 : 512) * 1024) // 1024 / 512 KiB
#define PROGRESS_WIDTH		20
#define SPLASH_DEFAULT_MSEC	1000
#define SPLASH_MIN_MSEC		500
#define SPLASH_MAX_MSEC		10000
#define VBLANK_APPROX_MSEC	16



u32 menuPresetNandTools(void);
u32 menuPresetBootMenu(void);
u32 menuPresetBootConfig(void);
u32 menuPresetSlotConfig1(void);
u32 menuPresetSlotConfig2(void);
u32 menuPresetSlotConfig3(void);
u32 menuPresetSlotConfig4(void);
u32 menuPresetSlotConfig5(void);
u32 menuPresetSlotConfig6(void);
u32 menuPresetSlotConfig7(void);
u32 menuPresetSlotConfig8(void);
u32 menuPresetSlotConfig9(void);
u32 menuPresetBootMode(void);
u32 menuPresetSplashConfig(void);

u32 menuReturn(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuSetBootMode(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuSetSplash(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuSetSplashDuration(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuSwitchFcramBoot(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuSetupBootSlot(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuSetupBootKeys(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuLaunchFirm(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuBackupNand(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuRestoreNand(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuInstallFirm(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuUpdateFastboot3ds(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuShowCredits(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 menuDumpBootrom(PrintConsole* term_con, PrintConsole* menu_con, u32 param);

// everything below has to go
u32 menuDummyFunc(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 debugSettingsView(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
u32 debugEscapeTest(PrintConsole* term_con, PrintConsole* menu_con, u32 param);
