/* ********************************************** */
/* the ZEXY external                              */
/* ********************************************** */
/*                            forum::für::umläute */
/* ********************************************** */

/* the ZEXY external is a runtime-library for miller s. puckette's realtime-computermusic-software "pure data"
 * therefore you NEED "pure data" to make any use of the ZEXY external
 * (except if you want to use the code for other things)
 * download "pure data" at

 http://iem.kug.ac.at/pd
 ftp://iem.kug.ac.at/pd

 *
 * if you are looking for the latest release of the ZEXY-external you should have another look at

 ftp://iem.kug.ac.at/pd/Externals/ZEXY

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

#define VERSION "1P2"

#ifdef NT
/* yes, we have beautiful hearts under NT */
#define HEARTSYMBOL 3
#else
/* but none for linux; indeed the only drawback */
#define HEARTSYMBOL 64
#endif

#endif
