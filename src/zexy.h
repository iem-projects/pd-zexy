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

#include "m_pd.h"

#define VERSION "2.0"

#ifdef NT
# pragma warning( disable : 4244 )
# pragma warning( disable : 4305 )
# define HEARTSYMBOL 3
# define sqrtf sqrt
# define fabsf fabs
# define STATIC_INLINE
#else
# define HEARTSYMBOL 169
#endif

#ifdef MACOSX
# define sqrtf sqrt
#endif


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


#endif /* INCLUDE_ZEXY_H__ */
