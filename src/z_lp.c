 /* 
   (c) 2005:forum::für::umläute:2000

   write to the parallel port
   extended to write to any port (if we do have permissions)

*/
#define BASE0  0x3bc
#define BASE1  0x378
#define BASE2  0x278

#define MODE_IOPERM 1
#define MODE_IOPL   0

#include "zexy.h"

#include <sys/io.h>
#include <stdlib.h>

/* ----------------------- lp --------------------- */

static int count_iopl = 0;

static t_class *lp_class;

typedef struct _lp
{
  t_object x_obj;

  unsigned long port;

  int mode; // MODE_IOPERM, MODE_IOPL
} t_lp;

static void lp_float(t_lp *x, t_floatarg f)
{
  if (x->port) {
    unsigned char b = f;
    outb(b, x->port);
  }
}

static void *lp_new(t_symbol *s, int argc, t_atom *argv)
{
  t_lp *x = (t_lp *)pd_new(lp_class);

  x->port = 0;
 
  if ((argc==0)||(argv->a_type==A_FLOAT)) {
    /* FLOAT specifies a parallel port */
    switch ((int)((argc)?atom_getfloat(argv):0)) {
    case 0:
      x->port = BASE0;
      break;
    case 1:
      x->port = BASE1;
      break;  
    case 2:
      x->port = BASE2;
      break;
    default:
      error("lp : only lp0, lp1 and lp2 are accessible");
      x->port = 0;
      return (x);
    }
  } else { 
    /* SYMBOL might be a file or a hex port-number;
       we ignore the file (device) case by now;
       LATER think about this
    */
    x->port=strtol(atom_getsymbol(argv)->s_name, 0, 16);
  }

  if (!x->port || x->port>65535){
    post("lp : bad port %x", x->port);
    x->port = 0;
    return (x);
  }

  if (x->port && x->port < 0x400){
    if (ioperm(x->port, 8, 1)) {
      error("lp : couldn't get write permissions");
      x->port = 0;
      return (x);
    }
    x->mode = MODE_IOPERM;
  } else {
    if (iopl(3)){
	error("lp : couldn't get write permissions");
	x->port = 0;
	return (x);
    }
    x->mode=MODE_IOPL;
    count_iopl++;
    post("iopl.............................%d", count_iopl);
  }
  
  post("connected to port %x in mode '%s'", x->port, (x->mode==MODE_IOPL)?"iopl":"ioperm");
  if (x->mode==MODE_IOPL)post("warning: this might seriously damage your pc...");
  
  return (x);
}

static void lp_free(t_lp *x)
{
  if (x->port) {
    if (x->mode==MODE_IOPERM && ioperm(x->port, 8, 0)) error("lp: couldn't clean up device");
    else if (x->mode==MODE_IOPL && (!--count_iopl) && iopl(0))
      error("lp: couldn't clean up device");
  }
}


static void helper(t_lp *x)
{
  post("\n%c lp :: direct access to the parallel port", HEARTSYMBOL);
  post("<byte>\t: write byte to the parallel-port");
  post("\ncreation:\t\"lp [<port>]\": connect to parallel port <port> (0..2)");
  post("\t\t\"lp <portaddr>\": connect to port @ <portaddr> (hex)");
}

void z_lp_setup(void)
{
  lp_class = class_new(gensym("lp"),
			  (t_newmethod)lp_new, (t_method)lp_free,
			  sizeof(t_lp), 0, A_GIMME, 0);

  class_addfloat(lp_class, (t_method)lp_float);

  class_addmethod(lp_class, (t_method)helper, gensym("help"), 0);
  class_sethelpsymbol(lp_class, gensym("zexy/lp"));
}
