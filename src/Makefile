default: all

.PHONEY: default all everything dist \
	clean realclean distclean \
	install install-bin install-doc install-abs \
	tests


TESTDIR=../tests

HELPERSOURCES=z_zexy.c zexy.c winNT_portio.c

OBJECTSOURCES=$(sort $(filter-out $(HELPERSOURCES), $(filter %.c, $(wildcard *.c))))

SOURCES=$(OBJECTSOURCES) $(HELPERSOURCES)

zexyconf.h: zexyconf.h.in configure
	./configure

configure: configure.ac
	autoconf

-include $(SOURCES:.c=.d)

Make.config: Make.config.in configure
	./configure

z_zexy.c z_zexy.h:
	./makesource.sh

-include Make.config


##  2nd only generate depend-files when we have Make.config included
##  and thus MAKEDEP_FLAGS defined
ifdef MAKEDEP_FLAGS
## dependencies: as proposed by the GNU-make documentation
## see http://www.gnu.org/software/make/manual/html_node/make_47.html#SEC51
%.d: %.c
	@set -e; rm -f $@; \
	 $(CPP) $(MAKEDEP_FLAGS) $(Z_CFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$
endif

.SUFFIXES: .$(EXT)

TARGETS = $(SOURCES:.c=.o)

OBJECTS = $(OBJECTSOURCES:.c=)

## if $(BUILDLIBRARY) is defined, we build everything as a single library
## else we build separate externals
ifneq "$(BUILDLIBRARY)" ""
all: $(LIBNAME)
	cp $(LIBNAME).$(EXT) ..
else
all: $(OBJECTS)
endif

$(OBJECTS): % : %.o
	$(LD) $(LFLAGS) -o $@.$(EXT) $@.o $(LIBS)
	$(STRIP) $(STRIPFLAGS) $@.$(EXT)

$(LIBNAME): $(TARGETS) z_zexy.c z_zexy.h
	$(LD) $(LFLAGS) -o $@.$(EXT) *.o $(LIBS)
	$(STRIP) $(STRIPFLAGS) $@.$(EXT)

$(TARGETS): %.o : %.c
	$(CC) $(Z_CFLAGS) -c -o $@ $*.c

externals: $(OBJECTS)

clean:
	-rm -f *.$(EXT) *.o 

realclean: clean
	-rm -f *~ _* config.*
	-rm -f *.d *.d.*

distclean: realclean
	-rm -f Make.config zexyconf.h ../*.$(EXT)
	-rm -f zexy.exp  zexy.lib  zexy.ncb  \
		zexy.opt  zexy.plg
	-rm -rf autom4te.cache/

tests:  all
	make -C $(TESTDIR)

install: install-bin install-doc install-abs

install-bin:
	-install -d $(INSTALL_BIN)
	-install -m 644 $(LIBNAME).$(EXT) $(INSTALL_BIN)

install-doc:
	-install -d $(INSTALL_DOC)
	-install -m 644 ../examples/*.pd $(INSTALL_DOC)

install-abs:
	-install -d $(INSTALL_BIN)
	-install -m 644 ../abs/*.pd $(INSTALL_BIN)

dist: all realclean
	(cd ../..;tar czvf $(TARNAME) $(LIBNAME))

everything: clean all install distclean

