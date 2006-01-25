/******************************************************
 *
 * zexy - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   1999:forum::für::umläute:2004
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

/*
  operating_system :  operating_system-code for message-objects
*/

#include "zexy.h"

/* ------------------------- operating_system ------------------------------- */

/*
MESSAGE OPERATING_SYSTEM: simple and easy
*/

static t_class *operating_system_class;

typedef struct _operating_system
{
  t_object x_obj;

} t_operating_system;


static void operating_system_bang(t_operating_system *x)
{
  t_symbol *s=gensym("unknown");
#ifdef __linux__
  s=gensym("linux");
#elif defined __APPLE__
  s=gensym("macos");
#elif defined __WIN32__
  s=gensym("windows");
#endif
  outlet_symbol(x->x_obj.ob_outlet, s);
}

static void *operating_system_new(t_floatarg f)
{
  t_operating_system *x = (t_operating_system *)pd_new(operating_system_class);
  outlet_new(&x->x_obj, 0);
  return (x);
}

void operating_system_setup(void)
{
  operating_system_class = class_new(gensym("operating_system"), (t_newmethod)operating_system_new, 
			      0, sizeof(t_operating_system), 0, A_DEFFLOAT,  0);
  
  class_addbang  (operating_system_class, operating_system_bang);
  class_sethelpsymbol(operating_system_class, gensym("zexy/operating_system"));
  zexy_register("operating_system");
}
