MODULE = bplib

# Copied CMake config from wamr package
# TODO swap this out when transforming to pkg
PKG_BUILD_DIR = $(CURDIR)/build

#strip_slash = $(patsubst %/,%,$(1))

# We dont want the -DRIOT_BORAD=... definitions in the flags, however CPU dependent things should be there
RIOTBUILD_H = $(shell echo "$(CFLAGS)" | grep -o '\-include [^ ]*')
#BPLIB_CFLAGS += -g -O3 # TODO move
BPLIB_CFLAGS += -Wno-format
BPLIB_CFLAGS += $(CFLAGS_CPU) $(CFLAGS_LINK) $(CFLAGS_DBG) $(CFLAGS_OPT) $(RIOTBUILD_H)

# Remove the prefixes
RIOT_INCLUDES := $(filter-out -%,$(subst -isystem,,$(subst -I,,$(INCLUDES))))
# Remove trailing slashes (probably not needed anymore)
#RIOT_INCLUDES := $(foreach d,$(RIOT_INCLUDES),$(call strip_slash,$(d)))
TINYCBOR_INCLUDE_DIRS := $(foreach d,$(RIOT_INCLUDES),$(if $(findstring tinycbor,$(d)),$(d),))

#-include /home/simon/Code/dtn/bplib/app/test/bin/native64/riotbuild/riotbuild.h


BPLIB_CMAKE_FLAGS +="-DRIOT_INCLUDES=$(RIOT_INCLUDES)"\
                     -DCMAKE_SYSTEM_NAME=Generic \
                     -DCMAKE_SYSTEM_PROCESSOR="$(MCPU)" \
                     -DCMAKE_C_COMPILER=$(CC) \
                     -DCMAKE_C_COMPILER_WORKS=1 \
					 -DBPLIB_OS_LAYER=RIOT\
					 -DCMAKE_C_FLAGS="$(BPLIB_CFLAGS)" \
                     -DTINYCBOR_INCLUDE_DIRS="$(TINYCBOR_INCLUDE_DIRS)" \
					 -DBPLIB_ENABLE_UNIT_TESTS=OFF \
					 -DBPLIB_BUILD_TEST_TOOLS=OFF \
					 #-DBPLIB_INCLUDE_BPV7=ON
                     #


all: $(BINDIR)/libbplib.a

$(BINDIR)/libbplib.a: $(PKG_BUILD_DIR)/libbplib.a
	cp $< $@

$(PKG_BUILD_DIR)/libbplib.a: $(PKG_BUILD_DIR)/Makefile
	+$(MAKE) -C $(PKG_BUILD_DIR) $(CMAKEMAKEFLAGS)

$(PKG_BUILD_DIR)/Makefile: $(PKG_PREPARED) print_build_target
	cmake -B$(PKG_BUILD_DIR) $(BPLIB_CMAKE_FLAGS)

print_build_target:
	@echo PKG_VERSION: $(PKG_VERSION)
	@echo native OS_ARCH: $(OS_ARCH)
	@echo CPU_ARCH: $(CPU_ARCH)
	@echo CPU: $(CPU)
	@echo CFLAGS: $(BPLIB_CFLAGS)
	@echo RIOT_INCLUDES: $(RIOT_INCLUDES)

include $(RIOTBASE)/Makefile.base
