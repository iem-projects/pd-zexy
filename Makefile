# Makefile to build 'zexy' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

## TODO
# - regex
# - aliases

# library name
lib.name = zexy

# input source file (class name == source file basename)
class.sources = \
	src/0x260x260x7e.c \
	src/0x2e.c \
	src/0x3c0x7e.c \
	src/0x3d0x3d0x7e.c \
	src/0x3e0x7e.c \
	src/0x7c0x7c0x7e.c \
	src/a2l.c \
	src/abs~.c \
	src/absgn~.c \
	src/atof.c \
	src/atoi.c \
	src/avg~.c \
	src/blockmirror~.c \
	src/blockshuffle~.c \
	src/blockswap~.c \
	src/date.c \
	src/demultiplex~.c \
	src/demultiplex.c \
	src/dfreq~.c \
	src/dirac~.c \
	src/doublepole~.c \
	src/drip.c \
	src/envrms~.c \
	src/fifop.c \
	src/freadln.c \
	src/fwriteln.c \
	src/glue.c \
	src/index.c \
	src/length.c \
	src/lifop.c \
	src/limiter~.c \
	src/list2int.c \
	src/list2lists.c \
	src/list2symbol.c \
	src/lister.c \
	src/listfind.c \
	src/liststorage.c \
	src/longload.c \
	src/makesymbol.c \
	src/matchbox.c \
	src/mavg.c \
	src/minmax.c \
	src/msgfile.c \
	src/multiline~.c \
	src/multiplex~.c \
	src/multiplex.c \
	src/multireceive.c \
	src/niagara.c \
	src/noish~.c \
	src/noisi~.c \
	src/operating_system.c \
	src/pack~.c \
	src/pack.c \
	src/packel.c \
	src/pdf~.c \
	src/prime.c \
	src/quantize~.c \
	src/rawprint.c \
	src/regex.c \
	src/relay.c \
	src/repack.c \
	src/repeat.c \
	src/route~.c \
	src/sfplay.c \
	src/sfrecord.c \
	src/sgn~.c \
	src/sigzero~.c \
	src/sleepgrain.c \
	src/sort.c \
	src/step~.c \
	src/strcmp.c \
	src/sum.c \
	src/swap~.c \
	src/symbol2list.c \
	src/tabdump.c \
	src/tabminmax.c \
	src/tabread4~~.c \
	src/tabset.c \
	src/tavg~.c \
	src/time.c \
	src/unpack~.c \
	src/unpack.c \
	src/urn.c \
	src/wrap.c \
	src/z~.c \
	$(empty)

## TODO: install help-files for these aliases
l.class.sources = src/lister.c
lp.class.sources = src/lpt.c
demux~.class.sources = src/demultiplex~.c
demux.class.sources = src/demultiplex.c
mux~.class.sources = src/multiplex~.c
mux.class.sources = src/multiplex.c
l2s.class.sources = src/list2symbol.c
s2l.class.sources = src/symbol2list.c
unfold.class.sources = src/drip.c
# abs-aliases: any2list, l2i

lpt.class.sources = \
	src/lpt.c \
	src/winNT_portio.c \
	$(empty)

lib.setup.sources = \
	src/zexy.c \
	src/z_zexy.c \
	$(empty)

# all extra files to be included in binary distribution of the library
datafiles = \
	AUTHORS \
	README.txt \
	LICENSE.txt \
	ChangeLog \
	zexy-meta.pd

datafiles += \
	$(wildcard abs/*.pd) \
	$(wildcard reference/*.pd) \
	$(empty)

cflags = -DVERSION='"$(lib.version)"'

DATE_FMT = %Y/%m/%d at %H:%M:%S UTC
ifdef SOURCE_DATE_EPOCH
    BUILD_DATE ?= $(shell date -u -d "@$(SOURCE_DATE_EPOCH)" "+$(DATE_FMT)" 2>/dev/null || date -u -r "$(SOURCE_DATE_EPOCH)" "+$(DATE_FMT)" 2>/dev/null || date -u "+$(DATE_FMT)")
endif
ifdef BUILD_DATE
cflags += -DBUILD_DATE='"$(BUILD_DATE)"'
endif


make-lib-executable=yes
ifeq ($(make-lib-executable),yes)
 CPPFLAGS+=-DZEXY_LIBRARY
endif

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
