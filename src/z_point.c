/* 
   (c) 1202:forum::für::umläute:2000
   
   "time" gets the current time from the system
   "date" gets the current date from the system
   
*/

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include "zexy.h"

typedef struct _inner
{
  int n;
  t_float f;
} t_inner;

/* ----------------------- test --------------------- */

static t_class *test_class;

typedef struct _test
{
  t_object x_obj;
  
  t_inner *val;

} t_test;

static void *test_new(t_symbol *s, int argc, t_atom *argv)
{
  t_test *x = (t_test *)pd_new(test_class);

  x->val = (t_inner *)getbytes(sizeof(t_inner));

  outlet_new(&x->x_obj, 0); 
  
  return (x);
}

static void test_point(t_test *x, t_gpointer *gp )
{
  t_inner *a=(t_inner *)gp;
  //  post("got pointer to %x or %x", gp, a);

  post("reading @ %x & %x............... f=%f n=%d", &(a->f), &(a->n), a->f, a->n);

  outlet_float(x->x_obj.ob_outlet, a->f);
  outlet_float(x->x_obj.ob_outlet, (t_float)a->n);

}

static void test_float(t_test *x,t_floatarg f )
{
  t_gpointer *gp;
  t_inner *a;

  x->val->f = f;
  x->val->n = f;

  gp=(t_gpointer *)(x->val);
  
  //  post("set pointer to %x or %x", x->val, gp);
 
  outlet_pointer(x->x_obj.ob_outlet, gp);

  //  post("setted at %x", gp);

  a = (t_inner *)gp;
  // post("yes %f at %x", a->f, a);
 
}
static void test_bang(t_test *x)
{
  post("bang");

  outlet_pointer(x->x_obj.ob_outlet, (t_gpointer *)x->val);
}

static void help_test(t_test *x)
{
}

void test_setup(void)
{
  test_class = class_new(gensym("test"),
			 (t_newmethod)test_new, 0,
			 sizeof(t_test), 0, A_GIMME, 0);
  
  class_addbang(test_class, test_bang);
  class_addpointer(test_class, test_point);
  class_addfloat(test_class, test_float);

  class_addmethod(test_class, (t_method)help_test, gensym("help"), 0);
  class_sethelpsymbol(test_class, gensym("zexy/test"));

}


/*	general setup */


void z_point_setup(void)
{
  test_setup();
}
