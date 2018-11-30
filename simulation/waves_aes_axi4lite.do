onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {BEGIN: Testbench}
add wave -noupdate /tb_axi_lite_aes/clk
add wave -noupdate /tb_axi_lite_aes/rst
add wave -noupdate /tb_axi_lite_aes/done
add wave -noupdate -expand -subitemconfig {/tb_axi_lite_aes/axi_lite_drv.axi -expand} /tb_axi_lite_aes/axi_lite_drv
add wave -noupdate -divider {END: Testbench}
add wave -noupdate -divider {BEGIN: DUT}
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/clk_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/rst_ni
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wb_clk
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wb_rst
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_adr_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_dat_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_sel_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_we_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_cyc_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_stb_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_cti_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_bte_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_dat_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_ack_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/wbs_d_aes_err_o
add wave -noupdate -divider {END: DUT}
add wave -noupdate -divider {BEGIN: axil2wb}
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_ACLK
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_ARESETN
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_AWADDR
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_AWVALID
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_AWREADY
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_WDATA
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_WSTRB
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_WVALID
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_WREADY
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_ARADDR
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_ARVALID
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_ARREADY
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_RDATA
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_RRESP
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_RVALID
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_RREADY
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_BRESP
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_BVALID
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/S_AXI_BREADY
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_clk_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_rst_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_addr_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_dat_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_we_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_sel_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_stb_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_cyc_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_dat_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_err_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wb_ack_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/read_wait
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/read_wait_in
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/write_wait
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/write_wait_in
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/rdata
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/rdata_in
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/addr
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/addr_in
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wdata
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/wdata_in
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/bresp
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/rresp
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/bresp_in
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/rresp_in
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/del_bresp
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/del_rresp
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/del_bresp_in
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/axil2wb/del_rresp_in
add wave -noupdate -divider {END: axil2wb}
add wave -noupdate -divider {BEGIN: AES}
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_adr_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_cyc_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_dat_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_sel_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_stb_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_we_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_ack_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_err_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_dat_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/int_o
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_clk_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/wb_rst_i
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/start
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/pt_big
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/key_big
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/ct
add wave -noupdate /tb_axi_lite_aes/aes_top_axi4lite_inst/aes/ct_valid
add wave -noupdate -divider {END: AES}
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {5268 ns} 0}
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
WaveRestoreZoom {2518618 ns} {2530168 ns}
