/******************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/
/****************************************************************************
 * configs/esp32_DevKitC/scripts/esp32_flash.ld
 ****************************************************************************/

/* Default entry point: */

ENTRY(__start);

SECTIONS
{
  /* Send .iram0 code to iram */

  .iram0.vectors :
  {
    /* Vectors go to IRAM */

    _init_start = ABSOLUTE(.);

    /* Vectors according to builds/RF-2015.2-win32/esp108_v1_2_s5_512int_2/config.html */

    . = 0x0;
    KEEP (*(.window_vectors.text));
    . = 0x180;
    KEEP (*(.xtensa_level2_vector.text));
    . = 0x1c0;
    KEEP (*(.xtensa_level3_vector.text));
    . = 0x200;
    KEEP (*(.xtensa_level4_vector.text));
    . = 0x240;
    KEEP (*(.xtensa_level5_vector.text));
    . = 0x280;
    KEEP (*(.debug_exception_vector.text));
    . = 0x2c0;
    KEEP (*(.nmi_vector.text));
    . = 0x300;
    KEEP (*(.kernel_exception_vector.text));
    . = 0x340;
    KEEP (*(.user_exception_vector.text));
    . = 0x3c0;
    KEEP (*(.double_exception_vector.text));
    . = 0x400;
    *(.*_vector.literal)

    . = ALIGN (16);
    *(.entry.text)
    *(.init.literal)
    *(.init)
    _init_end = ABSOLUTE(.);
  } > iram0_0_seg

  .iram0.text :
  {
    /* Code marked as runnning out of IRAM */

    _iram_text_start = ABSOLUTE(.);
    *(.iram1 .iram1.*)
    *libesp32.a:panic.*(.literal .text .literal.* .text.*)
    *libesp32.a:core_dump.*(.literal .text .literal.* .text.*)
    *librtc.a:(.literal .text .literal.* .text.*)
    *libsoc.a:(.literal .text .literal.* .text.*)
    *libspiflash.a:spi_flash_rom_patch.*(.literal .text .literal.* .text.*)
    _iram_text_end = ABSOLUTE(.);
  } > iram0_0_seg

  /* Shared RAM */

  .dram0.data :
  {
    /* .data initialized on power-up in ROMed configurations. */

    _sdata = ABSOLUTE(.);
    KEEP (*(.data))
    KEEP (*(.data.*))
    KEEP (*(.gnu.linkonce.d.*))
    KEEP (*(.data1))
    KEEP (*(.sdata))
    KEEP (*(.sdata.*))
    KEEP (*(.gnu.linkonce.s.*))
    KEEP (*(.sdata2))
    KEEP (*(.sdata2.*))
    KEEP (*(.gnu.linkonce.s2.*))
    KEEP (*(.jcr))
    *(.dram1 .dram1.*)
    *libesp32.a:panic.*(.rodata .rodata.*)
    *libphy.a:(.rodata .rodata.*)
    *libsoc.a:rtc_clk.*(.rodata .rodata.*)
    _edata = ABSOLUTE(.);

  } >dram0_0_seg

  .dram0.bss (NOLOAD) :
  {
    /* .bss initialized on power-up */

    . = ALIGN (8);
    _sbss = ABSOLUTE(.);
    *(.dynsbss)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(.sbss2)
    *(.sbss2.*)
    *(.gnu.linkonce.sb2.*)
    *(.dynbss)
    KEEP (*(.bss))
    *(.bss.*)
    *(.share.mem)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN (8);
    _ebss = ABSOLUTE(.);
    _sidle_stack = ABSOLUTE(.);
    . = . + CONFIG_IDLETHREAD_STACKSIZE ;
    /* Heap start address in internal RAM */
    _sint_heap = ABSOLUTE(.);

    /* Uninitialized .bss */
    *(.noinit)

    . = ALIGN(4);
    /* Heap starts at the end of .data */
    _sheap = ABSOLUTE(.);

  } >dram0_0_seg

  .flash.rodata :
  {
    _srodata = ABSOLUTE(.);
    *(.rodata)
    *(.rodata.*)
    *(.irom1.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    __XT_EXCEPTION_TABLE_ = ABSOLUTE(.);
    *(.xt_except_table)
    *(.gcc_except_table .gcc_except_table.*)
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    . = (. + 3) & ~ 3;
    __eh_frame = ABSOLUTE(.);
    KEEP(*(.eh_frame))
    . = (. + 7) & ~ 3;
    /* C++ constructor and destructor tables, properly ordered: */

    _sinit = ABSOLUTE(.);
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    _einit = ABSOLUTE(.);
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))

    /* C++ exception handlers table: */

    __XT_EXCEPTION_DESCS_ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
    _erodata = ABSOLUTE(.);
    /* Literals are also RO data. */
    _lit4_start = ABSOLUTE(.);
    *(*.lit4)
    *(.lit4.*)
    *(.gnu.linkonce.lit4.*)
    _lit4_end = ABSOLUTE(.);
    . = ALIGN(4);
  } >drom0_0_seg

  .flash.text :
  {
    _stext_flash = .;
    _text_start = ABSOLUTE(.);
    *(.literal .text .literal.* .text.* .stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.irom0.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.fini.literal)
    *(.fini)
    *(.gnu.version)
    _text_end = ABSOLUTE(.);
    _etext_flash = .;
  } >iram0_2_seg

  .rtc.text :
  {
    . = ALIGN(4);
    *(.rtc.literal .rtc.text)
  } >rtc_iram_seg

  .rtc.data :
  {
    *(.rtc.data)
    *(.rtc.rodata)
  } > rtc_slow_seg
}
