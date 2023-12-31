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
/****************************************************************************
 * arch/arm/include/arch.h
 *
 *   Copyright (C) 2007-2009, 2014 Gregory Nutt. All rights reserved.
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
 ****************************************************************************/

/* This file should never be included directed but, rather,
 * only indirectly through tinyara/arch.h
 */

#ifndef __ARCH_ARM_INCLUDE_ARCH_H
#define __ARCH_ARM_INCLUDE_ARCH_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#ifndef __ASSEMBLY__
#include <stdint.h>
#include <tinyara/pgalloc.h>
#include <tinyara/addrenv.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_PIC

/* This identifies the register the is used by the processor as the PIC base
 * register.  It is usually r9 or r10
 */

#define PIC_REG         r10
#define PIC_REG_STRING "r10"

/* Macros to get and set the PIC base register.  picbase is assumed to be
 * of type (void*) and that it will fit into a uint32_t.  These must be
 * inline so that they will be compatible with the ABIs rules for
 * preserving the PIC register
 */

#define up_getpicbase(ppicbase) \
	do { \
		uint32_t picbase; \
		__asm__ \
		( \
			"\tmov %0, " PIC_REG_STRING "\n\t" \
			: "=r"(picbase) \
		); \
		*ppicbase = (FAR void*)picbase; \
	} while (0)

#define up_setpicbase(picbase) \
	do { \
		uint32_t _picbase = (uint32_t)picbase; \
		__asm__ \
		( \
			"\tmov " PIC_REG_STRING ", %0\n\t" \
			: : "r"(_picbase) : PIC_REG_STRING \
		); \
	} while (0)

#endif							/* CONFIG_PIC */

#ifdef CONFIG_ARCH_ADDRENV
#if CONFIG_MM_PGSIZE != 4096
#error Only pages sizes of 4096 are currently supported (CONFIG_ARCH_ADDRENV)
#endif

/* Convert 4KiB pages to 1MiB sections */

#define __PG2SECT_SHIFT     (20 - MM_PGSHIFT)
#define __PG2SECT_MASK      ((1 << __PG2SECT_SHIFT) - 1)

#define ARCH_PG2SECT(p)     (((p) + __PG2SECT_MASK) >> __PG2SECT_SHIFT)
#define ARCH_SECT2PG(s)     ((s) << __PG2SECT_SHIFT)

#define ARCH_TEXT_NSECTS    ARCH_PG2SECT(CONFIG_ARCH_TEXT_NPAGES)
#define ARCH_DATA_NSECTS    ARCH_PG2SECT(CONFIG_ARCH_DATA_NPAGES)
#define ARCH_HEAP_NSECTS    ARCH_PG2SECT(CONFIG_ARCH_HEAP_NPAGES)

#ifdef CONFIG_MM_SHM
#define ARCH_SHM_NSECTS   ARCH_PG2SECT(ARCH_SHM_MAXPAGES)
#endif

#ifdef CONFIG_ARCH_STACK_DYNAMIC
#define ARCH_STACK_NSECTS ARCH_PG2SECT(CONFIG_ARCH_STACK_NPAGES)
#endif
#endif							/* CONFIG_ARCH_ADDRENV */

/****************************************************************************
 * Inline functions
 ****************************************************************************/

static inline uint32_t up_getsp(void)
{
	uint32_t sp;
	__asm__
	(
		"\tmov %0, sp\n\t"
		: "=r"(sp)
	);
	return sp;
}

#if defined(CONFIG_ARCH_ARMV8M_FAMILY)
static inline uint32_t get_PSPLIM(void)
{
	uint32_t result;
	__asm volatile ("MRS %0, psplim"  : "=r" (result));
	return result;
}
static inline void set_PSPLIM(uint32_t PSP_limit)
{
	__asm volatile ("MSR psplim, %0" : : "r" (PSP_limit));
}
#else
#define get_PSPLIM(void)					(0)
#define set_PSPLIM(PSP_limit)				(0)
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifdef CONFIG_ARCH_ADDRENV
/* The task group resources are retained in a single structure, task_group_s
 * that is defined in the header file os/include/tinyara/sched.h. The type
 * group_addrenv_t must be defined by platform specific logic in
 * os/arch/<architecture>/include/arch.h.
 *
 * These tables would hold the physical address of the level 2 page tables.
 * All would be initially NULL and would not be backed up with physical memory
 * until mappings in the level 2 page table are required.
 */

struct group_addrenv_s {
	/* Level 1 page table entries for each group section */

	FAR uintptr_t *text[ARCH_TEXT_NSECTS];
	FAR uintptr_t *data[ARCH_DATA_NSECTS];
#ifdef CONFIG_BUILD_KERNEL
	FAR uintptr_t *heap[ARCH_HEAP_NSECTS];
#ifdef CONFIG_MM_SHM
	FAR uintptr_t *shm[ARCH_SHM_NSECTS];
#endif

	/* Initial heap allocation (in bytes).  This exists only provide an
	 * indirect path for passing the size of the initial heap to the heap
	 * initialization logic.  These operations are separated in time and
	 * architecture.  REVISIT:  I would like a better way to do this.
	 */

	size_t heapsize;
#endif
};

typedef struct group_addrenv_s group_addrenv_t;

/* This type is used when the OS needs to temporarily instantiate a
 * different address environment.  Used in the implementation of
 *
 *   int up_addrenv_select(group_addrenv_t addrenv, save_addrenv_t *oldenv);
 *   int up_addrenv_restore(save_addrenv_t oldenv);
 *
 * In this case, the saved valued in the L1 page table are returned
 */

struct save_addrenv_s {
	FAR uint32_t text[ARCH_TEXT_NSECTS];
	FAR uint32_t data[ARCH_DATA_NSECTS];
#ifdef CONFIG_BUILD_KERNEL
	FAR uint32_t heap[ARCH_HEAP_NSECTS];
#ifdef CONFIG_MM_SHM
	FAR uint32_t shm[ARCH_SHM_NSECTS];
#endif
#endif
};

typedef struct save_addrenv_s save_addrenv_t;
#endif

/****************************************************************************
 * Public Variables
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif							/* __ARCH_ARM_INCLUDE_ARCH_H */
