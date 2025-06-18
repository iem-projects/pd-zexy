/*
 * envvar: access environment variables from within Pd
 *
 * (c) 2025 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
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

/* ------------------------- envvar ------------------------------- */
#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

static t_class *envvar_class = NULL;

typedef struct _envvar {
  t_object x_obj;
  t_outlet*x_outlet;
  t_outlet*x_errout;
} t_envvar;

static void envvar_symbol(t_envvar*x, t_symbol*s) {
  char*e = getenv(s->s_name);
  if(e)
    outlet_symbol(x->x_outlet, gensym(e));
  else if (x->x_errout) {
    outlet_bang(x->x_errout);
  } else {
    pd_error(x, "no such environment variable: %s", s->s_name);
  }

}

static void *envvar_new()
{
  t_envvar *x = (t_envvar *)pd_new(envvar_class);
  x->x_outlet = outlet_new(&x->x_obj, gensym("symbol"));
  x->x_errout = outlet_new(&x->x_obj, gensym("bang"));
  return (x);
}

ZEXY_SETUP void envvar_setup(void)
{
  envvar_class =
    zexy_new("envvar", envvar_new, 0, t_envvar, CLASS_DEFAULT, "");
  class_addsymbol(envvar_class, envvar_symbol);
  zexy_register("envvar");
}
