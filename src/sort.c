/*
 *  sort :  sort a list of floats
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

/* ------------------------- sort ------------------------------- */

/*
  SHELL SORT: simple and easy
*/

static t_class *sort_class = NULL;

typedef struct _sort {
  t_object x_obj;

  int bufsize;
  t_float *buffer;
  t_int *indices;

  int ascending;

  t_outlet *indexOut, *sortedOut;
} t_sort;

static void sort_dir(t_sort *x, t_float f)
{
  x->ascending = (f < 0.f) ? 0 : 1;
}

static void sort_buffer(t_sort *x, int argc, t_atom *argv)
{
  int n = argc;
  t_float *buf;
  t_atom *atombuf = argv;

  if (argc != x->bufsize) {
    if (x->buffer) {
      freebytes(x->buffer, x->bufsize * sizeof(t_float));
    }
    if (x->indices) {
      freebytes(x->indices, x->bufsize * sizeof(t_int));
    }

    x->bufsize = argc;
    x->buffer = getbytes(x->bufsize * sizeof(t_float));
    x->indices = getbytes(x->bufsize * sizeof(t_int));
  }

  buf = x->buffer;
  while (n--) {
    *buf++ = atom_getfloat(atombuf++);
    x->indices[n] = n;
  }
}

static void sort_list(t_sort *x, t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  int step = argc, n;
  t_atom *atombuf = (t_atom *)getbytes(sizeof(t_atom) * argc);
  t_float *buf;
  t_int *idx;

  int i, loops = 1;

  sort_buffer(x, argc, argv);
  buf = x->buffer;
  idx = x->indices;

  while (step > 1) {
    step = (step % 2) ? (step + 1) / 2 : step / 2;

    i = loops;
    loops += 2;

    while (i--) { /* there might be some optimization in here */
      for (n = 0; n < (argc - step); n++) {
        if (buf[n] > buf[n + step]) {
          t_int i_tmp = idx[n];
          t_float f_tmp = buf[n];
          buf[n] = buf[n + step];
          buf[n + step] = f_tmp;
          idx[n] = idx[n + step];
          idx[n + step] = i_tmp;
        }
      }
    }
  }

  if (x->ascending)
    for (n = 0; n < argc; n++) {
      SETFLOAT(&atombuf[n], idx[n]);
    }
  else
    for (n = 0, i = argc - 1; n < argc; n++, i--) {
      SETFLOAT(&atombuf[n], idx[i]);
    }

  outlet_list(x->indexOut, gensym("list"), n, atombuf);

  if (x->ascending)
    for (n = 0; n < argc; n++) {
      SETFLOAT(&atombuf[n], buf[n]);
    }
  else
    for (n = 0, i = argc - 1; n < argc; n++, i--) {
      SETFLOAT(&atombuf[n], buf[i]);
    }
  outlet_list(x->sortedOut, gensym("list"), n, atombuf);

  freebytes(atombuf, argc * sizeof(t_atom));
}

static void *sort_new(t_floatarg f)
{
  t_sort *x = (t_sort *)pd_new(sort_class);
  x->ascending = (f < 0.f) ? 0 : 1;

  x->sortedOut = outlet_new(&x->x_obj, gensym("list"));
  x->indexOut = outlet_new(&x->x_obj, gensym("list"));

  x->bufsize = 0;
  x->buffer = NULL;

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("direction"));

  return (x);
}

static void sort_help(t_sort *UNUSED(x))
{
  post("\n" HEARTSYMBOL " sort\t\t:: sort a list of numbers");
}
ZEXY_SETUP void zexy_sort_setup(void)
{
  sort_class = zexy_new("sort", sort_new, 0, t_sort, CLASS_DEFAULT, "F");

  class_addlist(sort_class, sort_list);
  zexy_addmethod(sort_class, (t_method)sort_dir, "direction", "F");
  zexy_addmethod(sort_class, (t_method)sort_help, "help", "");

  zexy_register("sort");
}
#ifdef ZEXY_LIBRARY
/* only use sort_setup() when building as standalone objects
 * see https://git.iem.at/pd/zexy/issues/5
 */
void sort_setup(void)
{
  zexy_sort_setup();
}
#endif
