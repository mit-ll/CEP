onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {BEGIN: TB}
add wave -noupdate /tb_axi_lite_sha256/clk
add wave -noupdate /tb_axi_lite_sha256/rst
add wave -noupdate /tb_axi_lite_sha256/done
add wave -noupdate /tb_axi_lite_sha256/pass
add wave -noupdate /tb_axi_lite_sha256/axi_lite_drv
add wave -noupdate -divider {END: TB}
add wave -noupdate -divider {BEGIN: SHA256 AXI4Lite}
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/clk_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/rst_ni
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wb_clk
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wb_rst
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wb_rst_n
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_adr_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_dat_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_sel_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_we_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_cyc_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_stb_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_cti_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_bte_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_dat_o
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_ack_o
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/wbs_d_sha_err_o
add wave -noupdate -divider {END: SHA256 AXI4Lite}
add wave -noupdate -divider {BEGIN: SHA256 Top}
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_adr_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_cyc_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_dat_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_sel_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_stb_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_we_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_ack_o
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_err_o
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_dat_o
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/int_o
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_clk_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/wb_rst_i
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/startHash
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/newMessage
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/bigData
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/hash
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/ready
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/hashValid
add wave -noupdate -divider {END: SHA256 Top}
add wave -noupdate -divider {BEGIN: SHA256}
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/clk
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/rst
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/init
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/next
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/block
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/ready
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/digest
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/digest_valid
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/a_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/a_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/b_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/b_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/c_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/c_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/d_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/d_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/e_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/e_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/f_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/f_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/g_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/g_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/h_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/h_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/a_h_we
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H0_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H0_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H1_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H1_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H2_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H2_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H3_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H3_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H4_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H4_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H5_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H5_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H6_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H6_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H7_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H7_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/H_we
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/t_ctr_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/t_ctr_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/t_ctr_we
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/t_ctr_inc
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/t_ctr_rst
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/digest_valid_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/digest_valid_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/digest_valid_we
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/sha256_ctrl_reg
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/sha256_ctrl_new
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/sha256_ctrl_we
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/digest_init
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/digest_update
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/state_init
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/state_update
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/first_block
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/ready_flag
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/t1
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/t2
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/k_data
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/w_init
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/w_next
add wave -noupdate /tb_axi_lite_sha256/sha256_top_axi4lite_inst/sha256_top_inst/sha256/w_data
add wave -noupdate -divider {END: SHA256}
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
WaveRestoreZoom {0 ns} {1149 ns}
