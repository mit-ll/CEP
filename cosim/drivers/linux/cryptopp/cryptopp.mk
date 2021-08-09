################################################################################
#
# cryptopp
#
################################################################################

CRYPTOPP_VERSION = 5.6.3
CRYPTOPP_SOURCE = cryptopp$(subst .,,$(CRYPTOPP_VERSION)).zip
CRYPTOPP_SITE = http://cryptopp.com
CRYPTOPP_INSTALL_STAGING = YES
CRYPTOPP_INSTALL_TARGET = NO
CRYPTOPP_INSTALL_HOST = NO
CRYPTOPP_DEPENDENCIES = 

# Set the compiler flags for where to look for header files (.h), shared static libraries (.a), shared dynamic libraries (.so).
# Use the "--sysroot" flag instead of -I$(STAGING_DIR)/usr/include -L$(STAGING_DIR)/usr/lib

define CRYPTOPP_EXTRACT_CMDS
	unzip $(DL_DIR)/$(CRYPTOPP_SOURCE) -d $(@D)
endef

define CRYPTOPP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) static --file=GNUmakefile-cross -C $(@D) \
	$(TARGET_CONFIGURE_OPTS) \
	CXXFLAGS+=--sysroot=$(STAGING_DIR) \
	LDFLAGS+=--sysroot=$(STAGING_DIR) \
	CXXFLAGS+='-D__always_inline__=__unused__'
# The above line is needed to suppress errors about inability to inline certain functions
endef

define CRYPTOPP_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libcryptopp.a $(STAGING_DIR)/usr/lib/libcryptopp.a
# Don't forget the headers; cep_diag will need them
	mkdir -p $(STAGING_DIR)/usr/include/cryptopp
	$(INSTALL) -D -m 0644 $(@D)/*.h $(STAGING_DIR)/usr/include/cryptopp
endef

$(eval $(generic-package))
