//--------------------------------------------------------------------------------------
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : cep_addresses.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : Defines the addresses used within CEP Cores
//                Constants related to "Functional" register decode, which occurrs in 
//                the Chisel world, can be found in this package.
//                For each CEP core, there are the following two pairs of constants:
//                  - <core>_base_addr          - Functional registers base address
//                  - <core>_base_depth         - Functional registers address depth/size
//                  - <core>_llki_base_addr     - LLKI interface base address
//                  - <core>_llki_base_depth    - LLKI interface address depth/size
//
//                Additional address constants related to the functional registers can
//                be found in this file, within the relevant object (e.g., AESAddresses)
//
//                LLKI related address constants can be found in llki_pkg.sv as all LLKI
//                functionality exists in the SystemVerilog world.
//
//--------------------------------------------------------------------------------------
package mitllBlocks.cep_addresses

import chisel3._
import chisel3.util._
import chisel3.experimental.{IntParam, BaseModule}
import freechips.rocketchip.config.Field
import freechips.rocketchip.subsystem.{BaseSubsystem, PeripheryBusKey}
import freechips.rocketchip.config.Parameters
import freechips.rocketchip.diplomacy._
import freechips.rocketchip.regmapper._
import freechips.rocketchip.tilelink._

object CEPVersion {
  val CEP_MAJOR_VERSION             = 0x03
  val CEP_MINOR_VERSION             = 0x30
}

object CEPBaseAddresses {
  val scratchpad_base_addr          = 0x64800000L
  val scratchpad_depth              = 0x0000FFFFL

  val cep_cores_base_addr           = 0x70000000L
    val aes_base_addr               = 0x70000000L
    val aes_depth                   = 0x000000FFL                                             
    val aes_llki_base_addr          = 0x70008000L
      val aes_llki_ctrlsts_addr     = 0x70008000L
      val aes_llki_sendrecv_addr    = 0x70008008L
    val aes_llki_depth              = 0x0000003fL
    
    val md5_base_addr               = 0x70010000L
    val md5_depth                   = 0x000000FFL                                             
    val md5_llki_base_addr          = 0x70018000L
      val md5_llki_ctrlsts_addr     = 0x70018000L
      val md5_llki_sendrecv_addr    = 0x70018008L
    val md5_llki_depth              = 0x000000ffL

    val sha256_base_addr            = 0x70020000L
    val sha256_depth                = 0x000000FFL                                             
    val sha256_llki_base_addr       = 0x70028000L
      val sha256_llki_ctrlsts_addr  = 0x70028000L
      val sha256_llki_sendrecv_addr = 0x70028008L
    val sha256_llki_depth           = 0x000000ffL

    val rsa_base_addr               = 0x70030000L
    val rsa_depth                   = 0x000000FFL                                             
    val rsa_llki_base_addr          = 0x70038000L
      val rsa_llki_ctrlsts_addr     = 0x70038000L
      val rsa_llki_sendrecv_addr    = 0x70038008L
    val rsa_llki_depth              = 0x000000ffL

    val des3_base_addr              = 0x70040000L
    val des3_depth                  = 0x000000FFL                                             
    val des3_llki_base_addr         = 0x70048000L
      val des3_llki_ctrlsts_addr    = 0x70048000L
      val des3_llki_sendrecv_addr   = 0x70048008L
    val des3_llki_depth             = 0x000000ffL

    val dft_base_addr               = 0x70050000L
    val dft_depth                   = 0x000000FFL                                             
    val dft_llki_base_addr          = 0x70058000L
      val dft_llki_ctrlsts_addr     = 0x70058000L
      val dft_llki_sendrecv_addr    = 0x70058008L
    val dft_llki_depth              = 0x000000ffL

    val idft_base_addr              = 0x70060000L
    val idft_depth                  = 0x000000FFL                                             
    val idft_llki_base_addr         = 0x70068000L
      val idft_llki_ctrlsts_addr    = 0x70068000L
      val idft_llki_sendrecv_addr   = 0x70068008L
    val idft_llki_depth             = 0x000000ffL

    val fir_base_addr               = 0x70070000L  
    val fir_depth                   = 0x000000FFL                                             
    val fir_llki_base_addr          = 0x70078000L
      val fir_llki_ctrlsts_addr     = 0x70078000L
      val fir_llki_sendrecv_addr    = 0x70078008L
    val fir_llki_depth              = 0x000000ffL

    val iir_base_addr               = 0x70080000L 
    val iir_depth                   = 0x000000FFL                                             
    val iir_llki_base_addr          = 0x70088000L
      val iir_llki_ctrlsts_addr     = 0x70088000L
      val iir_llki_sendrecv_addr    = 0x70088008L
    val iir_llki_depth              = 0x000000ffL

    val gps_base_addr               = 0x70090000L  
    val gps_depth                   = 0x000000FFL                                             
    val gps_llki_base_addr          = 0x70098000L
      val gps_llki_ctrlsts_addr     = 0x70098000L
      val gps_llki_sendrecv_addr    = 0x70098008L
    val gps_llki_depth              = 0x000000ffL
  val cep_cores_depth               = 0x0000FFFFL
  
  val cepregs_base_addr             = 0x700F0000L
  val cepregs_base_depth            = 0x0000FFFFL

  val srot_base_addr                = 0x70200000L
  val srot_base_depth               = 0x0000FFFFL
}

object AESAddresses {
  val aes_ctrlstatus_addr           = 0x0000
  val aes_pt0_addr                  = 0x0008
  val aes_pt1_addr                  = 0x0010
  val aes_ct0_addr                  = 0x0018
  val aes_ct1_addr                  = 0x0020
  val aes_key0_addr                 = 0x0028
  val aes_key1_addr                 = 0x0030
  val aes_key2_addr                 = 0x0038
}

object DES3Addresses {
    val des3_ctrlstatus_addr        = 0x0000
    val des3_decrypt_addr           = 0x0008
    val des3_desIn_addr             = 0x0010  
    val des3_key1_addr              = 0x0018
    val des3_key2_addr              = 0x0020
    val des3_key3_addr              = 0x0028
    val des3_done                   = 0x0030
    val des3_desOut_addr            = 0x0038
}

object MD5Addresses {
    val md5_ready                   = 0x0000
    val md5_msg_padded_w0           = 0x0008
    val md5_msg_padded_w1           = 0x0010
    val md5_msg_padded_w2           = 0x0018
    val md5_msg_padded_w3           = 0x0020
    val md5_msg_padded_w4           = 0x0028
    val md5_msg_padded_w5           = 0x0030
    val md5_msg_padded_w6           = 0x0038
    val md5_msg_padded_w7           = 0x0040 
    val md5_msg_output_w0           = 0x0048 
    val md5_msg_output_w1           = 0x0050
    val md5_rst                     = 0x0058
    val md5_in_valid                = 0x0060
    val md5_out_valid               = 0x0068
}

object SHA256Addresses{
    val sha256_ctrlstatus_addr      = 0x0000
    val sha256_block_w0             = 0x0008
    val sha256_block_w1             = 0x0010
    val sha256_block_w2             = 0x0018
    val sha256_block_w3             = 0x0020
    val sha256_block_w4             = 0x0028
    val sha256_block_w5             = 0x0030
    val sha256_block_w6             = 0x0038
    val sha256_block_w7             = 0x0040
    val sha256_done                 = 0x0048
    val sha256_digest_w0            = 0x0050
    val sha256_digest_w1            = 0x0058
    val sha256_digest_w2            = 0x0060
    val sha256_digest_w3            = 0x0068
}

object RSAAddresses{
    val rsa_ctrlstatus_addr         = 0x0000
    val rsa_exp_ptr_rst_addr        = 0x0008
    val rsa_exp_data_addr           = 0x0010
    val rsa_exp_ctrl_addr           = 0x0018
    val rsa_mod_ptr_rst_addr        = 0x0020
    val rsa_mod_data                = 0x0028
    val rsa_mod_ctrl_addr           = 0x0030
    val rsa_message_ptr_rst         = 0x0038
    val rsa_message_data            = 0x0040
    val rsa_message_ctrl_addr       = 0x0048
    val rsa_mod_length              = 0x0050
    val rsa_exp_length              = 0x0058
    val rsa_result_ptr_rst          = 0x0060
    val rsa_result_data_addr        = 0x0068
    val rsa_result_ctrl_addr        = 0x0070
    val rsa_cycles_addr             = 0x0078
}

object GPSAddresses{
    val gps_ctrlstatus_addr         = 0x0000
    val gps_ca_code_addr            = 0x0008
    val gps_p_code_addr_w0          = 0x0010
    val gps_p_code_addr_w1          = 0x0018
    val gps_l_code_addr_w0          = 0x0020
    val gps_l_code_addr_w1          = 0x0028    
    val gps_sv_num_addr             = 0x0030
    val gps_reset_addr              = 0x0038
    val gps_aes_key_addr_w0         = 0x0040
    val gps_aes_key_addr_w1         = 0x0048
    val gps_aes_key_addr_w2         = 0x0050
    val gps_pcode_speed_addr        = 0x0058
    val gps_pcode_xini_addr         = 0x0060
}

object DFTAddresses{
    val dft_ctrlstatus_addr         = 0x0000
    val dft_datain_addr_addr        = 0x0008
    val dft_datain_data_addr        = 0x0010
    val dft_dataout_addr_addr       = 0x0018
    val dft_dataout_data_addr       = 0x0020

}

object IDFTAddresses{
    val idft_ctrlstatus_addr        = 0x0000
    val idft_datain_addr_addr       = 0x0008
    val idft_datain_data_addr       = 0x0010
    val idft_dataout_addr_addr      = 0x0018
    val idft_dataout_data_addr      = 0x0020
}

object IIRAddresses{
    val iir_ctrlstatus_addr         = 0x0000
    val iir_datain_addr_addr        = 0x0008
    val iir_datain_data_addr        = 0x0010
    val iir_dataout_addr_addr       = 0x0018
    val iir_dataout_data_addr       = 0x0020
    val iir_reset_addr              = 0x0028
    val iir_shift_addr              = 0x0030
    val iir_shift_addr2             = 0x0038
}

object FIRAddresses {
  val fir_ctrlstatus_addr           = 0x0000
  val fir_datain_addr_addr          = 0x0008
  val fir_datain_data_addr          = 0x0010
  val fir_dataout_addr_addr         = 0x0018
  val fir_dataout_data_addr         = 0x0020
  val fir_reset_addr                = 0x0028    
}

object CEPRegisterAddresses {
    val version_register            = 0x0000
    val testNset                    = 0xFD10    
    //
    val scratch_w0                  = 0xFE00
    val scratch_w1                  = 0xFE08
    val scratch_w2                  = 0xFE10
    val scratch_w3                  = 0xFE18
    val scratch_w4                  = 0xFE20
    val scratch_w5                  = 0xFE28
    val scratch_w6                  = 0xFE30
    val scratch_w7                  = 0xFE38
    //
    val core0_status                = 0xFF00
    val core1_status                = 0xFF08
    val core2_status                = 0xFF10
    val core3_status                = 0xFF18
}

// These are intended to be the universal TL parameters
// for all the LLKI-enabled cores (including the SRoT).
// They need to match those values called out in top_pkg.sv
// Exceptions (when needed) are explictly coded into
// the specific TLModuleImp
object LLKITilelinkParameters {
    val BeatBytes                   = 8     // Should be = TL_DW / 8
    val AddressBits                 = 32    // Should be = TL_DW
    val SourceBits                  = 4     // Should be = TL_AIW
    val SinkBits                    = 2     // Should be = TL_DIW
    val SizeBits                    = 3     // Should be = TL_SZW
}

// The following class is used to pass paramaters down into the CEP cores
case class COREParams(
  slave_base_addr     : BigInt,
  slave_depth         : BigInt,
  llki_base_addr      : BigInt,
  llki_depth          : BigInt,
  llki_ctrlsts_addr   : BigInt,
  llki_sendrecv_addr  : BigInt,
  dev_name            : String
)

// The following parameter pass attachment info to the lower level objects/classes/etc.
case class COREAttachParams(
  coreparams          : COREParams,
  llki_bus            : TLBusWrapper,
  slave_bus           : TLBusWrapper
)

// The following class is used to pass paramaters down into the CEP Registers
case class CEPREGSParams(
  slave_base_addr     : BigInt,
  slave_depth         : BigInt,
  dev_name            : String
)

// The following parameter pass attachment info to the lower level objects/classes/etc.
case class CEPREGSAttachParams(
  cepregsparams       : CEPREGSParams,
  slave_bus           : TLBusWrapper
)

