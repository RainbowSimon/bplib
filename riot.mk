BPLIB_FOLDERS := aa/arp aa/as aa/fwp aa/nc \
				 bpa/bi bpa/ct bpa/ebp bpa/pdb bpa/pi bpa/stor \
				 ci/cbor ci/crc ci/eid ci/em ci/mem ci/pl ci/qm ci/rbt ci/time \
				 cla version

BPLIB_PRIVATE_INCLUDE_DIRS := $(foreach dir,$(BPLIB_FOLDERS),$(dir)/inc) inc
BPLIB_PRIVATE_INCLUDES := $(foreach dir,$(BPLIB_PRIVATE_INCLUDE_DIRS),-I$(dir))

# Note: We dont compile the bpa/stor
BPLIB_SOURCES_INIT := $(foreach dir,$(BPLIB_FOLDERS),$(wildcard $(dir)/src/*.c))
BPLIB_IGNORED_SOURCES := bpa/stor/src/% ci/mem/src/bplib_std_allocator.c
BPLIB_SOURCES := $(filter-out $(BPLIB_IGNORED_SOURCES),$(BPLIB_SOURCES_INIT))

BPLIB_OBJECTS := $(patsubst %.c,$(BUILDDIR)/%.o,$(BPLIB_SOURCES))
BPLIB_OBJECT_DIRS := $(foreach dir,$(BPLIB_FOLDERS),$(BUILDDIR)/$(dir)/src)

BPLIB_CFLAGS += $(CFLAGS)
BPLIB_CFLAGS += -Wno-format
BPLIB_CFLAGS += -Wno-int-to-pointer-cast
BPLIB_CFLAGS += -Wno-old-style-definition
BPLIB_CFLAGS += -Wno-unused-parameter
BPLIB_CFLAGS += -Wno-cast-function-type
BPLIB_CFLAGS += -Wno-sign-compare
# LLVM specifics that GCC does not complain about
BPLIB_CFLAGS += -Wno-strict-prototypes
BPLIB_CFLAGS += -Wno-format-nonliteral
BPLIB_CFLAGS += -Wno-unused-but-set-variable

.PHONY: create_dirs

$(BUILDDIR)/libbplib.a: create_dirs $(BPLIB_OBJECTS)
	@$(AR) rcs $@ $(BPLIB_OBJECTS)

create_dirs:
	@mkdir -p $(BPLIB_OBJECT_DIRS)

$(BUILDDIR)/%.o: %.c
	@$(CC) $(BPLIB_CFLAGS) $(BPLIB_PRIVATE_INCLUDES) $(INCLUDES) -c $< -o $@