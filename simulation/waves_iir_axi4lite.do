onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {BEGIN: TB}
add wave -noupdate /tb_axi4lite_iir/clk
add wave -noupdate /tb_axi4lite_iir/rst
add wave -noupdate /tb_axi4lite_iir/done
add wave -noupdate /tb_axi4lite_iir/pass
add wave -noupdate -divider {END: TB}
add wave -noupdate -divider {BEGIN: IIR TOP AXI4LITE}
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/clk_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/rst_ni
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/o_axi_awready
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_awaddr
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_awcache
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_awprot
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_awvalid
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/o_axi_wready
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_wdata
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_wstrb
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_wvalid
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/o_axi_bresp
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/o_axi_bvalid
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_bready
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/o_axi_arready
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_araddr
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_arcache
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_arprot
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_arvalid
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/o_axi_rresp
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/o_axi_rvalid
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/o_axi_rdata
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/i_axi_rready
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wb_rst
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wbs_adr_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wbs_dat_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wbs_sel_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wbs_we_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wbs_cyc_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wbs_stb_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wbs_dat_o
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wbs_err_o
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/wbs_ack_o
add wave -noupdate -divider {END: IIR TOP AXI4LITE}
add wave -noupdate -divider {BEGIN: IIR TOP}
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_adr_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_cyc_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_dat_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_sel_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_stb_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_we_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_ack_o
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_err_o
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_dat_o
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/int_o
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_clk_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/wb_rst_i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/next
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/xSel
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/ySel
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/count
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/dataIn
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/dataOut
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/data_Out
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/data_In_data
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/data_In_addr
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/data_Out_addr
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/data_valid
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/data_In_write
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/next_out
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/data_In_write_r
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/data_In_write_posedge
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/i
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/next_r
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/next_posedge
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/next_out_r
add wave -noupdate /tb_axi4lite_iir/iir_top_axi4lite_inst/iir_top_wb_inst/next_out_posedge
add wave -noupdate -divider {END: IIR TOP}
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {0 ns} 0}
quietly wave cursor active 0
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
WaveRestoreZoom {0 ns} {6395 ns}
