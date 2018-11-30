onerror {resume}
quietly virtual signal -install /tb_axi_lite_des3/des3_top_axi4lite_inst/des3 { /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_adr_i[5:2]} wb_adr_ia
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_axi_lite_des3/clk
add wave -noupdate /tb_axi_lite_des3/rst
add wave -noupdate /tb_axi_lite_des3/done
add wave -noupdate /tb_axi_lite_des3/axi_lite_drv
add wave -noupdate -divider {BEGIN: DES3_TOP_AXILITE}
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/clk_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/rst_ni
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wb_clk
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wb_rst
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wb_rst_n
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_adr_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_dat_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_sel_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_we_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_cyc_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_stb_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_cti_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_bte_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_dat_o
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_ack_o
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/wbs_d_des3_err_o
add wave -noupdate -divider {END: DES3_TOP_AXILITE}
add wave -noupdate -divider {BEGIN: DES3}
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_adr_ia
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_adr_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_cyc_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_dat_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_sel_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_stb_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_we_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_ack_o
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_err_o
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_dat_o
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/int_o
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_clk_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_rst_i
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/start
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/start_r
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/decrypt
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/decrypt_r
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/wb_stb_i_r
add wave -noupdate -expand /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/key
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/des_in
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/key_1
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/key_2
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/key_3
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/ct
add wave -noupdate /tb_axi_lite_des3/des3_top_axi4lite_inst/des3/ct_valid
add wave -noupdate -divider {END: DES3}
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {0 ns} 0}
quietly wave cursor active 0
configure wave -namecolwidth 150
configure wave -valuecolwidth 216
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
WaveRestoreZoom {0 ns} {368 ns}
