/*
 * date: gets the current date from the system
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
   (c) 1202:forum::für::umläute:2000
   1506:forum::für::umläute:2003: use timeb only if needed (like on windoze)
*/

#include "zexy.h"

#if (defined __WIN32__)
#  if (defined __i386__) && (defined __MINGW32__)
/* unless compiling under mingw/32bit, we want USE_TIMEB in redmond-land */
#  else
#    define USE_TIMEB
#  endif
#endif

#ifdef __APPLE__
#  include <sys/types.h>
/* typedef     _BSD_TIME_T_    time_t;                */
#endif

#include <time.h>

#ifdef USE_TIMEB
#  include <sys/timeb.h>
#else
#  include <sys/time.h>
#endif

/* ----------------------- date --------------------- */

static t_class *date_class = NULL;

typedef struct _date {
  t_object x_obj;

  int GMT;

  t_outlet *x_outlet1;
  t_outlet *x_outlet2;
  t_outlet *x_outlet3;
  t_outlet *x_outlet4;
  t_outlet *x_outlet5;
  t_outlet *x_outlet6;
} t_date;

static void *date_new(t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  t_date *x = (t_date *)pd_new(date_class);

  x->GMT = 0;
  if (argc) {
    char buf[5];
    atom_string(argv, buf, 5);
    if (buf[0] == 'G' && buf[1] == 'M' && buf[2] == 'T') {
      x->GMT = 1;
    }
  }

  x->x_outlet1 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet2 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet3 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet4 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet5 = outlet_new(&x->x_obj, gensym("float"));
  x->x_outlet6 = outlet_new(&x->x_obj, gensym("float"));

  return (x);
}

static void date_bang(t_date *x)
{
  struct tm *resolvetime;
#ifdef USE_TIMEB
  struct timeb mytime;
  ftime(&mytime);
  resolvetime = (x->GMT) ? gmtime(&mytime.time) : localtime(&mytime.time);
#else
  struct timeval tv;
  gettimeofday(&tv, 0);
  resolvetime = (x->GMT) ? gmtime(&tv.tv_sec) : localtime(&tv.tv_sec);
#endif
  outlet_float(x->x_outlet6, (t_float)resolvetime->tm_isdst);
  outlet_float(x->x_outlet5, (t_float)resolvetime->tm_yday);
  outlet_float(x->x_outlet4, (t_float)resolvetime->tm_wday);
  outlet_float(x->x_outlet3, (t_float)resolvetime->tm_mday);
  outlet_float(x->x_outlet2, (t_float)resolvetime->tm_mon + 1);
  outlet_float(x->x_outlet1, (t_float)resolvetime->tm_year + 1900);
}

static void help_date(t_date *UNUSED(x))
{
  post("\n" HEARTSYMBOL " date\t\t:: get the current system date");
  post("\noutputs are\t: year / month / day / day of week /day of year / "
       "daylightsaving (1/0)");
  post("\ncreation\t::'date [GMT]': show local date or GMT");
}

ZEXY_SETUP void date_setup(void)
{
  date_class = zexy_new("date", date_new, 0, t_date, CLASS_DEFAULT, "*");

  class_addbang(date_class, date_bang);

  zexy_addmethod(date_class, (t_method)help_date, "help", "");
  zexy_register("date");
}
