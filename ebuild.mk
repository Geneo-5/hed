################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of hed.
################################################################################

config-in       := Config.in
config-h        := hed/priv/config.h
config-obj      := config.o

HEADERDIR       := $(CURDIR)/include
headers         := hed/cdefs.h
headers         += hed/rpc.h
headers         += hed/server.h
headers         += hed/repo.h
headers         += $(call kconf_enabled,HED_TROER_BASE,hed/base.h)
headers         += $(call kconf_enabled,HED_TROER_BASE,hed/base.yml)
headers         += $(call kconf_enabled,HED_TROER_INET,hed/inet.h)
headers         += $(call kconf_enabled,HED_TROER_INET,hed/inet.yml)

subdirs         := lib

define libhed_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libhed
Description: Hed library
Version: $(VERSION)
Requires: libstroll libgalv libdpack $(call kconf_enabled,HED_TROER,json-c)
Requires.private: libstroll libgalv libdpack $(call kconf_enabled,HED_TROER,json-c)
Cflags: -I$${includedir}
Libs: -L$${libdir} -Wl,--push-state,--as-needed -lhed -Wl,--pop-state
endef

pkgconfigs      := libhed.pc
libhed.pc-tmpl  := libhed_pkgconf_tmpl

################################################################################
# Source code tags generation
################################################################################

tagfiles := $(shell find $(addprefix $(CURDIR)/,$(subdirs)) \
                         $(HEADERDIR) \
                         -type f)

# ex: filetype=make :
