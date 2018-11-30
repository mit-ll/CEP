onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {BEGIN: TB}
add wave -noupdate /tb_orpsoc_top/clk
add wave -noupdate /tb_orpsoc_top/rst_n
add wave -noupdate /tb_orpsoc_top/clk_n
add wave -noupdate /tb_orpsoc_top/clk_p
add wave -noupdate /tb_orpsoc_top/uart_stx_pad_o
add wave -noupdate /tb_orpsoc_top/uart_srx_pad_i
add wave -noupdate /tb_orpsoc_top/uart_cts_pad_i
add wave -noupdate /tb_orpsoc_top/uart_rts_pad_o
add wave -noupdate /tb_orpsoc_top/button_W
add wave -noupdate /tb_orpsoc_top/button_N
add wave -noupdate /tb_orpsoc_top/button_E
add wave -noupdate /tb_orpsoc_top/button_S
add wave -noupdate -divider {END: TB}
add wave -noupdate -divider {BEGIN: CEP}
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/sys_clk_in_p
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/sys_clk_in_n
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/rst_n_pad_i
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/uart_srx_pad_i
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/uart_stx_pad_o
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/uart_rts_pad_o
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/uart_cts_pad_i
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/GPIO_LED
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/button_W
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/button_N
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/button_E
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/button_S
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/core_clk
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/core_rst
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/gps_clk_fast
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/gps_clk_slow
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/mor1kx_pic_ints
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/mor1kx_dbg_rst
add wave -noupdate /tb_orpsoc_top/orpsoc_top_inst/uart_irq
add wave -noupdate -divider {END: CEP}
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {1999000000 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 150
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ns
update
WaveRestoreZoom {1998537560 ps} {2000076980 ps}
