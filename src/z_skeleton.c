
/* 1008:forum::für::umläute:2001 */

/*
  skeleton :  skeleton-code for message-objects
*/

#include "zexy.h"

/* ------------------------- skeleton ------------------------------- */

/*
MESSAGE SKELETON: simple and easy
*/

static t_class *skeleton_class;

typedef struct _skeleton
{
  t_object x_obj;

} t_skeleton;


static void skeleton_float(t_skeleton *x, t_float f)
{

}

static void skeleton_list(t_skeleton *x, t_symbol *s, int argc, t_atom *argv)
{

}

static void skeleton_foo(t_skeleton *x, t_float f)
{

}

static void *skeleton_new(t_floatarg f)
{
  t_skeleton *x = (t_skeleton *)pd_new(skeleton_class);

  return (x);
}

void z_skeleton_setup(void)
{
  skeleton_class = class_new(gensym("skeleton"), (t_newmethod)skeleton_new, 
			      0, sizeof(t_skeleton), 0, A_DEFFLOAT,  0);
  
  class_addlist  (skeleton_class, skeleton_list);
  class_addfloat (skeleton_class, skeleton_float);
  class_addmethod(skeleton_class, (t_method)skeleton_foo, gensym("foo"), A_DEFFLOAT, 0);

  class_sethelpsymbol(skeleton_class, gensym("zexy/skeleton"));
}
