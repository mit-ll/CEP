# Reset - center button
set_property PACKAGE_PIN AV39 [get_ports rst_pad_i]
set_property IOSTANDARD LVCMOS18 [get_ports rst_pad_i]

# 200 MHz system clock
set_property IOSTANDARD LVDS [get_ports sys_clk_in_p]
set_property PACKAGE_PIN E19 [get_ports sys_clk_in_p]
set_property PACKAGE_PIN E18 [get_ports sys_clk_in_n]
set_property IOSTANDARD LVDS [get_ports sys_clk_in_n]

#UART
set_property PACKAGE_PIN AU36 [get_ports uart0_stx_pad_o]
set_property IOSTANDARD LVCMOS18 [get_ports uart0_stx_pad_o]
set_property PACKAGE_PIN AU33 [get_ports uart0_srx_pad_i]
set_property IOSTANDARD LVCMOS18 [get_ports uart0_srx_pad_i]
set_property PACKAGE_PIN AT32 [get_ports uart0_cts_pad_i]
set_property IOSTANDARD LVCMOS18 [get_ports uart0_cts_pad_i]
set_property PACKAGE_PIN AR34 [get_ports uart0_rts_pad_o]
set_property IOSTANDARD LVCMOS18 [get_ports uart0_rts_pad_o]

#LED
set_property PACKAGE_PIN AM39 [get_ports GPIO_LED[0]]
set_property IOSTANDARD LVCMOS18 [get_ports GPIO_LED[0]]
set_property PACKAGE_PIN AN39 [get_ports GPIO_LED[1]]
set_property IOSTANDARD LVCMOS18 [get_ports GPIO_LED[1]]
set_property PACKAGE_PIN AR37 [get_ports GPIO_LED[2]]
set_property IOSTANDARD LVCMOS18 [get_ports GPIO_LED[2]]
set_property PACKAGE_PIN AT37 [get_ports GPIO_LED[3]]
set_property IOSTANDARD LVCMOS18 [get_ports GPIO_LED[3]]
set_property PACKAGE_PIN AR35 [get_ports GPIO_LED[4]]
set_property IOSTANDARD LVCMOS18 [get_ports GPIO_LED[4]]
set_property PACKAGE_PIN AP41 [get_ports GPIO_LED[5]]
set_property IOSTANDARD LVCMOS18 [get_ports GPIO_LED[5]]
set_property PACKAGE_PIN AP42 [get_ports GPIO_LED[6]]
set_property IOSTANDARD LVCMOS18 [get_ports GPIO_LED[6]]
set_property PACKAGE_PIN AU39 [get_ports GPIO_LED[7]]
set_property IOSTANDARD LVCMOS18 [get_ports GPIO_LED[7]]

#PUSH BUTTONS
set_property PACKAGE_PIN AP40 [get_ports button_S]
set_property IOSTANDARD LVCMOS18 [get_ports button_S]
set_property PACKAGE_PIN AR40 [get_ports button_N]
set_property IOSTANDARD LVCMOS18 [get_ports button_N]
set_property PACKAGE_PIN AU38 [get_ports button_E]
set_property IOSTANDARD LVCMOS18 [get_ports button_E]
set_property PACKAGE_PIN AW40 [get_ports button_W]
set_property IOSTANDARD LVCMOS18 [get_ports button_W]

create_clock -period 5.000 -name sys_clk_in_p -waveform {0.000 2.500} [get_ports -filter { NAME =~  "*sys_clk_in_p*" && DIRECTION == "IN" }]
