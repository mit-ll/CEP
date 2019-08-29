onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {BEGIN: Testbench}
add wave -noupdate /tb_axi4lite_aes/clk
add wave -noupdate /tb_axi4lite_aes/rst
add wave -noupdate /tb_axi4lite_aes/done
add wave -noupdate /tb_axi4lite_aes/pass
add wave -noupdate -expand /tb_axi4lite_aes/axi_lite_drv
add wave -noupdate -divider {END: Testbench}
add wave -noupdate -divider {BEGIN: DUT}
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/clk_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/rst_ni
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/o_axi_awready
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_awaddr
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_awcache
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_awprot
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_awvalid
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/o_axi_wready
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_wdata
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_wstrb
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_wvalid
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/o_axi_bresp
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/o_axi_bvalid
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_bready
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/o_axi_arready
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_araddr
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_arcache
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_arprot
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_arvalid
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/o_axi_rresp
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/o_axi_rvalid
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/o_axi_rdata
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/i_axi_rready
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/wb_rst
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/wbs_d_aes_adr_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/wbs_d_aes_dat_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/wbs_d_aes_sel_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/wbs_d_aes_we_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/wbs_d_aes_cyc_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/wbs_d_aes_stb_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/wbs_d_aes_dat_o
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/wbs_d_aes_ack_o
add wave -noupdate -divider {END: DUT}
add wave -noupdate -divider {BEGIN: AES}
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/DW
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/AW
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_adr_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_cyc_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_dat_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_sel_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_stb_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_we_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_ack_o
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_err_o
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_dat_o
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/int_o
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_clk_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/wb_rst_i
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/start
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/start_r
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/pt
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/key
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/pt_big
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/key_big
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/ct
add wave -noupdate /tb_axi4lite_aes/aes_top_axi4lite_inst/aes_top_wb_inst/ct_valid
add wave -noupdate -divider {END: AES}
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {16171 ns} 0}
quietly wave cursor active 1
configure wave -namecolwidth 254
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
WaveRestoreZoom {1175 ns} {1265 ns}
