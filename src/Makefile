default: all

Make.config: Make.config.in configure
	./configure


configure: configure.ac
	autoconf

include Make.config

.SUFFIXES: .$(EXT)

SOURCES=$(sort $(filter %.c, $(wildcard *.c)))

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

cleaner: clean
	-rm -f *~ _* config.*
	-rm -f *.d *.d.*

cleanest: cleaner
	-rm -f Makefile ../*.$(EXT)

distclean: cleanest newmakefile

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

dist: all cleaner
	(cd ../..;tar czvf $(TARNAME) $(LIBNAME))

everything: clean all install distclean

newmakefile:
	echo "current:">Makefile
	echo "	./configure && make">>Makefile

## dependencies: as proposed by the GNU-make documentation
## see http://www.gnu.org/software/make/manual/html_node/make_47.html#SEC51
-include $(SOURCES:.c=.d)
%.d: %.c
	@set -e; rm -f $@; \
	 $(CC) $(MAKEDEP_FLAGS) $(Z_CFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$
