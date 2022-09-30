#!/usr/bin/env bash

xterm -title "GDB Server" -hold -e "/opt/SEGGER/JLink_V698b/JLinkGDBServer -device cortex-m33 -if SWD -scriptfile AP0_KM4.JLinkScript -port 2335" &
xterm -title "GDB Client" -hold -e "arm-none-eabi-gdb -x rtl_gdb_debug.txt"
