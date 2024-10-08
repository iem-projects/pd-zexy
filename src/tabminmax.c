/*
 * tabminmax: get minimum and maximum of a table
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

/* =================== tabminmax ====================== */

static t_class *tabminmax_class = NULL;

typedef struct _tabminmax {
  t_object x_obj;
  t_outlet *min_out, *max_out;
  t_symbol *x_arrayname;
  t_int startindex, stopindex;
} t_tabminmax;

static void tabminmax_bang(t_tabminmax *x)
{
  t_garray *A;
  int npoints;
  t_word *vec;

  if (!(A = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class))) {
    pd_error(x, "%s: no such array", x->x_arrayname->s_name);
  } else if (!garray_getfloatwords(A, &npoints, &vec)) {
    pd_error(x, "%s: bad template for tabminmax", x->x_arrayname->s_name);
  } else {
    int n;
    t_atom atombuf[2];
    t_float min, max;
    int mindex, maxdex;

    int start = x->startindex;
    int stop = x->stopindex;
    if (start < 0 || start > stop) {
      start = 0;
    }
    if (stop < start || stop > npoints) {
      stop = npoints;
    }
    npoints = stop - start;

    min = vec[start].w_float;
    max = min;

    mindex = start;
    maxdex = start;

    for (n = 1; n < npoints; n++) {
      t_float val = vec[start + n].w_float;
      if (val < min) {
        mindex = start + n;
        min = val;
      }
      if (val > max) {
        maxdex = start + n;
        max = val;
      }
    }

    SETFLOAT(atombuf, max);
    SETFLOAT(atombuf + 1, maxdex);
    outlet_list(x->max_out, gensym("list"), 2, atombuf);

    SETFLOAT(atombuf, min);
    SETFLOAT(atombuf + 1, mindex);
    outlet_list(x->min_out, gensym("list"), 2, atombuf);
  }
}

static void tabminmax_list(
    t_tabminmax *x, t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  int a, b;
  switch (argc) {
  case 2:
    a = atom_getint(argv);
    b = atom_getint(argv + 1);
    x->startindex = (a < b) ? a : b;
    x->stopindex = (a > b) ? a : b;
    tabminmax_bang(x);
    break;
  default:
    pd_error(x, "tabminmax: list must be 2 floats (is %d atoms)", argc);
  }
}

static void tabminmax_set(t_tabminmax *x, t_symbol *s)
{
  x->x_arrayname = s;
}

static void *tabminmax_new(t_symbol *s)
{
  t_tabminmax *x = (t_tabminmax *)pd_new(tabminmax_class);
  x->x_arrayname = s;
  x->startindex = 0;
  x->stopindex = -1;
  x->min_out = outlet_new(&x->x_obj, gensym("list"));
  x->max_out = outlet_new(&x->x_obj, gensym("list"));

  return (x);
}

static void tabminmax_helper(void)
{
  post("\n" HEARTSYMBOL
       " tabminmax - object : dumps a table as a package of floats");
  post("'set <table>'\t: read out another table\n"
       "'bang'\t\t: get min and max of the table\n"
       "outlet\t\t: table-data as package of floats");
  post("creation\t: \"tabminmax <table>\"");
}

ZEXY_SETUP void tabminmax_setup(void)
{
  tabminmax_class =
      zexy_new("tabminmax", tabminmax_new, 0, t_tabminmax, CLASS_DEFAULT, "S");
  class_addbang(tabminmax_class, (t_method)tabminmax_bang);
  class_addlist(tabminmax_class, (t_method)tabminmax_list);

  zexy_addmethod(tabminmax_class, (t_method)tabminmax_set, "set", "s");

  zexy_addmethod(tabminmax_class, (t_method)tabminmax_helper, "help", "");
  zexy_register("tabminmax");
}
