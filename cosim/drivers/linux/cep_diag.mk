#************************************************************************
# Copyright (C) 2019 Massachusetts Institute of Technology
#
# File Name:      cep_diag.mk
# Program:        Common Evaluation Platform (CEP)
# Description:    Buildroot mk file for CEP Regression Suite
#************************************************************************

CEP_DIAG_VERSION = 2.0.0
CEP_DIAG_SITE = ./package/cep_diag/src
CEP_DIAG_SITE_METHOD = local

define CEP_DIAG_BUILD_CMDS
  $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define CEP_DIAG_INSTALL_TARGET_CMDS
  $(INSTALL) -D -m 0755 $(@D)/cep_diag $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
