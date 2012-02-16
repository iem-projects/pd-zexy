/* 
 * atof: ascii to A_FLOAT converter
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "zexy.h"
#include <stdlib.h>

static t_class *atof_class;

typedef struct _atof
{
  t_object x_obj;
  t_float f;
} t_atof;
static void atof_bang(t_atof *x)
{
  outlet_float(x->x_obj.ob_outlet, (t_float)x->f);
}
static void atof_float(t_atof *x, t_floatarg f)
{
  x->f = f;
  outlet_float(x->x_obj.ob_outlet, (t_float)x->f);
}
static void atof_symbol(t_atof *x, t_symbol *s)
{
  const char* c = s->s_name;
  x->f=strtod(c, 0);
  outlet_float(x->x_obj.ob_outlet, (t_float)x->f);
}
static void atof_list(t_atof *x, t_symbol *s, int argc, t_atom *argv)
{
  const char* c;
  ZEXY_USEVAR(s);

  if (argv->a_type==A_FLOAT){
    x->f=atom_getfloat(argv);
    outlet_float(x->x_obj.ob_outlet, (t_float)x->f);
    return;
  }

  c=atom_getsymbol(argv)->s_name;
  x->f=strtod(c, 0);
  outlet_float(x->x_obj.ob_outlet, (t_float)x->f);
}

static void *atof_new(void)
{
  t_atof *x = (t_atof *)pd_new(atof_class);
  x->f = 0.;
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

void atof_setup(void)
{
  atof_class = class_new(gensym("atof"), (t_newmethod)atof_new, 0,
			 sizeof(t_atof), 0, A_DEFFLOAT, 0);

  class_addbang(atof_class, (t_method)atof_bang);
  class_addfloat(atof_class, (t_method)atof_float);
  class_addlist(atof_class, (t_method)atof_list);
  class_addsymbol(atof_class, (t_method)atof_symbol);
  class_addanything(atof_class, (t_method)atof_symbol);

  zexy_register("atof");
}
