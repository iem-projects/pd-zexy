/* ********************************************** */
/* the ZEXY external                              */
/* ********************************************** */
/*                            forum::für::umläute */
/* ********************************************** */

/* the ZEXY external is a runtime-library for miller s. puckette's realtime-computermusic-software "pure data"
 * therefore you NEED "pure data" to make any use of the ZEXY external
 * (except if you want to use the code for other things)
 * download "pure data" at

 http://pd.iem.at
 ftp://iem.at/pd

 *
 * if you are looking for the latest release of the ZEXY-external you should have another look at

 ftp://iem.at/pd/Externals/ZEXY

 * 
 * ZEXY is published under the GNU GeneralPublicLicense, that must be shipped with ZEXY.
 * if you are using Debian GNU/linux, the GNU-GPL can be found under /usr/share/common-licenses/GPL
 * if you still haven't found a copy of the GNU-GPL, have a look at http://www.gnu.org
 *
 * "pure data" has it's own license, that comes shipped with "pure data".
 *
 * there are ABSOLUTELY NO WARRANTIES for anything
 */

#ifndef INCLUDE_ZEXY_H__
#define INCLUDE_ZEXY_H__

#ifdef __WIN32__
# ifndef NT
#  define NT
# endif
# ifndef MSW
#  define MSW
# endif
#endif

#ifdef ZEXY_LIBRARY
/* some defines, which turn on special compile-options 
 * (like parallel-port)
 * the ifdef is here, to not break the externals/build-system
 * (read: build-systems outside of zexy)
 */
# include "zexyconf.h"
#endif

#include "m_pd.h"

#ifdef __SSE__
# include <xmmintrin.h>
# define Z_SIMD_BLOCK 16  /* must be a power of 2 */
# define Z_SIMD_BYTEALIGN (128/8)   /* assume 128 bits */
# define Z_SIMD_CHKBLOCKSIZE(n) (!(n&(Z_SIMD_BLOCK-1)))
# define Z_SIMD_CHKALIGN(ptr) ( ((unsigned long)(ptr) & (Z_SIMD_BYTEALIGN-1)) == 0 )

#endif /* __SSE__ */

#include <math.h>

#define VERSION "2.1"

/* these pragmas are only used for MSVC, not MinGW or Cygwin */
#ifdef _MSC_VER
# pragma warning( disable : 4018 )
# pragma warning( disable : 4244 )
# pragma warning( disable : 4305 )
#endif

#ifdef __WIN32__
# define HEARTSYMBOL 3
# define STATIC_INLINE
# define sqrtf sqrt
# define fabsf fabs
#else
# define HEARTSYMBOL 169
# define STATIC_INLINE static
#endif

#ifdef __APPLE__
# include <AvailabilityMacros.h>
# if defined (MAC_OS_X_VERSION_10_3) && MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_3
#  define sqrtf sqrt
# endif /* OSX-10.3 */
#endif /* APPLE */

typedef struct _mypdlist
{
  t_object x_obj;

  int x_n;
  t_atom *x_list;
} t_mypdlist;

#ifndef ZEXY_LIBRARY
static void zexy_register(char*object){
  if(object!=0){
    post("[%s]", object);
    post("\tpart of zexy-%s", VERSION);
    post("\tCopyright (l) IOhannes m zmölnig, 1999-2005");
    post("\tforum::für::umläute");
    post("\tIEM");
    post("\tcompiled:  "__DATE__" ");
  }
}
#else
static void zexy_register(char*object){}
#endif /* ZEXY_LIBRARY */

#if (defined PD_MAJOR_VERSION && defined PD_MINOR_VERSION) && (PD_MAJOR_VERSION > 0 || PD_MINOR_VERSION > 38)
/* 
 * pd>=0.39 has a verbose() function; older versions don't
 * btw, this finally makes zexy binary incompatible with older version
 */
# define z_verbose verbose
#else
/* 
 * this might not work on compilers other than gcc
 * is it ISO-C99 or just a gnu-cpp thing ?
 # define z_verbose(level, format, ...) post(format, ## __VA_ARGS__)
 *
 * so we make it simpler: on older version we just shut up!
 */
# define z_verbose
#endif


#endif /* INCLUDE_ZEXY_H__ */
