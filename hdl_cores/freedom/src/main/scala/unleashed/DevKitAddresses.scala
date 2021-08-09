//--------------------------------------------------------------------------------------
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File         : DevKitAddresses.scala
// Project      : Common Evaluation Platform (CEP)
// Description  : Defines addresses used within the CEP cores that are used Institute
//                Chisel and/or passed down into the blackbox verilog through
//                parameters
//--------------------------------------------------------------------------------------
package sifive.freedom.unleashed

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
  val cep_cores_depth               = 0x000FFFFFL
  
  val cepregs_base_addr             = 0x700F0000L
  val cepregs_base_depth            = 0x0000FFFFL

  val srot_base_addr                = 0x70200000L
  val srot_base_depth               = 0x0000ffffL
}