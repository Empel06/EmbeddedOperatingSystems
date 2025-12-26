# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\Users\12302093\AppData\Roaming\Xilinx\Vivado\myproj\Vitis4\Pong_system\_ide\scripts\debugger_pong-default.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source C:\Users\12302093\AppData\Roaming\Xilinx\Vivado\myproj\Vitis4\Pong_system\_ide\scripts\debugger_pong-default.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Xilinx HW-FTDI-TEST FT2232H 1234-tulA" && level==0 && jtag_device_ctx=="jsn-HW-FTDI-TEST FT2232H-1234-tulA-23727093-0"}
fpga -file C:/Users/12302093/AppData/Roaming/Xilinx/Vivado/myproj/Vitis4/Pong/_ide/bitstream/hdmi_vdma.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw C:/Users/12302093/AppData/Roaming/Xilinx/Vivado/myproj/Vitis4/hdmi_vdma/export/hdmi_vdma/hw/hdmi_vdma.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source C:/Users/12302093/AppData/Roaming/Xilinx/Vivado/myproj/Vitis4/Pong/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
configparams force-mem-access 0
bpadd -addr &main
