/* 
   (c) 1202:forum::für::umläute:2000
   
   "time" gets the current time from the system
   "date" gets the current date from the system
   
*/

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifdef MACOSX
#include <sys/types.h>
/* typedef     _BSD_TIME_T_    time_t;                */
#endif


#include "zexy.h"
#include <time.h>
#include <sys/timeb.h>

/* ----------------------- time --------------------- */

static t_class *time_class;

typedef struct _time
{
  t_object x_obj;
  
  int GMT;
  
  t_outlet *x_outlet1;
  t_outlet *x_outlet2;
  t_outlet *x_outlet3;
  t_outlet *x_outlet4;
} t_time;

static void *time_new(t_symbol *s, int argc, t_atom *argv)
{
  t_time *x = (t_time *)pd_new(time_class);
  char buf[5];
  
  x->GMT=0;
  if (argc) {
    atom_string(argv, buf, 5);
    if (buf[0]=='G' && buf[1]=='M' && buf[2]=='T')
      x->GMT = 1;
  }
  
  x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
  x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
  x->x_outlet3 = outlet_new(&x->x_obj, &s_float);
  x->x_outlet4 = outlet_new(&x->x_obj, &s_float);
  
  return (x);
}

static void time_bang(t_time *x)
{
  struct timeb mytime;
  struct tm *resolvetime;
  
  ftime(&mytime);
  resolvetime = (x->GMT)?gmtime(&mytime.time):localtime(&mytime.time);

  outlet_float(x->x_outlet4, (t_float)(mytime.millitm));
  outlet_float(x->x_outlet3, (t_float)resolvetime->tm_sec);
  outlet_float(x->x_outlet2, (t_float)resolvetime->tm_min);  
  outlet_float(x->x_outlet1, (t_float)resolvetime->tm_hour);
}

static void help_time(t_time *x)
{
  post("\n%c time\t\t:: get the current system time", HEARTSYMBOL);
  post("\noutputs are\t:  hour / minute / sec / msec");
  post("\ncreation\t:: 'time [GMT]': show local time or GMT");
}

void time_setup(void)
{
  time_class = class_new(gensym("time"),
			 (t_newmethod)time_new, 0,
			 sizeof(t_time), 0, A_GIMME, 0);
  
  class_addbang(time_class, time_bang);
  
  class_addmethod(time_class, (t_method)help_time, gensym("help"), 0);
  class_sethelpsymbol(time_class, gensym("zexy/time"));
}

/* ----------------------- date --------------------- */

static t_class *date_class;

typedef struct _date
{
  t_object x_obj;
  
  int GMT;
  
  t_outlet *x_outlet1;
  t_outlet *x_outlet2;
  t_outlet *x_outlet3;
} t_date;

static void *date_new(t_symbol *s, int argc, t_atom *argv)
{
  t_date *x = (t_date *)pd_new(date_class);
  char buf[5];
  
  x->GMT=0;
  if (argc) {
    atom_string(argv, buf, 5);
    if (buf[0]=='G' && buf[1]=='M' && buf[2]=='T')
      x->GMT = 1;
  }
  
  x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
  x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
  x->x_outlet3 = outlet_new(&x->x_obj, &s_float);
  
  return (x);
}

static void date_bang(t_date *x)
{
  struct timeb mytime;
  struct tm *resolvetime;
    
  ftime(&mytime);
  resolvetime=(x->GMT)?gmtime(&mytime.time):localtime(&mytime.time);
  
  outlet_float(x->x_outlet3, (t_float)resolvetime->tm_mday);
  outlet_float(x->x_outlet2, (t_float)resolvetime->tm_mon + 1);
  outlet_float(x->x_outlet1, (t_float)resolvetime->tm_year + 1900);
}

static void help_date(t_date *x)
{
  post("\n%c date\t\t:: get the current system date", HEARTSYMBOL);
  post("\noutputs are\t: year / month / day");
  post("\ncreation\t::'date [GMT]': show local date or GMT");
}

void date_setup(void)
{
  date_class = class_new(gensym("date"),
			 (t_newmethod)date_new, 0,
			 sizeof(t_date), 0, A_GIMME, 0);
  
  class_addbang(date_class, date_bang);
  
  class_addmethod(date_class, (t_method)help_date, gensym("help"), 0);
  class_sethelpsymbol(date_class, gensym("zexy/date"));
}


/*	general setup */


void z_datetime_setup(void)
{
  time_setup();
  date_setup();
}
