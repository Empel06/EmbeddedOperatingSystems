# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\12302093\AppData\Roaming\Xilinx\Vivado\myproj\Vitis2\VivadoHDMIUDP\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\12302093\AppData\Roaming\Xilinx\Vivado\myproj\Vitis2\VivadoHDMIUDP\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {VivadoHDMIUDP}\
-hw {C:\Users\12302093\AppData\Roaming\Xilinx\Vivado\myproj\VivadoHDMIUDP.xsa}\
-out {C:/Users/12302093/AppData/Roaming/Xilinx/Vivado/myproj/Vitis2}

platform write
domain create -name {freertos10_xilinx_ps7_cortexa9_0} -display-name {freertos10_xilinx_ps7_cortexa9_0} -os {freertos10_xilinx} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {freertos_lwip_udp_perf_server}
platform generate -domains 
platform active {VivadoHDMIUDP}
domain active {zynq_fsbl}
domain active {freertos10_xilinx_ps7_cortexa9_0}
platform generate -quick
