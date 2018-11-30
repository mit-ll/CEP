onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {BEGIN: TB}
add wave -noupdate /tb_axi_lite_fir/clk
add wave -noupdate /tb_axi_lite_fir/rst
add wave -noupdate /tb_axi_lite_fir/done
add wave -noupdate /tb_axi_lite_fir/pass
add wave -noupdate -divider {END: TB}
add wave -noupdate -divider {BEGIN: FIR TOP AXI4LITE}
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/clk_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/rst_ni
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wb_clk
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wb_rst
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wb_rst_n
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_adr_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_dat_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_sel_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_we_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_cyc_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_stb_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_cti_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_bte_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_dat_o
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_ack_o
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/wbs_d_fir_err_o
add wave -noupdate -divider {END: FIR TOP AXI4LITE}
add wave -noupdate -divider {BEGIN: FIR TOP}
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_adr_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_cyc_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_dat_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_sel_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_stb_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_we_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_ack_o
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_err_o
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_dat_o
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/int_o
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_clk_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/wb_rst_i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/next
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/xSel
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/ySel
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/count
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/dataIn
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/dataOut
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/data_Out
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/data_In_data
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/data_In_addr
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/data_Out_addr
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/data_valid
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/data_In_write
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/next_out
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/data_In_write_r
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/data_In_write_posedge
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/i
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/next_r
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/next_posedge
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/next_out_r
add wave -noupdate /tb_axi_lite_fir/fir_top_axi4lite_inst/fir_top_inst/next_out_posedge
add wave -noupdate -divider {END: FIR TOP}
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
