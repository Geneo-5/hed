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

subdirs         := lib

define libhed_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libhed
Description: Hed library
Version: $(VERSION)
Requires: libutils libstroll libgalv libdpack
Requires.private: libutils libstroll libgalv libdpack
Cflags: -I$${includedir}
Libs: -L$${libdir} -Wl,--push-state,--as-needed -lhed -Wl,--pop-state
endef

pkgconfigs      := libgalv.pc
libgalv.pc-tmpl := libgalv_pkgconf_tmpl

################################################################################
# Source code tags generation
################################################################################

tagfiles := $(shell find $(addprefix $(CURDIR)/,$(subdirs)) \
                         $(HEADERDIR) \
                         -type f)

# ex: filetype=make :
