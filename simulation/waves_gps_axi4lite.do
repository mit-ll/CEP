onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {BEGIN: TB}
add wave -noupdate /tb_axi4lite_gps/clk
add wave -noupdate /tb_axi4lite_gps/rst
add wave -noupdate /tb_axi4lite_gps/done
add wave -noupdate /tb_axi4lite_gps/pass
add wave -noupdate /tb_axi4lite_gps/gps_clk_fast
add wave -noupdate /tb_axi4lite_gps/gps_clk_slow
add wave -noupdate -divider {END: TB}
add wave -noupdate -divider {BEGIN: GPS TOP AXI4LITE}
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/clk_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/rst_ni
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_clk_fast
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_clk_slow
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/o_axi_awready
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_awaddr
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_awcache
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_awprot
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_awvalid
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/o_axi_wready
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_wdata
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_wstrb
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_wvalid
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/o_axi_bresp
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/o_axi_bvalid
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_bready
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/o_axi_arready
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_araddr
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_arcache
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_arprot
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_arvalid
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/o_axi_rresp
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/o_axi_rvalid
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/o_axi_rdata
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/i_axi_rready
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wb_rst
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wbs_adr_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wbs_dat_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wbs_sel_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wbs_we_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wbs_cyc_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wbs_stb_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wbs_dat_o
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wbs_err_o
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/wbs_ack_o
add wave -noupdate -divider {END: GPS TOP AXI4LITE}
add wave -noupdate -divider {BEGIN: GPS TOP}
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_adr_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_cyc_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_dat_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_sel_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_stb_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_we_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_ack_o
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_err_o
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_dat_o
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_clk_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/wb_rst_i
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/gps_clk_fast
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/gps_clk_slow
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/genNext
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/ca_code
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/p_code
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/l_code
add wave -noupdate /tb_axi4lite_gps/gps_top_axi4lite_inst/gps_top_wb_inst/codes_valid
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
