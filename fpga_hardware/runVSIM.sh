vsim +notimingchecks -c -noglitch -t ps -novopt work.orpsoc_testbench
#add wave -r /orpsoc_testbench/dut/xilinx_ddr2_0/xilinx_ddr2_if0/ddr2_mig0/u_ddr2_top_0/*
#run -all
