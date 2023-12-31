/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
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
 * lib/libc/machine/arm/armv7-r/arm_elf.c
 *
 *   Copyright (C) 2015, 2017 Gregory Nutt. All rights reserved.
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <stdlib.h>
#include <errno.h>
#include <debug.h>

#include <arch/elf.h>
#include <tinyara/elf.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_checkarch
 *
 * Description:
 *   Given the ELF header in 'ehdr', verify that the ELF file is appropriate
 *   for the current, configured architecture.  Every architecture that uses
 *   the ELF loader must provide this function.
 *
 * Input Parameters:
 *   ehdr - The ELF header read from the ELF file.
 *
 * Returned Value:
 *   True if the architecture supports this ELF file.
 *
 ****************************************************************************/

bool up_checkarch(FAR const Elf32_Ehdr *ehdr)
{
	/* Make sure it's an ARM executable */

	if (ehdr->e_machine != EM_ARM) {
		berr("ERROR: Not for ARM: e_machine=%04x\n", ehdr->e_machine);
		return false;
	}

	/* Make sure that 32-bit objects are supported */

	if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
		berr("ERROR: Need 32-bit objects: e_ident[EI_CLASS]=%02x\n", ehdr->e_ident[EI_CLASS]);
		return false;
	}

	/* Verify endian-ness */

#ifdef CONFIG_ENDIAN_BIG
	if (ehdr->e_ident[EI_DATA] != ELFDATA2MSB)
#else
	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
#endif
	{
		berr("ERROR: Wrong endian-ness: e_ident[EI_DATA]=%02x\n", ehdr->e_ident[EI_DATA]);
		return false;
	}

	/* Make sure the entry point address is properly aligned */

	if ((ehdr->e_entry & 3) != 0) {
		berr("ERROR: Entry point is not properly aligned: %08x\n", ehdr->e_entry);
		return false;
	}

	/* TODO:  Check ABI here. */
	return true;
}

/****************************************************************************
 * Name: up_relocate and up_relocateadd
 *
 * Description:
 *   Perform on architecture-specific ELF relocation.  Every architecture
 *   that uses the ELF loader must provide this function.
 *
 * Input Parameters:
 *   rel - The relocation type
 *   sym - The ELF symbol structure containing the fully resolved value.
 *         There are a few relocation types for a few architectures that do
 *         not require symbol information.  For those, this value will be
 *         NULL.  Implementations of these functions must be able to handle
 *         that case.
 *   addr - The address that requires the relocation.
 *
 * Returned Value:
 *   Zero (OK) if the relocation was successful.  Otherwise, a negated errno
 *   value indicating the cause of the relocation failure.
 *
 ****************************************************************************/

int up_relocate(FAR const Elf32_Rel *rel, FAR const Elf32_Sym *sym, uintptr_t addr)
{
	int32_t offset;
	unsigned int relotype;

	/* All relocations except R_ARM_V4BX depend upon having valid symbol
	 * information.
	 */

	relotype = ELF32_R_TYPE(rel->r_info);
	if (sym == NULL && relotype != R_ARM_NONE && relotype != R_ARM_V4BX) {
		return -EINVAL;
	}

	/* Handle the relocation by relocation type */

	switch (relotype) {
	case R_ARM_NONE: {
		/* No relocation */
	}
	break;

	case R_ARM_PC24:
	case R_ARM_CALL:
	case R_ARM_JUMP24: {
		binfo("Performing PC24 [%d] link at addr %08lx [%08lx] to sym '%p' st_value=%08lx\n", ELF32_R_TYPE(rel->r_info), (long)addr, (long)(*(uint32_t *)addr), sym, (long)sym->st_value);

		offset = (*(uint32_t *)addr & 0x00ffffff) << 2;
		if (offset & 0x02000000) {
			offset -= 0x04000000;
		}

		offset += sym->st_value - addr;
		if (offset & 3 || offset < (int32_t)0xfe000000 || offset >= (int32_t)0x02000000) {
			berr("ERROR:   ERROR: PC24 [%d] relocation out of range, offset=%08lx\n", ELF32_R_TYPE(rel->r_info), offset);

			return -EINVAL;
		}

		offset >>= 2;

		*(uint32_t *)addr &= 0xff000000;
		*(uint32_t *)addr |= offset & 0x00ffffff;
	}
	break;

	case R_ARM_ABS32:
	case R_ARM_TARGET1: {	/* New ABI:  TARGET1 always treated as ABS32 */
		binfo("Performing ABS32 link at addr=%08lx [%08lx] to sym=%p st_value=%08lx\n", (long)addr, (long)(*(uint32_t *)addr), sym, (long)sym->st_value);

		*(uint32_t *)addr += sym->st_value;
	}
	break;

	case R_ARM_V4BX: {
		binfo("Performing V4BX link at addr=%08lx [%08lx]\n", (long)addr, (long)(*(uint32_t *)addr));

		/* Preserve only Rm and the condition code */

		*(uint32_t *)addr &= 0xf000000f;

		/* Change instruction to 'mov pc, Rm' */

		*(uint32_t *)addr |= 0x01a0f000;
	}
	break;

	case R_ARM_PREL31: {
		binfo("Performing PREL31 link at addr=%08lx [%08lx] to sym=%p st_value=%08lx\n", (long)addr, (long)(*(uint32_t *)addr), sym, (long)sym->st_value);

		offset = *(uint32_t *)addr + sym->st_value - addr;
		*(uint32_t *)addr = offset & 0x7fffffff;
	}
	break;

	case R_ARM_MOVW_ABS_NC:
	case R_ARM_MOVT_ABS: {
		binfo("Performing MOVx_ABS [%d] link at addr=%08lx [%08lx] to sym=%p st_value=%08lx\n", ELF32_R_TYPE(rel->r_info), (long)addr, (long)(*(uint32_t *)addr), sym, (long)sym->st_value);

		offset = *(uint32_t *)addr;
		offset = ((offset & 0xf0000) >> 4) | (offset & 0xfff);

		offset += sym->st_value;
		if (ELF32_R_TYPE(rel->r_info) == R_ARM_MOVT_ABS) {
			offset >>= 16;
		}

		*(uint32_t *)addr &= 0xfff0f000;
		*(uint32_t *)addr |= ((offset & 0xf000) << 4) | (offset & 0x0fff);
	}
	break;

	default:
		berr("ERROR: Unsupported relocation: %d\n", ELF32_R_TYPE(rel->r_info));
		return -EINVAL;
	}

	return OK;
}

int up_relocateadd(FAR const Elf32_Rela *rel, FAR const Elf32_Sym *sym, uintptr_t addr)
{
	berr("ERROR: RELA relocation not supported\n");
	return -ENOSYS;
}
