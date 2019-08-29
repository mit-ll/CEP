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
	val CEP_MINOR_VERSION		= 0x00

}

object CEPBaseAddresses {

	val aes_base_addr           = 0x70000000L
	val fir_base_addr           = 0x70070000L
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