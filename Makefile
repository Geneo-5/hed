################################################################################
# SPDX-License-Identifier: LGPL-3.0-only
#
# This file is part of Hed.
################################################################################

override PACKAGE := hed
override VERSION := 1.0
EXTRA_CFLAGS     := -O2 -DNDEBUG -Wall -Wextra -Wformat=2
EXTRA_LDFLAGS    := -O2
TROER            := troer
EXTRA_TROER_ARGS :=

export VERSION EXTRA_CFLAGS EXTRA_LDFLAGS

ifeq ($(strip $(EBUILDDIR)),)
ifneq ($(realpath ebuild/main.mk),)
EBUILDDIR := $(realpath ebuild)
else  # ($(realpath ebuild/main.mk),)
EBUILDDIR := $(realpath /usr/share/ebuild)
endif # !($(realpath ebuild/main.mk),)
endif # ($(strip $(EBUILDDIR)),)

ifeq ($(realpath $(EBUILDDIR)/main.mk),)
$(error '$(EBUILDDIR)': no valid eBuild install found !)
endif # ($(realpath $(EBUILDDIR)/main.mk),)

include $(EBUILDDIR)/main.mk

yml := $(patsubst include/hed/%.yml,%,$(wildcard include/hed/*.yml))

$(CURDIR)/troer/%:
	@mkdir -p $(@)

define troer_recipe
@echo "  TROER   $(strip $(1))"
$(Q)$(TROER) --no-indent --json --makefile no $(3) $(strip $(1)) $(strip $(2))
endef

.PHONY: troer
troer: | $(patsubst %,$(CURDIR)/troer/%,$(yml))
	$(foreach y,$(yml),\
	          $(call troer_recipe,$(CURDIR)/include/hed/$(y).yml,\
	                              $(CURDIR)/lib/,\
	                              --include-dir $(CURDIR)/include/hed/ \
	                              --include-prefix hed/ \
	                              $(EXTRA_TROER_ARGS))$(newline))

# ex: filetype=make :
