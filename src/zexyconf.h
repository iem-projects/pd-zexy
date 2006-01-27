/* this is a zexyconf.h for windows 
 * on unix-derivatives (linux, os-X,...) this file should be overwritten by configure
 * via the template zexyconf.h.in
 *
 * if you cannot use configure to re-generate this file, make sure all the defines 
 * are set correctly
 */


/* Define if you have the <regex.h> header file.  */
#undef HAVE_REGEX_H

/* Define if you have the <alloca.h> header file.  */
#undef HAVE_ALLOCA_H

/* define if you want parallelport-support (direct access to the port address) */
#define Z_WANT_LPT 1

/* define if you have the <linux/ppdev.h> header file.
 * (for parport _device_ support) 
 * you need Z_WANT_LPT for this to have an effect ! 
 */
#undef HAVE_LINUX_PPDEV_H


