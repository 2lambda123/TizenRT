/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/************************************************************************************
 * fs/driver/mtd/m25px.c
 * Driver for SPI-based M25P1 (128Kbit),  M25P64 (32Mbit), M25P64 (64Mbit), and
 * M25P128 (128Mbit) FLASH (and compatible).
 *
 *   Copyright (C) 2009-2011, 2013 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name tinyara nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ************************************************************************************/

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include <tinyara/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <debug.h>

#include <tinyara/kmalloc.h>
#include <tinyara/fs/ioctl.h>
#include <tinyara/spi/spi.h>
#include <tinyara/fs/mtd.h>

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/
/* Configuration ********************************************************************/
/* Per the data sheet, M25P10 parts can be driven with either SPI mode 0 (CPOL=0 and
 * CPHA=0) or mode 3 (CPOL=1 and CPHA=1). But I have heard that other devices can
 * operated in mode 0 or 1.  So you may need to specify CONFIG_M25P_SPIMODE to
 * select the best mode for your device.  If CONFIG_M25P_SPIMODE is not defined,
 * mode 0 will be used.
 */

#ifndef CONFIG_M25P_SPIMODE
#define CONFIG_M25P_SPIMODE SPIDEV_MODE0
#endif

/* Various manufacturers may have produced the parts.  0x20 is the manufacturer ID
 * for the STMicro MP25x serial FLASH.  If, for example, you are using the a Macronix
 * International MX25 serial FLASH, the correct manufacturer ID would be 0xc2.
 */

#ifndef CONFIG_M25P_MANUFACTURER
#error  "Manufacturer Id not selected for the flash device"
#endif

#ifndef CONFIG_M25P_MEMORY_TYPE
#error  "Memory Type not defined no selected for the flash device"
#endif

/* M25P Registers *******************************************************************/
/* Indentification register values */

#define M25P_MANUFACTURER          CONFIG_M25P_MANUFACTURER
#define M25P_MEMORY_TYPE           CONFIG_M25P_MEMORY_TYPE
#define M25P_RES_ID                0x13
#define M25P_M25P1_CAPACITY        0x11	/* 1 M-bit */
#define M25P_EN25F80_CAPACITY      0x14	/* 8 M-bit */
#define M25P_M25P16_CAPACITY       0x15	/* 16 M-bit */
#define M25P_M25P32_CAPACITY       0x16	/* 32 M-bit */
#define M25P_M25P64_CAPACITY       0x17	/* 64 M-bit */
#define M25P_M25P128_CAPACITY      0x18	/* 128 M-bit */
#define M25P_M25P256_CAPACITY      0x19	/* 256 M-bit */

/* Winbond flash & Memory types */
#define WINBOND_SFLASH_ID          0xEF
#define WINBOND_W25Q128_TYPE       0x40

/*  M25P1 capacity is 131,072 bytes:
 *  (4 sectors) * (32,768 bytes per sector)
 *  (512 pages) * (256 bytes per page)
 */

#define M25P_M25P1_SECTOR_SHIFT    15	/* Sector size 1 << 15 = 65,536 */
#define M25P_M25P1_NSECTORS        4
#define M25P_M25P1_PAGE_SHIFT      8	/* Page size 1 << 8 = 256 */
#define M25P_M25P1_NPAGES          512

/*  EN25F80 capacity is 1,048,576 bytes:
 *  (16 sectors) * (65,536 bytes per sector)
 *  (512 pages) * (256 bytes per page)
 */
#define M25P_EN25F80_SECTOR_SHIFT  16	/* Sector size 1 << 15 = 65,536 */
#define M25P_EN25F80_NSECTORS      16
#define M25P_EN25F80_PAGE_SHIFT    8	/* Page size 1 << 8 = 256 */
#define M25P_EN25F80_NPAGES        4096
#define M25P_EN25F80_SUBSECT_SHIFT 12	/* Sub-Sector size 1 << 12 = 4,096 */
#define M25P_EN25F80_NSUBSECTORS   256

/*  M25P16 capacity is 2,097,152 bytes:
 *  (32 sectors) * (65,536 bytes per sector)
 *  (8192 pages) * (256 bytes per page)
 */

#define M25P_M25P16_SECTOR_SHIFT   16	/* Sector size 1 << 16 = 65,536 */
#define M25P_M25P16_NSECTORS       32
#define M25P_M25P16_PAGE_SHIFT     8	/* Page size 1 << 8 = 256 */
#define M25P_M25P16_NPAGES         8192
#define M25P_M25PX16_SUBSECT_SHIFT 12	/* Sub-Sector size 1 << 12 = 4,096 */

/*  M25P32 capacity is 4,194,304 bytes:
 *  (64 sectors) * (65,536 bytes per sector)
 *  (16384 pages) * (256 bytes per page)
 */

#define M25P_M25P32_SECTOR_SHIFT   16	/* Sector size 1 << 16 = 65,536 */
#define M25P_M25P32_NSECTORS       64
#define M25P_M25P32_PAGE_SHIFT     8	/* Page size 1 << 8 = 256 */
#define M25P_M25P32_NPAGES         16384

/*  M25P64 capacity is 8,338,608 bytes:
 *  (128 sectors) * (65,536 bytes per sector)
 *  (32768 pages) * (256 bytes per page)
 */

#define M25P_M25P64_SECTOR_SHIFT   16	/* Sector size 1 << 16 = 65,536 */
#define M25P_M25P64_NSECTORS       128
#define M25P_M25P64_PAGE_SHIFT     8	/* Page size 1 << 8 = 256 */
#define M25P_M25P64_NPAGES         32768
#define M25P_M25P64_SUBSECT_SHIFT  12

/*  M25P128 capacity is 16,777,216 bytes:
 *  (64 sectors) * (262,144 bytes per sector)
 *  (65536 pages) * (256 bytes per page)
 */

#define M25P_M25P128_SECTOR_SHIFT  18	/* Sector size 1 << 18 = 262,144 */
#define M25P_M25P128_NSECTORS      64
#define M25P_M25P128_PAGE_SHIFT    8	/* Page size 1 << 8 = 256 */
#define M25P_M25P128_NPAGES        65536

/* W25Q128JV  capacity :
 * The W25Q128JV array is organized into 65,536 programmable pages of 256-bytes
 * each. Up to 256 bytes can be programmed at a time. Pages can be erased in groups
 * of 16 (4KB sector erase), groups of 128 (32KB block erase), groups of 256
 * (64KB block erase) or the entire chip (chip erase). The W25Q128JV has 4,096
 * erasable sectors and 256 erasable blocks respectively.
 * The small 4KB sectors allow for greater flexibility in applications
 * that require data and parameter storage
*/
#define W25Q128_SECTOR_SHIFT       16	/* Sector size 1 << 16 = 65,536 */
#define W25Q128_NSECTORS           256  /* Erase blocks = 256 */
#define W25Q128_PAGE_SHIFT         8	/* Page size 1 << 8 = 256 */
#define W25Q128_NPAGES             65536
#define W25Q128_SUBSECT_SHIFT      12	/* Sub sector size 1 << 12 = 4K */


/*  M25P256 capacity is 33,554,432 bytes:
 *  (512 sectors) * (65,536 bytes per sector)
 *  (131072 pages) * (256 bytes per page)
 */

#define M25P_M25P256_SECTOR_SHIFT  16	/* Sector size 1 << 16 = 65,536 */
#define M25P_M25P256_NSECTORS      512
#define M25P_M25P256_PAGE_SHIFT    8	/* Page size 1 << 8 = 256 */
#define M25P_M25P256_NPAGES        131072

/* Instructions */
/*      Command        Value      N Description             Addr Dummy Data   */
#define M25P_WREN      0x06		/* 1 Write Enable              0   0     0     */
#define M25P_WRDI      0x04		/* 1 Write Disable             0   0     0     */
#define M25P_RDID      0x9f		/* 1 Read Identification       0   0     1-3   */
#define M25P_RDSR      0x05		/* 1 Read Status Register      0   0     >=1   */
#define M25P_WRSR      0x01		/* 1 Write Status Register     0   0     1     */
#define M25P_READ      0x03		/* 1 Read Data Bytes           3   0     >=1   */
#define M25P_FAST_READ 0x0b		/* 1 Higher speed read         3   1     >=1   */
#define M25P_PP        0x02		/* 1 Page Program              3   0     1-256 */
#define M25P_SE        0xd8		/* 1 Sector Erase              3   0     0     */
#define M25P_BE        0xc7		/* 1 Bulk Erase                0   0     0     */
#define M25P_DP        0xb9		/* 2 Deep power down           0   0     0     */
#define M25P_RES       0xab		/* 2 Read Electronic Signature 0   3     >=1   */
#define M25P_SSE       0x20		/* 3 Sub-Sector Erase          0   0     0     */

/* NOTE 1: All parts.
 * NOTE 2: M25P632/M25P64
 * NOTE 3: EN25F80.  In EN25F80 terminology, 0xd8 is a block erase and 0x20
 *         is a sector erase.
 */

/* Status register bit definitions */

#define M25P_SR_WIP            (1 << 0)	/* Bit 0: Write in progress bit */
#define M25P_SR_WEL            (1 << 1)	/* Bit 1: Write enable latch bit */
#define M25P_SR_BP_SHIFT       (2)	/* Bits 2-4: Block protect bits */
#define M25P_SR_BP_MASK        (7 << M25P_SR_BP_SHIFT)
#define M25P_SR_BP_NONE      (0 << M25P_SR_BP_SHIFT)	/* Unprotected */
#define M25P_SR_BP_UPPER64th (1 << M25P_SR_BP_SHIFT)	/* Upper 64th */
#define M25P_SR_BP_UPPER32nd (2 << M25P_SR_BP_SHIFT)	/* Upper 32nd */
#define M25P_SR_BP_UPPER16th (3 << M25P_SR_BP_SHIFT)	/* Upper 16th */
#define M25P_SR_BP_UPPER8th  (4 << M25P_SR_BP_SHIFT)	/* Upper 8th */
#define M25P_SR_BP_UPPERQTR  (5 << M25P_SR_BP_SHIFT)	/* Upper quarter */
#define M25P_SR_BP_UPPERHALF (6 << M25P_SR_BP_SHIFT)	/* Upper half */
#define M25P_SR_BP_ALL       (7 << M25P_SR_BP_SHIFT)	/* All sectors */
/* Bits 5-6:  Unused, read zero */
#define M25P_SR_SRWD           (1 << 7)	/* Bit 7: Status register write protect */

#define M25P_DUMMY     0xa5

/************************************************************************************
 * Private Types
 ************************************************************************************/

/* This type represents the state of the MTD device.  The struct mtd_dev_s
 * must appear at the beginning of the definition so that you can freely
 * cast between pointers to struct mtd_dev_s and struct m25p_dev_s.
 */

struct m25p_dev_s {
	struct mtd_dev_s mtd;		/* MTD interface */
	FAR struct spi_dev_s *dev;	/* Saved SPI interface instance */
	uint8_t sectorshift;		/* 16 or 18 */
	uint8_t pageshift;			/* 8 */
	uint16_t nsectors;			/* 128 or 64 */
	uint32_t npages;			/* 32,768 or 65,536 */
#ifdef CONFIG_M25P_SUBSECTOR_ERASE
	uint8_t subsectorshift;		/* 0, 12 or 13 (4K or 8K) */
#endif
};

/************************************************************************************
 * Private Function Prototypes
 ************************************************************************************/

/* Helpers */

static void m25p_lock(FAR struct spi_dev_s *dev);
static inline void m25p_unlock(FAR struct spi_dev_s *dev);
static inline int m25p_readid(struct m25p_dev_s *priv);
static void m25p_waitwritecomplete(struct m25p_dev_s *priv);
static void m25p_writeenable(struct m25p_dev_s *priv);
static inline void m25p_sectorerase(struct m25p_dev_s *priv, off_t offset, uint8_t type);
static inline int m25p_bulkerase(struct m25p_dev_s *priv);
static inline void m25p_pagewrite(struct m25p_dev_s *priv, FAR const uint8_t *buffer, off_t offset);

/* MTD driver methods */

static int m25p_erase(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks);
static ssize_t m25p_bread(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks, FAR uint8_t *buf);
static ssize_t m25p_bwrite(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks, FAR const uint8_t *buf);
static ssize_t m25p_read(FAR struct mtd_dev_s *dev, off_t offset, size_t nbytes, FAR uint8_t *buffer);
#ifdef CONFIG_MTD_BYTE_WRITE
static ssize_t m25p_write(FAR struct mtd_dev_s *dev, off_t offset, size_t nbytes, FAR const uint8_t *buffer);
#endif
static int m25p_ioctl(FAR struct mtd_dev_s *dev, int cmd, unsigned long arg);

/************************************************************************************
 * Private Data
 ************************************************************************************/

/************************************************************************************
 * Private Functions
 ************************************************************************************/

/************************************************************************************
 * Name: m25p_lock
 ************************************************************************************/

static void m25p_lock(FAR struct spi_dev_s *dev)
{
	/* On SPI busses where there are multiple devices, it will be necessary to
	 * lock SPI to have exclusive access to the busses for a sequence of
	 * transfers.  The bus should be locked before the chip is selected.
	 *
	 * This is a blocking call and will not return until we have exclusiv access to
	 * the SPI buss.  We will retain that exclusive access until the bus is unlocked.
	 */

	(void)SPI_LOCK(dev, true);

	/* After locking the SPI bus, the we also need call the setfrequency, setbits, and
	 * setmode methods to make sure that the SPI is properly configured for the device.
	 * If the SPI buss is being shared, then it may have been left in an incompatible
	 * state.
	 */

	SPI_SETMODE(dev, CONFIG_M25P_SPIMODE);
	SPI_SETBITS(dev, 8);
#ifdef CONFIG_SPI_HWFEATURES
	(void)SPI_HWFEATURES(dev, 0);
#endif
	(void)SPI_SETFREQUENCY(dev, 20000000);
}

/************************************************************************************
 * Name: m25p_unlock
 ************************************************************************************/

static inline void m25p_unlock(FAR struct spi_dev_s *dev)
{
	(void)SPI_LOCK(dev, false);
}

/************************************************************************************
 * Name: m25p_readid
 ************************************************************************************/

static inline int m25p_readid(struct m25p_dev_s *priv)
{
	uint16_t manufacturer;
	uint16_t memory;
	uint16_t capacity;

	fvdbg("priv: %p\n", priv);

	/* Lock the SPI bus, configure the bus, and select this FLASH part. */

	m25p_lock(priv->dev);
	SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

	/* Send the "Read ID (RDID)" command and read the first three ID bytes */

	(void)SPI_SEND(priv->dev, M25P_RDID);
	manufacturer = SPI_SEND(priv->dev, M25P_DUMMY);
	memory = SPI_SEND(priv->dev, M25P_DUMMY);
	capacity = SPI_SEND(priv->dev, M25P_DUMMY);

	/* Deselect the FLASH and unlock the bus */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
	m25p_unlock(priv->dev);

	fvdbg("manufacturer: %02x memory: %02x capacity: %02x\n", manufacturer, memory, capacity);

	/* Check for a valid manufacturer and memory type */

	if (manufacturer == M25P_MANUFACTURER && memory == M25P_MEMORY_TYPE) {
		/* Okay.. is it a FLASH capacity that we understand? */

#ifdef CONFIG_M25P_SUBSECTOR_ERASE
		priv->subsectorshift = 0;
#endif

		if (capacity == M25P_M25P1_CAPACITY) {
			/* Save the FLASH geometry */

			priv->sectorshift = M25P_M25P1_SECTOR_SHIFT;
			priv->nsectors = M25P_M25P1_NSECTORS;
			priv->pageshift = M25P_M25P1_PAGE_SHIFT;
			priv->npages = M25P_M25P1_NPAGES;
			return OK;
		} else if (capacity == M25P_EN25F80_CAPACITY) {
			/* Save the FLASH geometry */

			priv->pageshift = M25P_EN25F80_PAGE_SHIFT;
			priv->npages = M25P_EN25F80_NPAGES;
			priv->sectorshift = M25P_EN25F80_SECTOR_SHIFT;
			priv->nsectors = M25P_EN25F80_NSECTORS;
#ifdef CONFIG_M25P_SUBSECTOR_ERASE
			priv->subsectorshift = M25P_EN25F80_SUBSECT_SHIFT;
#endif
			return OK;
		} else if (capacity == M25P_M25P16_CAPACITY) {
			/* Save the FLASH geometry */

			priv->sectorshift = M25P_M25P16_SECTOR_SHIFT;
			priv->nsectors = M25P_M25P16_NSECTORS;
			priv->pageshift = M25P_M25P16_PAGE_SHIFT;
			priv->npages = M25P_M25P16_NPAGES;
#ifdef CONFIG_M25P_SUBSECTOR_ERASE
			priv->subsectorshift = M25P_M25PX16_SUBSECT_SHIFT;
#endif
			return OK;
		} else if (capacity == M25P_M25P32_CAPACITY) {
			/* Save the FLASH geometry */

			priv->sectorshift = M25P_M25P32_SECTOR_SHIFT;
			priv->nsectors = M25P_M25P32_NSECTORS;
			priv->pageshift = M25P_M25P32_PAGE_SHIFT;
			priv->npages = M25P_M25P32_NPAGES;
			return OK;
		} else if (capacity == M25P_M25P64_CAPACITY) {
			/* Save the FLASH geometry */

			priv->sectorshift = M25P_M25P64_SECTOR_SHIFT;
			priv->nsectors = M25P_M25P64_NSECTORS;
			priv->pageshift = M25P_M25P64_PAGE_SHIFT;
			priv->npages = M25P_M25P64_NPAGES;
#if defined(CONFIG_M25P_SUBSECTOR_ERASE)
			priv->subsectorshift = M25P_M25P64_SUBSECT_SHIFT;
#endif
			return OK;
		} else if (capacity == M25P_M25P128_CAPACITY) {
			/* Save the FLASH geometry */

			if (manufacturer == WINBOND_SFLASH_ID &&
				memory == WINBOND_W25Q128_TYPE) {

				priv->sectorshift = W25Q128_SECTOR_SHIFT;
				priv->nsectors = W25Q128_NSECTORS;
				priv->pageshift = W25Q128_PAGE_SHIFT;
				priv->npages = W25Q128_NPAGES;
#if defined(CONFIG_M25P_SUBSECTOR_ERASE)
				priv->subsectorshift = W25Q128_SUBSECT_SHIFT;
#endif

			} else {

				priv->sectorshift = M25P_M25P128_SECTOR_SHIFT;
				priv->nsectors = M25P_M25P128_NSECTORS;
				priv->pageshift = M25P_M25P128_PAGE_SHIFT;
				priv->npages = M25P_M25P128_NPAGES;

			}

			return OK;
		} else if (capacity == M25P_M25P256_CAPACITY) {
			/* Save the FLASH geometry */

			priv->sectorshift = M25P_M25P256_SECTOR_SHIFT;
			priv->nsectors = M25P_M25P256_NSECTORS;
			priv->pageshift = M25P_M25P256_PAGE_SHIFT;
			priv->npages = M25P_M25P256_NPAGES;
			return OK;
		}
	}

	return -ENODEV;
}

/************************************************************************************
 * Name: m25p_waitwritecomplete
 ************************************************************************************/

static void m25p_waitwritecomplete(struct m25p_dev_s *priv)
{
	uint8_t status;

	/* Loop as long as the memory is busy with a write cycle */

	do {
		/* Select this FLASH part */

		SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

		/* Send "Read Status Register (RDSR)" command */

		(void)SPI_SEND(priv->dev, M25P_RDSR);

		/* Send a dummy byte to generate the clock needed to shift out the status */

		status = SPI_SEND(priv->dev, M25P_DUMMY);

		/* Deselect the FLASH */

		SPI_SELECT(priv->dev, SPIDEV_FLASH, false);

		/* Given that writing could take up to few tens of milliseconds, and erasing
		 * could take more.  The following short delay in the "busy" case will allow
		 * other peripherals to access the SPI bus.
		 */

		if ((status & M25P_SR_WIP) != 0) {
			m25p_unlock(priv->dev);
			usleep(1000);
			m25p_lock(priv->dev);
		}
	} while ((status & M25P_SR_WIP) != 0);

	fvdbg("Complete\n");
}

/************************************************************************************
 * Name:  m25p_writeenable
 ************************************************************************************/

static void m25p_writeenable(struct m25p_dev_s *priv)
{
	/* Select this FLASH part */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

	/* Send "Write Enable (WREN)" command */

	(void)SPI_SEND(priv->dev, M25P_WREN);

	/* Deselect the FLASH */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
	fvdbg("Enabled\n");
}

/************************************************************************************
 * Name:  m25p_sectorerase
 ************************************************************************************/

static void m25p_sectorerase(struct m25p_dev_s *priv, off_t sector, uint8_t type)
{
	off_t offset;

#ifdef CONFIG_M25P_SUBSECTOR_ERASE
	if (priv->subsectorshift > 0) {
		offset = sector << priv->subsectorshift;
	} else
#endif
	{
		offset = sector << priv->sectorshift;
	}

	fvdbg("sector: %08lx\n", (long)sector);

	/* Wait for any preceding write to complete.  We could simplify things by
	 * perform this wait at the end of each write operation (rather than at
	 * the beginning of ALL operations), but have the wait first will slightly
	 * improve performance.
	 */

	m25p_waitwritecomplete(priv);

	/* Send write enable instruction */

	m25p_writeenable(priv);

	/* Select this FLASH part */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

	/* Send the "Sector Erase (SE)" or Sub-Sector Erase (SSE) instruction
	 * that was passed in as the erase type.
	 */

	(void)SPI_SEND(priv->dev, type);

	/* Send the sector offset high byte first.  For all of the supported
	 * parts, the sector number is completely contained in the first byte
	 * and the values used in the following two bytes don't really matter.
	 */

	(void)SPI_SEND(priv->dev, (offset >> 16) & 0xff);
	(void)SPI_SEND(priv->dev, (offset >> 8) & 0xff);
	(void)SPI_SEND(priv->dev, offset & 0xff);

	/* Deselect the FLASH */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
	fvdbg("Erased\n");
}

/************************************************************************************
 * Name:  m25p_bulkerase
 ************************************************************************************/

static inline int m25p_bulkerase(struct m25p_dev_s *priv)
{
	fvdbg("priv: %p\n", priv);

	/* Wait for any preceding write to complete.  We could simplify things by
	 * perform this wait at the end of each write operation (rather than at
	 * the beginning of ALL operations), but have the wait first will slightly
	 * improve performance.
	 */

	m25p_waitwritecomplete(priv);

	/* Send write enable instruction */

	m25p_writeenable(priv);

	/* Select this FLASH part */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

	/* Send the "Bulk Erase (BE)" instruction */

	(void)SPI_SEND(priv->dev, M25P_BE);

	/* Deselect the FLASH */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
	fvdbg("Return: OK\n");
	return OK;
}

/************************************************************************************
 * Name:  m25p_pagewrite
 ************************************************************************************/

static inline void m25p_pagewrite(struct m25p_dev_s *priv, FAR const uint8_t *buffer, off_t page)
{
	off_t offset = page << priv->pageshift;

	fvdbg("page: %08lx offset: %08lx\n", (long)page, (long)offset);

	/* Wait for any preceding write to complete.  We could simplify things by
	 * perform this wait at the end of each write operation (rather than at
	 * the beginning of ALL operations), but have the wait first will slightly
	 * improve performance.
	 */

	m25p_waitwritecomplete(priv);

	/* Enable the write access to the FLASH */

	m25p_writeenable(priv);

	/* Select this FLASH part */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

	/* Send "Page Program (PP)" command */

	(void)SPI_SEND(priv->dev, M25P_PP);

	/* Send the page offset high byte first. */

	(void)SPI_SEND(priv->dev, (offset >> 16) & 0xff);
	(void)SPI_SEND(priv->dev, (offset >> 8) & 0xff);
	(void)SPI_SEND(priv->dev, offset & 0xff);

	/* Then write the specified number of bytes */

	SPI_SNDBLOCK(priv->dev, buffer, 1 << priv->pageshift);

	/* Deselect the FLASH: Chip Select high */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
	fvdbg("Written\n");
}

/************************************************************************************
 * Name:  m25p_bytewrite
 ************************************************************************************/

#ifdef CONFIG_MTD_BYTE_WRITE
static inline void m25p_bytewrite(struct m25p_dev_s *priv, FAR const uint8_t *buffer, off_t offset, uint16_t count)
{
	fvdbg("offset: %08lx  count:%d\n", (long)offset, count);

	/* Wait for any preceding write to complete.  We could simplify things by
	 * perform this wait at the end of each write operation (rather than at
	 * the beginning of ALL operations), but have the wait first will slightly
	 * improve performance.
	 */

	m25p_waitwritecomplete(priv);

	/* Enable the write access to the FLASH */

	m25p_writeenable(priv);

	/* Select this FLASH part */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

	/* Send "Page Program (PP)" command */

	(void)SPI_SEND(priv->dev, M25P_PP);

	/* Send the page offset high byte first. */

	(void)SPI_SEND(priv->dev, (offset >> 16) & 0xff);
	(void)SPI_SEND(priv->dev, (offset >> 8) & 0xff);
	(void)SPI_SEND(priv->dev, offset & 0xff);

	/* Then write the specified number of bytes */

	SPI_SNDBLOCK(priv->dev, buffer, count);

	/* Deselect the FLASH: Chip Select high */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
	fvdbg("Written\n");
}
#endif

/************************************************************************************
 * Name: m25p_erase
 ************************************************************************************/

static int m25p_erase(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks)
{
	FAR struct m25p_dev_s *priv = (FAR struct m25p_dev_s *)dev;
	size_t blocksleft = nblocks;

	fvdbg("startblock: %08lx nblocks: %d\n", (long)startblock, (int)nblocks);

	/* Lock access to the SPI bus until we complete the erase */

	m25p_lock(priv->dev);
	while (blocksleft > 0) {
#ifdef CONFIG_M25P_SUBSECTOR_ERASE
		size_t sectorboundry;
		size_t blkper;

		/* If we have a smaller erase size, then we will find as many full
		 * sector erase blocks as possible to speed up the process instead of
		 * erasing everything in smaller chunks.
		 */

		if (priv->subsectorshift > 0) {
			blkper = 1 << (priv->sectorshift - priv->subsectorshift);
			sectorboundry = (startblock + blkper - 1) / blkper;
			sectorboundry *= blkper;

			/* If we are on a sector boundry and have at least a full sector
			 * of blocks left to erase, then we can do a full sector erase.
			 */

			if (startblock == sectorboundry && blocksleft >= blkper) {
				/* Do a full sector erase */

				m25p_sectorerase(priv, startblock, M25P_SE);
				startblock += blkper;
				blocksleft -= blkper;
				continue;
			} else {
				/* Just do a sub-sector erase */

				m25p_sectorerase(priv, startblock, M25P_SSE);
				startblock++;
				blocksleft--;
				continue;
			}
		}
#endif

		/* Not using sub-sector erase.  Erase each full sector */

		m25p_sectorerase(priv, startblock, M25P_SE);
		startblock++;
		blocksleft--;
	}

	m25p_unlock(priv->dev);
	return (int)nblocks;
}

/************************************************************************************
 * Name: m25p_bread
 ************************************************************************************/

static ssize_t m25p_bread(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks, FAR uint8_t *buffer)
{
	FAR struct m25p_dev_s *priv = (FAR struct m25p_dev_s *)dev;
	ssize_t nbytes;

	fvdbg("startblock: %08lx nblocks: %d\n", (long)startblock, (int)nblocks);

	/* On this device, we can handle the block read just like the byte-oriented read */

	nbytes = m25p_read(dev, startblock << priv->pageshift, nblocks << priv->pageshift, buffer);
	if (nbytes > 0) {
		return nbytes >> priv->pageshift;
	}

	return (int)nbytes;
}

/************************************************************************************
 * Name: m25p_bwrite
 ************************************************************************************/

static ssize_t m25p_bwrite(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks, FAR const uint8_t *buffer)
{
	FAR struct m25p_dev_s *priv = (FAR struct m25p_dev_s *)dev;
	size_t blocksleft = nblocks;
	size_t pagesize = 1 << priv->pageshift;

	fvdbg("startblock: %08lx nblocks: %d\n", (long)startblock, (int)nblocks);

	/* Lock the SPI bus and write each page to FLASH */

	m25p_lock(priv->dev);
	while (blocksleft-- > 0) {
		m25p_pagewrite(priv, buffer, startblock);
		buffer += pagesize;
		startblock++;
	}

	m25p_unlock(priv->dev);
	return nblocks;
}

/************************************************************************************
 * Name: m25p_read
 ************************************************************************************/

static ssize_t m25p_read(FAR struct mtd_dev_s *dev, off_t offset, size_t nbytes, FAR uint8_t *buffer)
{
	FAR struct m25p_dev_s *priv = (FAR struct m25p_dev_s *)dev;

	fvdbg("offset: %08lx nbytes: %d\n", (long)offset, (int)nbytes);

	/* Lock the SPI bus NOW because the following call must be executed with
	 * the bus locked.
     */

	m25p_lock(priv->dev);

	/* Wait for any preceding write to complete.  We could simplify things by
	 * perform this wait at the end of each write operation (rather than at
	 * the beginning of ALL operations), but have the wait first will slightly
	 * improve performance.
	 */

	m25p_waitwritecomplete(priv);

	/* Select this FLASH part */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, true);

	/* Send "Read from Memory" instruction */

	(void)SPI_SEND(priv->dev, M25P_READ);

	/* Send the page offset high byte first. */

	(void)SPI_SEND(priv->dev, (offset >> 16) & 0xff);
	(void)SPI_SEND(priv->dev, (offset >> 8) & 0xff);
	(void)SPI_SEND(priv->dev, offset & 0xff);

	/* Then read all of the requested bytes */

	SPI_RECVBLOCK(priv->dev, buffer, nbytes);

	/* Deselect the FLASH and unlock the SPI bus */

	SPI_SELECT(priv->dev, SPIDEV_FLASH, false);
	m25p_unlock(priv->dev);

	fvdbg("return nbytes: %d\n", (int)nbytes);
	return nbytes;
}

/************************************************************************************
 * Name: m25p_write
 ************************************************************************************/

#ifdef CONFIG_MTD_BYTE_WRITE
static ssize_t m25p_write(FAR struct mtd_dev_s *dev, off_t offset, size_t nbytes, FAR const uint8_t *buffer)
{
	FAR struct m25p_dev_s *priv = (FAR struct m25p_dev_s *)dev;
	int startpage;
	int endpage;
	int count;
	int index;
	int pagesize;
	int bytestowrite;

	fvdbg("offset: %08lx nbytes: %d\n", (long)offset, (int)nbytes);

	/* We must test if the offset + count crosses one or more pages
	 * and perform individual writes.  The devices can only write in
	 * page increments.
	 */

	startpage = offset / (1 << priv->pageshift);
	endpage = (offset + nbytes) / (1 << priv->pageshift);

	if (startpage == endpage) {
		/* All bytes within one programmable page.  Just do the write. */

		m25p_bytewrite(priv, buffer, offset, nbytes);
	} else {
		/* Write the 1st partial-page */

		count = nbytes;
		pagesize = (1 << priv->pageshift);
		bytestowrite = pagesize - (offset & (pagesize - 1));
		m25p_bytewrite(priv, buffer, offset, bytestowrite);

		/* Update offset and count */

		offset += bytestowrite;
		count -= bytestowrite;
		index = bytestowrite;

		/* Write full pages */

		while (count >= pagesize) {
			m25p_bytewrite(priv, &buffer[index], offset, pagesize);

			/* Update offset and count */

			offset += pagesize;
			count -= pagesize;
			index += pagesize;
		}

		/* Now write any partial page at the end */

		if (count > 0) {
			m25p_bytewrite(priv, &buffer[index], offset, count);
		}
	}

	return nbytes;
}
#endif							/* CONFIG_MTD_BYTE_WRITE */

/************************************************************************************
 * Name: m25p_ioctl
 ************************************************************************************/

static int m25p_ioctl(FAR struct mtd_dev_s *dev, int cmd, unsigned long arg)
{
	FAR struct m25p_dev_s *priv = (FAR struct m25p_dev_s *)dev;
	int ret = -EINVAL;			/* Assume good command with bad parameters */

	fvdbg("cmd: %d \n", cmd);

	switch (cmd) {
	case MTDIOC_GEOMETRY: {
		FAR struct mtd_geometry_s *geo = (FAR struct mtd_geometry_s *)((uintptr_t)arg);
		if (geo) {
			/* Populate the geometry structure with information need to know
			 * the capacity and how to access the device.
			 *
			 * NOTE: that the device is treated as though it where just an array
			 * of fixed size blocks.  That is most likely not true, but the client
			 * will expect the device logic to do whatever is necessary to make it
			 * appear so.
			 */

			geo->blocksize = (1 << priv->pageshift);
#ifdef CONFIG_M25P_SUBSECTOR_ERASE
			if (priv->subsectorshift > 0) {
				geo->erasesize = (1 << priv->subsectorshift);
				geo->neraseblocks = priv->nsectors * (1 << (priv->sectorshift - priv->subsectorshift));
			} else
#endif
			{
				geo->erasesize = (1 << priv->sectorshift);
				geo->neraseblocks = priv->nsectors;
			}

			ret = OK;

			fvdbg("blocksize: %d erasesize: %d neraseblocks: %d\n", geo->blocksize, geo->erasesize, geo->neraseblocks);
		}
	}
	break;

	case MTDIOC_BULKERASE: {
		/* Erase the entire device */

		m25p_lock(priv->dev);
		ret = m25p_bulkerase(priv);
		m25p_waitwritecomplete(priv);
		m25p_unlock(priv->dev);
	}
	break;

	case MTDIOC_XIPBASE:
	default:
		ret = -ENOTTY;			/* Bad command */
		break;
	}

	fvdbg("return %d\n", ret);
	return ret;
}

/************************************************************************************
 * Public Functions
 ************************************************************************************/

/************************************************************************************
 * Name: m25p_initialize
 *
 * Description:
 *   Create an initialize MTD device instance.  MTD devices are not registered
 *   in the file system, but are created as instances that can be bound to
 *   other functions (such as a block or character driver front end).
 *
 ************************************************************************************/

FAR struct mtd_dev_s *m25p_initialize(FAR struct spi_dev_s *dev)
{
	FAR struct m25p_dev_s *priv;
	int ret;

	fvdbg("dev: %p\n", dev);

	/* Allocate a state structure (we allocate the structure instead of using
	 * a fixed, static allocation so that we can handle multiple FLASH devices.
	 * The current implementation would handle only one FLASH part per SPI
	 * device (only because of the SPIDEV_FLASH definition) and so would have
	 * to be extended to handle multiple FLASH parts on the same SPI bus.
	 */

	priv = (FAR struct m25p_dev_s *)kmm_zalloc(sizeof(struct m25p_dev_s));
	if (priv) {
		/* Initialize the allocated structure. (unsupported methods were
		 * nullified by kmm_zalloc).
		 */

		priv->mtd.erase = m25p_erase;
		priv->mtd.bread = m25p_bread;
		priv->mtd.bwrite = m25p_bwrite;
		priv->mtd.read = m25p_read;
#ifdef CONFIG_MTD_BYTE_WRITE
		priv->mtd.write = m25p_write;
#endif
		priv->mtd.ioctl = m25p_ioctl;
#ifdef CONFIG_MTD_REGISTRATION
		priv->mtd.name = "m25px";
#endif
		priv->dev = dev;

		/* Deselect the FLASH */

		SPI_SELECT(dev, SPIDEV_FLASH, false);

		/* Identify the FLASH chip and get its capacity */

		ret = m25p_readid(priv);
		if (ret != OK) {
			/* Unrecognized! Discard all of that work we just did and return NULL */

			fdbg("Unrecognized\n");
			kmm_free(priv);
			priv = NULL;
		}
	}

	/* Return the implementation-specific state structure as the MTD device */

	fvdbg("Return %p\n", priv);
	return (FAR struct mtd_dev_s *)priv;
}
