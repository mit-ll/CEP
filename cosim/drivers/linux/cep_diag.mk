#************************************************************************
# Copyright 2021 Massachusetts Institute of Technology
#
# File Name:      zcep_diag.mk
# Program:        Common Evaluation Platform (CEP)
# Description:    Buildroot mk file for CEP Regression Suite
#************************************************************************

CEP_DIAG_VERSION = 3.3
CEP_DIAG_SITE = ./package/cep_diag/src
CEP_DIAG_SITE_METHOD = local

CEP_DIAG_CONF_OPTS += 
CEP_DIAG_DEPENDENCIES += cryptopp

define CEP_DIAG_BUILD_CMDS
  $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define CEP_DIAG_INSTALL_TARGET_CMDS
  $(INSTALL) -D -m 0755 $(@D)/cep_diag $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
