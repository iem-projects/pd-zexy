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


/* 3108:forum::für::umläute:2000 */

/* ------------------------- packel ------------------------------- */

/*
  get the nth element of a package
*/

#include "zexy.h"
#include <string.h>
// do we need memory.h ?
#include <memory.h>


static t_class *packel_class;

typedef struct _packel
{
  t_object x_obj;
  t_float position;
} t_packel;

static void packel_list(t_packel *x, t_symbol *s, int argc, t_atom *argv)
{
  int mypos = x->position;

  if (mypos) {
    t_atom *current;
    int pos = (mypos < 0)?(argc+mypos):(mypos-1);

    if(argc==0){
      if (pos==0||pos==-1)outlet_bang(x->x_obj.ob_outlet);
      return;
    }
    
    if (pos < 0 || pos >= argc)return;

    current = &(argv[pos]);

    switch (current->a_type) {
    case A_FLOAT:
      outlet_float(x->x_obj.ob_outlet, atom_getfloat(current));
      break;
    case A_SYMBOL:
      outlet_symbol(x->x_obj.ob_outlet, atom_getsymbol(current));
      break;
    case A_POINTER:
      outlet_pointer(x->x_obj.ob_outlet, current->a_w.w_gpointer);
      break;
    case A_NULL:
      outlet_bang(x->x_obj.ob_outlet);
    default:
	  ;
    }
  } else outlet_list(x->x_obj.ob_outlet, s, argc, argv);
}

static void packel_anything(t_packel *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *av2 = (t_atom *)getbytes((argc + 1) * sizeof(t_atom));
  int i;

  for (i = 0; i < argc; i++)
    av2[i + 1] = argv[i];
  SETSYMBOL(av2, s);
  packel_list(x, gensym("list"), argc+1, av2);
  freebytes(av2, (argc + 1) * sizeof(t_atom));
}

static void *packel_new(t_floatarg f)
{
  t_packel *x = (t_packel *)pd_new(packel_class);
  outlet_new(&x->x_obj, 0);
  floatinlet_new(&x->x_obj, &x->position);
  x->position = (int) f;

  return (x);
}

void packel_setup(void)
{
  packel_class = class_new(gensym("packel"), (t_newmethod)packel_new, 
			   0, sizeof(t_packel), 0, A_DEFFLOAT, 0);

  class_addlist  (packel_class, packel_list);
  class_addanything(packel_class, packel_anything);

  class_sethelpsymbol(packel_class, gensym("zexy/packel"));
  zexy_register("packel");
}
