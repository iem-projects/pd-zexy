/*
 *  sum :: the sum of a list of floats
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

static t_class *sum_class = NULL;

typedef struct _sum {
  t_object x_obj;
} t_sum;

static void sum_list(t_sum *x, t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  t_float sum = 0.f;

  while (argc--) {
    sum += atom_getfloat(argv++);
  }

  outlet_float(x->x_obj.ob_outlet, sum);
}

static void *sum_new(void)
{
  t_sum *x = (t_sum *)pd_new(sum_class);

  outlet_new(&x->x_obj, gensym("float"));

  return (x);
}

static void sum_help(void)
{
  post("sum\t:: calculate the sum of a list of floats");
}

ZEXY_SETUP void sum_setup(void)
{
  sum_class = zexy_new("sum", sum_new, 0, t_sum, CLASS_DEFAULT, "");

  class_addlist(sum_class, (t_method)sum_list);
  zexy_addmethod(sum_class, (t_method)sum_help, "help", "");

  zexy_register("sum");
}
