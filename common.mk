################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of hed.
################################################################################

common-cflags         := -Wall \
                         -Wextra \
                         -Wformat=2 \
                         -Wconversion \
                         -Wundef \
                         -Wshadow \
                         -Wcast-qual \
                         -Wcast-align \
                         -Wmissing-declarations \
                         -fvisibility=hidden \
                         -D_GNU_SOURCE \
                         -iquote $(TOPDIR)/include \
                         -I $(TOPDIR)/include \
                         $(EXTRA_CFLAGS)

common-ldflags        := $(common-cflags) \
                         $(EXTRA_LDFLAGS) \
                         -Wl,-z,start-stop-visibility=hidden

ifeq ($(CONFIG_HED_UTEST)$(CONFIG_HED_ASSERT_API),yy)
# When unit testsuite is required to be built, make sure to enable ELF semantic
# interposition.
# This allows unit test programs to override the stroll_assert_fail() using
# their own definitions based on CUTe's expectations to validate assertions.
#
# See http://maskray.me/blog/2021-05-09-fno-semantic-interposition for more
# informations about semantic interposition.
common-cflags         := $(common-cflags) -fsemantic-interposition
endif # ($(CONFIG_HED_UTEST)$(CONFIG_HED_ASSERT_API),yy)

ifneq ($(filter y,$(CONFIG_HED_ASSERT_API) $(CONFIG_HED_ASSERT_INTERN)),)
common-cflags         := $(filter-out -DNDEBUG,$(common-cflags))
endif # ($(filter y,$(CONFIG_HED_ASSERT_API) $(CONFIG_HED_ASSERT_INTERN)),)

shared-common-cflags  := $(filter-out -fpie -fPIE,$(common-cflags)) -fpic

shared-common-ldflags := $(filter-out -pie -fpie -fPIE,$(common-ldflags)) \
                         -shared -Bsymbolic -fpic

ifneq ($(filter y,$(CONFIG_HED_ASSERT_API) $(CONFIG_HED_ASSERT_INTERN)),)
common-ldflags        := $(filter-out -DNDEBUG,$(common-ldflags))
endif # ($(filter y,$(CONFIG_HED_ASSERT_API) $(CONFIG_HED_ASSERT_INTERN)),)

common-pkgconf        := libutils libstroll libglav libdpack

# ex: filetype=make :
