/************************************************************************************
 * arch/arm/src/stm32h745/chip.h
 *
 *   Copyright (C) 2015 Sebastien Lorquet. All rights reserved.
 *   Author: Sebastien Lorquet <sebastien@lorquet.fr>
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
 * 3. Neither the name NuttX nor the names of its contributors may be
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

#ifndef __ARCH_ARM_SRC_STM32H745_CHIP_H
#define __ARCH_ARM_SRC_STM32H745_CHIP_H

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include <tinyara/config.h>

/* Include the memory map and the chip definitions file.  Other chip hardware files
 * should then include this file for the proper setup.
 */

#include <arch/irq.h>
#include <arch/stm32h745/chip.h>

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/

/* If the common ARMv7-M vector handling logic is used, then it expects the
 * following definition in this file that provides the number of supported external
 * interrupts which, for this architecture, is provided in the arch/stm32l4/chip.h
 * header file.
 */

#define ARMV7M_PERIPHERAL_INTERRUPTS STM32H745_IRQ_NEXTINTS

/* Cache line sizes (in bytes) for the STM32L4 */

#define ARMV7M_DCACHE_LINESIZE 0  /* no cache */
#define ARMV7M_ICACHE_LINESIZE 0  /* no cache */

#endif /* __ARCH_ARM_SRC_STM32H745_CHIP_H */
