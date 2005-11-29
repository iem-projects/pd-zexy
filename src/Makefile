default: all

.PHONEY: default all everything dist \
	clean realclean distclean \
	install install-bin install-doc install-abs

SOURCES=$(sort $(filter %.c, $(wildcard *.c)))

Make.config: Make.config.in configure
	./configure


configure: configure.ac
	autoconf

## uaehh, here comes some magic
##  1st we don't want depend and config-makefiles to be included on "clean"-targets

ifeq (,$(findstring clean, $(MAKECMDGOALS)))
-include $(SOURCES:.c=.d)
-include Make.config
endif

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


all: $(LIBNAME)
	cp $(LIBNAME).$(EXT) ..

$(LIBNAME): $(TARGETS)
	$(LD) $(LFLAGS) -o $(LIBNAME).$(EXT) *.o $(LIBS)
	$(STRIP) $(STRIPFLAGS) $(LIBNAME).$(EXT)

$(TARGETS): %.o : %.c
	$(CC) -c -o $@ $(Z_CFLAGS) $*.c


clean:
	-rm -f *.$(EXT) *.o 

realclean: clean
	-rm -f *~ _* config.*
	-rm -f *.d *.d.*

distclean: realclean
	-rm -f Make.config ../*.$(EXT)

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

