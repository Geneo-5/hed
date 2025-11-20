################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of hed.
################################################################################

include ../common.mk

libhed-objects  := codec.o

solibs          := libhed.so
libhed-objs     := $(addprefix shared/,$(libhed-objects))
libhed-cflags   := $(shared-common-cflags)
libhed-ldflags  := $(shared-common-ldflags)
libhed-pkgconf  := $(common-pkgconf)

arlibs          := libhed.a
libhed.a-objs   := $(addprefix static/,$(libhed-objects))
libhed.a-cflags := $(common-cflags)

# ex: filetype=make :
