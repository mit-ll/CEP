# Reset
set_property PACKAGE_PIN AV40 [get_ports rst_n_pad_i]
set_property IOSTANDARD LVCMOS18 [get_ports rst_n_pad_i]

# 200 MHz system clock
set_property PACKAGE_PIN E19 [get_ports sys_clk_in_p]
set_property IOSTANDARD LVDS [get_ports sys_clk_in_p]
set_property PACKAGE_PIN E18 [get_ports sys_clk_in_n]
set_property IOSTANDARD LVDS [get_ports sys_clk_in_n]

#UART
set_property PACKAGE_PIN AU36 [get_ports uart0_srx_pad_i]
set_property IOSTANDARD LVCMOS18 [get_ports uart0_srx_pad_i]
set_property PACKAGE_PIN AU33 [get_ports uart0_stx_pad_o]
set_property IOSTANDARD LVCMOS18 [get_ports uart0_stx_pad_o]
set_property PACKAGE_PIN AT32 [get_ports uart0_rts_pad_o]
set_property IOSTANDARD LVCMOS18 [get_ports uart0_rts_pad_o]
set_property PACKAGE_PIN AR34 [get_ports uart0_cts_pad_i]
set_property IOSTANDARD LVCMOS18 [get_ports uart0_cts_pad_i]

