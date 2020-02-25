//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : cep_addresses.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : Defines the addresses used within CEP Cores
//
package mitllBlocks.cep_addresses

object CEPVersion {
	val CEP_MAJOR_VERSION		= 0x02
	val CEP_MINOR_VERSION		= 0x02
}

object CEPBaseAddresses {
	val aes_base_addr           = 0x70000000L
    val md5_base_addr           = 0x70010000L
    val sha256_base_addr        = 0x70020000L
    val rsa_base_addr           = 0x70030000L
    val des3_base_addr          = 0x70040000L
    val dft_base_addr           = 0x70050000L
    val idft_base_addr          = 0x70060000L
    val fir_base_addr           = 0x70070000L  
	val iir_base_addr           = 0x70080000L 
    val gps_base_addr           = 0x70090000L  
    val cepregisters_base_addr  = 0x700F0000L
}

object AESAddresses {
	val aes_ctrlstatus_addr     = 0x0000
    val aes_pt0_addr            = 0x0008
    val aes_pt1_addr            = 0x0010
    val aes_ct0_addr            = 0x0018
    val aes_ct1_addr            = 0x0020
    val aes_key0_addr           = 0x0028
    val aes_key1_addr           = 0x0030
    val aes_key2_addr           = 0x0038
}

object DES3Addresses {
    val des3_ctrlstatus_addr     = 0x0000
    val des3_decrypt_addr        = 0x0008
    val des3_desIn_addr          = 0x0010  
    val des3_key1_addr           = 0x0018
    val des3_key2_addr           = 0x0020
    val des3_key3_addr           = 0x0028
    val des3_done                = 0x0030
    val des3_desOut_addr         = 0x0038

}

object MD5Addresses {
    val md5_ready               = 0x0000
    val md5_msg_padded_w0       = 0x0008
    val md5_msg_padded_w1       = 0x0010
    val md5_msg_padded_w2       = 0x0018
    val md5_msg_padded_w3       = 0x0020
    val md5_msg_padded_w4       = 0x0028
    val md5_msg_padded_w5       = 0x0030
    val md5_msg_padded_w6       = 0x0038
    val md5_msg_padded_w7       = 0x0040 
    val md5_msg_output_w0       = 0x0048 
    val md5_msg_output_w1       = 0x0050
    val md5_rst                 = 0x0058
    val md5_in_valid            = 0x0060
    val md5_out_valid           = 0x0068
}

object SHA256Addresses{
    val sha256_ctrlstatus_addr               = 0x0000
    val sha256_block_w0                      = 0x0008
    val sha256_block_w1                      = 0x0010
    val sha256_block_w2                      = 0x0018
    val sha256_block_w3                      = 0x0020
    val sha256_block_w4                      = 0x0028
    val sha256_block_w5                      = 0x0030
    val sha256_block_w6                      = 0x0038
    val sha256_block_w7                      = 0x0040
    val sha256_done                          = 0x0048
    val sha256_digest_w0                     = 0x0050
    val sha256_digest_w1                     = 0x0058
    val sha256_digest_w2                     = 0x0060
    val sha256_digest_w3                     = 0x0068
}

object RSAAddresses{
    val rsa_ctrlstatus_addr  = 0x0000
    val rsa_exp_ptr_rst_addr = 0x0008
    val rsa_exp_data_addr    = 0x0010
    val rsa_exp_ctrl_addr    = 0x0018
    val rsa_mod_ptr_rst_addr = 0x0020
    val rsa_mod_data         = 0x0028
    val rsa_mod_ctrl_addr    = 0x0030
    val rsa_message_ptr_rst  = 0x0038
    val rsa_message_data     = 0x0040
    val rsa_message_ctrl_addr= 0x0048
    val rsa_mod_length       = 0x0050
    val rsa_exp_length       = 0x0058
    val rsa_result_ptr_rst   = 0x0060
    val rsa_result_data_addr = 0x0068
    val rsa_result_ctrl_addr = 0x0070
    val rsa_cycles_addr      = 0x0078
}

object GPSAddresses{
    val gps_ctrlstatus_addr        = 0x0000
    val gps_ca_code_addr           = 0x0008
    val gps_p_code_addr_w0         = 0x0010
    val gps_p_code_addr_w1         = 0x0018
    val gps_l_code_addr_w0         = 0x0020
    val gps_l_code_addr_w1         = 0x0028    
    val gps_sv_num_addr            = 0x0030
    val gps_reset_addr             = 0x0038
}

object DFTAddresses{
    val dft_ctrlstatus_addr     = 0x0000
    val dft_datain_addr_addr    = 0x0008
    val dft_datain_data_addr    = 0x0010
    val dft_dataout_addr_addr   = 0x0018
    val dft_dataout_data_addr   = 0x0020

}

object IDFTAddresses{
    val idft_ctrlstatus_addr     = 0x0000
    val idft_datain_addr_addr    = 0x0008
    val idft_datain_data_addr    = 0x0010
    val idft_dataout_addr_addr   = 0x0018
    val idft_dataout_data_addr   = 0x0020
}

object IIRAddresses{
    val iir_ctrlstatus_addr     = 0x0000
    val iir_datain_addr_addr    = 0x0008
    val iir_datain_data_addr    = 0x0010
    val iir_dataout_addr_addr   = 0x0018
    val iir_dataout_data_addr   = 0x0020
    val iir_reset_addr          = 0x0028
    val iir_shift_addr          = 0x0030
    val iir_shift_addr2         = 0x0038
}

object FIRAddresses {
	val fir_ctrlstatus_addr		= 0x0000
	val fir_datain_addr_addr	= 0x0008
	val fir_datain_data_addr	= 0x0010
	val fir_dataout_addr_addr	= 0x0018
	val fir_dataout_data_addr	= 0x0020
}

object CEPRegisterAddresses {

	val version_register		= 0x0000

}