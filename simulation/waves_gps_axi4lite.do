onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {BEGIN: TB}
add wave -noupdate /tb_axi_lite_gps/clk
add wave -noupdate /tb_axi_lite_gps/gps_clk_fast
add wave -noupdate /tb_axi_lite_gps/gps_clk_slow
add wave -noupdate /tb_axi_lite_gps/rst
add wave -noupdate /tb_axi_lite_gps/done
add wave -noupdate /tb_axi_lite_gps/pass
add wave -noupdate /tb_axi_lite_gps/axi_lite_drv
add wave -noupdate -divider {END: TB}
add wave -noupdate -divider {BEGIN: GPS TOP AXI4LITE}
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/clk_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/rst_ni
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps_clk_fast
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps_clk_slow
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wb_clk
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wb_rst
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wb_rst_n
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_adr_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_dat_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_sel_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_we_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_cyc_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_stb_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_cti_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_bte_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_dat_o
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_ack_o
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/wbs_d_gps_err_o
add wave -noupdate -divider {END: GPS TOP AXI4LITE}
add wave -noupdate -divider {BEGIN: GPS TOP}
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_adr_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_cyc_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_dat_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_sel_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_stb_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_we_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_ack_o
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_err_o
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_dat_o
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_clk_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/wb_rst_i
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/gps_clk_fast
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/gps_clk_slow
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/genNext
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/ca_code
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/p_code
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/l_code
add wave -noupdate /tb_axi_lite_gps/gps_top_axi4lite_inst/gps/codes_valid
add wave -noupdate -divider {END: GPS TOP}
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {0 ns} 0}
quietly wave cursor active 0
configure wave -namecolwidth 236
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
WaveRestoreZoom {0 ns} {1233 ns}
