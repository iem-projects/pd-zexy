==============================================================================
the zexy external
==============================================================================




outline of this file::
==============================================================================
 +  general
 +  installation
   +  linux
   +  w32
   +  irix
   +  osX
 +  using
 +  authors



general::
==============================================================================
the zexy external is a collection of externals for miller.s.puckette's 
realtime-computermusic-environment called "puredata" (or abbreviated "pd")
this zexy external will be of no use, if you don't have a running version of 
pd on your system.
check out for http://pd.iem.at to learn more about pd and how to get it 

note: the zexy external is published under the Gnu General Public License 
that is included (GnuGPL.txt). some parts of the code are taken directly 
from the pd source-code, they, of course, fall under the license pd is 
published under.



installation::
==============================================================================

linux :
------------------------------------------------------------------------------

#1>  cd src/
#2>  autoconf
#3>  ./configure
#4>  make
#5>  make install

this will install the zexy external into /usr/local/lib/pd/externs
(the path can be changed either via the "--prefix"-flag to "configure"
or by editing the makefile
alternatively you can try "make everything" (after ./configure)
note: if you don't want the parallel-port object [lpt]
 (e.g.: because you don't have a parallel-port) you can disable it 
 with "--disable-lpt"


macOS-X:
------------------------------------------------------------------------------
see installation/linux
the configure-script should work here too;

win32 :
------------------------------------------------------------------------------

#1 extract the zexy-0_x.zip to your pd-path (this file should be located 
   at <mypdpath>/pd/zexy/)
#2 execute the "z_install.bat", this should copy all necessary files 
   to the correct places

to compile: 
 + w/ MSVC use makefile.nt or zexy.dsw; 
 OR
 + with GCC configure your pd path, eg:
	#> ./configure --prefix=/c/program/pd; make; make install
 OR
 + cross-compilation for windows on linux using mingw (assumes that the 
   crosscompiler is "i586-mingw32msvc-cc")
	#> ./configure --host=i586-mingw32msvc --with-extension=dll \
	   --with-pd=/path/to/win/pd/ --disable-PIC

irix :
------------------------------------------------------------------------------

though i have physical access to both SGI's O2s and indys,
i haven't tried to compile the zexy externals there for years.
the configure-script should work here too;
if not, try "make -f makefile.irix"
Good luck !



making pd run with the zexy external::
==============================================================================
make sure, that pd will be looking at this location 
(add "-path <mypath>/pd/externs" either to your .pdrc or each time 
you execute pd)
make sure, that you somehow load the zexy external (either add "-lib zexy" 
(if you advised pd somehow to look at the correct place) 
or "-lib <myzexypath>/zexy" to your startup-script (.pdrc or whatever) 
or load it via the object "zexy" at runtime



authors::
==============================================================================
this software is 
copyleft 1999-2005 by iohannes m zmoelnig <zmoelnig@iem.kug.ac.at>
with some contributions by winfried ritsch, guenter geiger, miller.s.puckette 
and surely some others


