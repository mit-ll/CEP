#************************************************************************
# Copyright (C) 2019 Massachusetts Institute of Technology
#
# File Name:      cepregression.mk
# Program:        Common Evaluation Platform (CEP)
# Description:    Buildroot mk file for CEP Regression Suite
#************************************************************************

CEPREGRESSION_VERSION = 2.0.0
CEPREGRESSION_SITE = ./package/cepregression/src
CEPREGRESSION_SITE_METHOD = local

define CEPREGRESSION_BUILD_CMDS
  $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define CEPREGRESSION_INSTALL_TARGET_CMDS
  $(INSTALL) -D -m 0755 $(@D)/cepregression $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))