################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of hed.
################################################################################

include ../common.mk

libhed-objects  := rpc.o server.o repo.o
libhed-objects  += $(call kconf_enabled,HED_TROER_BASE,base.o base-json.o)
libhed-objects  += $(call kconf_enabled,HED_TROER_INET,inet.o inet-json.o inet-impl.o)

solibs             := libhed.so
libhed.so-objs     := $(addprefix shared/,$(libhed-objects))
libhed.so-cflags   := $(shared-common-cflags)
libhed.so-ldflags  := $(shared-common-ldflags)
libhed.so-pkgconf  := $(common-pkgconf)

arlibs             := libhed.a
libhed.a-objs      := $(addprefix static/,$(libhed-objects))
libhed.a-cflags    := $(common-cflags)

# ex: filetype=make :
