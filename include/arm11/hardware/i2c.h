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

#include "types.h"


// REG_I2C_CNT
#define I2C_STOP          (1u)
#define I2C_START         (1u<<1)
#define I2C_ERROR         (1u<<2)
#define I2C_ACK           (1u<<4)
#define I2C_DIR_WRITE     (0u)
#define I2C_DIR_READ      (1u<<5)
#define I2C_IRQ_ENABLE    (1u<<6)
#define I2C_ENABLE        (1u<<7)

// REG_I2C_CNTEX
#define I2C_SCL_STATE     (1u)     // Read-only SCL line state?
#define I2C_CLK_STRETCH   (1u<<1)  // Enables clock stretching
#define I2C_UNK_CNTEX15   (1u<<15) // Unknown

// REG_I2C_SCL
#define I2C_DELAYS(high, low)  ((high)<<8 | (low)) // TODO: How long and when does it delay?


typedef enum
{
	I2C_DEV_TWL_MCU   =  0u, // DSi mode MCU
	I2C_DEV_CAMERA1   =  1u, // Internal self-facing camera
	I2C_DEV_CAMERA2   =  2u, // External right camera
	I2C_DEV_CTR_MCU   =  3u,
	I2C_DEV_CAMERA3   =  4u, // External left camera
	I2C_DEV_LCD0      =  5u, // Upper LCD
	I2C_DEV_LCD1      =  6u, // Lower LCD
	I2C_DEV_UNK7      =  7u, // Debug?
	I2C_DEV_UNK8      =  8u, // Debug?
	I2C_DEV_UNK9      =  9u, // HID debug?
	I2C_DEV_GYRO_OLD  = 10u, // Old 3DS only?
	I2C_DEV_GYRO_NEW  = 11u, // New 3DS only?
	I2C_DEV_UNK12     = 12u, // HID "DebugPad"?
	I2C_DEV_IR        = 13u, // Infrared (IrDA)
	I2C_DEV_EEPROM    = 14u, // Dev unit only?
	I2C_DEV_NFC       = 15u,
	I2C_DEV_QTM       = 16u, // IO expander chip (New 3DS only)
	I2C_DEV_N3DS_HID  = 17u  // C-Stick and ZL/ZR buttons
} I2cDevice;



/**
 * @brief      Initializes the I2C buses. Call this only once.
 */
void I2C_init(void);

/**
 * @brief      Reads data from a I2C register to a buffer.
 *
 * @param[in]  devId    The device ID. Use the enum above.
 * @param[in]  regAddr  The register address.
 * @param      out      The output buffer pointer.
 * @param[in]  size     The read size.
 *
 * @return     Returns true on success and false on failure.
 */
bool I2C_readRegBuf(I2cDevice devId, u8 regAddr, u8 *out, u32 size);

/**
 * @brief      Writes a buffer to a I2C register.
 *
 * @param[in]  devId    The device ID. Use the enum above.
 * @param[in]  regAddr  The register address.
 * @param[in]  in       The input buffer pointer.
 * @param[in]  size     The write size.
 *
 * @return     Returns true on success and false on failure.
 */
bool I2C_writeRegBuf(I2cDevice devId, u8 regAddr, const u8 *in, u32 size);

/**
 * @brief      Reads a byte from a I2C register.
 *
 * @param[in]  devId    The device ID. Use the enum above.
 * @param[in]  regAddr  The register address.
 *
 * @return     Returns the value read on success otherwise 0xFF.
 */
u8 I2C_readReg(I2cDevice devId, u8 regAddr);

/**
 * @brief      Writes a byte to a I2C register.
 *
 * @param[in]  devId    The device ID. Use the enum above.
 * @param[in]  regAddr  The register address.
 * @param[in]  data     The data to write.
 *
 * @return     Returns true on success and false on failure.
 */
bool I2C_writeReg(I2cDevice devId, u8 regAddr, u8 data);

/**
 * @brief      Writes a byte to a I2C register without interrupts.
 *
 * @param[in]  devId    The device ID. Use the enum above.
 * @param[in]  regAddr  The register address.
 * @param[in]  data     The data to write.
 *
 * @return     Returns true on success and false on failure.
 */
bool I2C_writeRegIntSafe(I2cDevice devId, u8 regAddr, u8 data);
