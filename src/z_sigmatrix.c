#include "zexy.h"

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* the sigmatrix objects ::
   matrix~      : multiply a n-vector of in~ with a matrix to get a m-vector of out~
                  line~ between the 2 matrices, to make it useable as a mixer
   multiplex~   : multiplex 1-of-n in~ to 1 out~
   demultiplex~ : demultiplex 1 in~ to 1-of-n out~

 to do :
   patchbay~    : array of mux~ and demux~

*/


/* --------------------------- matrix~ ----------------------------------
 *
 * multiply a n-vector of signals with a (n*m) matrix, to get m output-streams.
 * make the (n*m)-matrix of scalars to be liny~
 *
 *  1703:forum::für::umläute:2001
 */

static t_class *sigmtx_class;

typedef struct _sigmtx {
  t_object x_obj;

  t_float time;
  int ticksleft;
  int retarget;
  t_float msec2tick;

  t_float **value;
  t_float **target;
  t_float **increment; /* single precision is really a bad, especially when doing long line~s.
			* but the biginc (like in msp's line~ (d_ctl.c) is far too expensive... */
  t_float **sigIN;
  t_float **sigOUT;
  t_float  *sigBUF;

  int       n_sigIN;    /* columns */
  int       n_sigOUT;   /* rows    */
} t_sigmtx;

/* the message thing */

static void sigmtx_matrix(t_sigmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row, c=0, r=0;

  if (argc<2){
    post("matrix~ : bad matrix !");
    return;
  }

  row = atom_getfloat(argv++);
  col = atom_getfloat(argv++);
  argc-=2;

  if((col!=x->n_sigOUT)||(row!=x->n_sigIN)){
    post("matrix~ : matrix dimensions do not match !!");
    return;
  }
  if(argc<row*col){
    post("matrix~ : reduced matrices not yet supported");
    return;
  }

  if (x->time<=0) {
    for(r=0; r<row; r++)
      for(c=0; c<col; c++)
	x->target[c][r]=x->value[c][r]=atom_getfloat(argv++);
    x->time=x->ticksleft=x->retarget=0;
  } else {
    for(r=0; r<row; r++)
      for(c=0; c<col; c++)
	x->target[c][r]=atom_getfloat(argv++);
    x->retarget=1;
  }
}

static void sigmtx_stop(t_sigmtx *x)
{
  int c = x->n_sigOUT, r;
  t_float *tgt, *val;
  while(c--){
    tgt=x->target[c];
    val=x->value [c];
    r=x->n_sigIN;
    while(r--)*tgt++=*val++;
  }
  x->ticksleft = x->retarget = 0;
}


/* the dsp thing */

static t_int *sigmtx_perform(t_int *w)
{
  t_sigmtx *x = (t_sigmtx *)(w[1]);
  int n = (int)(w[2]);

  int r, c;

  t_float **out = x->sigOUT;
  t_float **in  = x->sigIN;

  t_float  *buf = x->sigBUF, *sigBUF = buf;

  t_float **value     = x->value;
  t_float **target    = x->target;
  t_float **increment = x->increment;

  t_float *inc, *val, *tgt;

  int n_IN=x->n_sigIN, n_OUT=x->n_sigOUT;

  if (x->retarget) {
    int nticks = x->time * x->msec2tick;
    t_float oneovernos;

    if (!nticks) nticks = 1;
    oneovernos = 1./(nticks*n);
    x->ticksleft = nticks;

    c = n_OUT;
    while(c--) {
      inc=increment[c];
      val=value[c];
      tgt=target[c];
      r=n_IN;
      while(r--)*inc++=(*tgt++-*val++)*oneovernos;
    }

    x->retarget = 0;
  }

  if (x->ticksleft) {
    int N=n-1;
    n=-1;
    //    while (n--) {
    while(n++<N){
      c = n_OUT;
      while(c--) {
	t_float sum = 0;
	val = value[c]+n_IN-1;
	inc = increment[c]+n_IN-1;
	r=n_IN;

	while(r--)sum+=in[r][n]*(*val--+=*inc--);

	sigBUF[c]=sum;
      }
      buf = sigBUF;
      c = n_OUT;
      while(c--)out[c][n]=*buf++;
    }
    if (!--x->ticksleft) {
      c = n_OUT;
      while(c--){
	val=value[c];
	tgt=target[c];
	r=n_IN;
	while(r--)*val++=*tgt++;
      }
    }
  } else { /* no ticks left */
    while (n--) {
      c = n_OUT;
      while(c--) {
	t_float sum = 0;
	val = value[c]+n_IN-1;
	r = n_IN;
	while(r--)sum+=in[r][n]**val--; 
	sigBUF[c]=sum;
      }
      buf = sigBUF;
      c = n_OUT;
      while(c--)out[c][n]=*buf++;
    }
  }
  return (w+3);
}

static void sigmtx_dsp(t_sigmtx *x, t_signal **sp)
{
  int o = x->n_sigOUT, i=x->n_sigIN, n=0;
  t_float **dummy = x->sigIN;

  while(i--)*dummy++=sp[n++]->s_vec;

  dummy =x->sigOUT;
  while(o--)dummy[o]=sp[n++]->s_vec;

  x->msec2tick = sp[0]->s_sr / (1000.f * sp[0]->s_n);
  dsp_add(sigmtx_perform, 2, x, sp[0]->s_n);
}


/* setup/setdown things */

static void sigmtx_free(t_sigmtx *x)
{
  int i = x->n_sigOUT;
  while(i--) {
    freebytes(x->value    [i], x->n_sigOUT * sizeof(t_float *));
    freebytes(x->target   [i], x->n_sigOUT * sizeof(t_float *));
    freebytes(x->increment[i], x->n_sigOUT * sizeof(t_float *));
  }

  freebytes(x->value,     sizeof(x->value));
  freebytes(x->target,    sizeof(x->target));
  freebytes(x->increment, sizeof(x->increment));

  freebytes(x->sigIN,    x->n_sigIN  * sizeof(t_float *));
  freebytes(x->sigOUT,   x->n_sigOUT * sizeof(t_float *));
  freebytes(x->sigBUF,   x->n_sigOUT * sizeof(t_float  ));
}

static void *sigmtx_new(t_symbol *s, int argc, t_atom *argv)
{
    t_sigmtx *x = (t_sigmtx *)pd_new(sigmtx_class);
    int i;

    x->time = 0;

    switch (argc) {
    case 0:
      x->n_sigIN = x->n_sigOUT = 1;
     break;
    case 1:
      x->n_sigIN = x->n_sigOUT = atom_getfloat(argv);
      break;
    default:
      x->time= atom_getfloat(argv+2);
    case 2:
      x->n_sigIN  = atom_getfloat(argv);
      x->n_sigOUT = atom_getfloat(argv+1);
      break;
    }

    if (x->time<0) x->time=0;
    if (x->n_sigIN <1) x->n_sigIN =1;
    if (x->n_sigOUT<1) x->n_sigOUT=1;

    /* the inlets */
    i=x->n_sigIN-1;
    while(i--)inlet_new(&x->x_obj,&x->x_obj.ob_pd,&s_signal,&s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    floatinlet_new(&x->x_obj, &x->time);

    /* the outlets */
    i=x->n_sigOUT;
    while(i--)outlet_new(&x->x_obj,&s_signal);

    /* make all the buffers */
    x->sigIN  = (t_float **)getbytes(x->n_sigIN  * sizeof(t_float *));
    x->sigOUT = (t_float **)getbytes(x->n_sigOUT * sizeof(t_float *));
    x->sigBUF = (t_float  *)getbytes(x->n_sigOUT * sizeof(t_float  ));

    x->value     = (t_float **)getbytes(x->n_sigOUT * sizeof(t_float));
    x->target    = (t_float **)getbytes(x->n_sigOUT * sizeof(t_float));
    x->increment = (t_float **)getbytes(x->n_sigOUT * sizeof(t_float));

    i = x->n_sigOUT;
    while(i--){
      int j = x->n_sigIN;
      x->sigOUT   [i] = 0;
      x->value    [i] = (t_float *)getbytes(x->n_sigIN * sizeof(t_float));
      x->target   [i] = (t_float *)getbytes(x->n_sigIN * sizeof(t_float));
      x->increment[i] = (t_float *)getbytes(x->n_sigIN * sizeof(t_float));

      while(j--)x->value[i][j]=x->target[i][j]=x->increment[i][j]=0;
    }

    i = x->n_sigIN;
    while(i--)x->sigIN[i] = 0;    

    x->msec2tick = x->ticksleft = x->retarget = 0;
    return (x);
}

static void sigmtx_setup(void)
{
  sigmtx_class = class_new(gensym("matrix~"), (t_newmethod)sigmtx_new, (t_method)sigmtx_free,
			  sizeof(t_sigmtx), 0, A_GIMME, 0);

  class_addmethod(sigmtx_class, (t_method)sigmtx_dsp, gensym("dsp"), 0);
  class_addmethod(sigmtx_class, nullfn, gensym("signal"), 0);

  class_addmethod(sigmtx_class, (t_method)sigmtx_matrix, gensym(""), A_GIMME, 0);
  class_addmethod(sigmtx_class, (t_method)sigmtx_stop, gensym("stop"), 0);

  class_sethelpsymbol(sigmtx_class, gensym("zexy/matrix~"));
}

/* ------------------------------------------------------------------------------ */

/* demux~ : demultiplex a signal to a specified output */

static t_class *demux_class;

typedef struct _demux {
  t_object x_obj;

  int output;

  int n_out;
  t_float **out;

} t_demux;

static void demux_output(t_demux *x, t_floatarg f)
{
  if ((f>=0)&&(f<x->n_out)){
    x->output=f;
  } else
    error("demultiplex: %d is channel out of range (0..%d)", (int)f, x->n_out);
}


static t_int *demux_perform(t_int *w)
{
  t_demux *x = (t_demux *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  int N = (int)(w[3]);
  int n = N;


  int channel=x->n_out;


  while(channel--){
    t_float*out=x->out[channel];
    n=N;
    if(x->output==channel){
      while(n--)*out++=*in++;
    } else
      while(n--)*out++=0.f;
  }
  return (w+4);
}

static void demux_dsp(t_demux *x, t_signal **sp)
{
  int n = x->n_out;
  t_float **dummy=x->out;
  while(n--)*dummy++=sp[x->n_out-n]->s_vec;
  dsp_add(demux_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}


static void demux_helper(void)
{
  post("\n%c demux~\t:: demultiplex a signal to one of various outlets", HEARTSYMBOL);
  post("<#out>\t : the outlet-number (counting from 0) to witch the inlet is routed"
       "'help'\t : view this");
  post("creation : \"demux~ [arg1 [arg2...]]\"\t: the number of arguments equals the number of outlets\n");
}

static void demux_free(t_demux *x)
{
  freebytes(x->out, x->n_out * sizeof(t_float *));
}

static void *demux_new(t_symbol *s, int argc, t_atom *argv)
{
	t_demux *x = (t_demux *)pd_new(demux_class);
	int i;

	if (!argc)argc=2;
	x->n_out=argc;
	x->output=0;

	while(argc--)outlet_new(&x->x_obj, gensym("signal"));

	x->out = (t_float **)getbytes(x->n_out * sizeof(t_float *));
	i=x->n_out;
	while(i--)x->out[i]=0;

	return (x);
}

void demux_setup(void)
{
	demux_class = class_new(gensym("demultiplex~"), (t_newmethod)demux_new, (t_method)demux_free, sizeof(t_demux), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)demux_new, gensym("demux~"), A_GIMME, 0);

	class_addfloat(demux_class, demux_output);
	class_addmethod(demux_class, (t_method)demux_dsp, gensym("dsp"), 0);
	class_addmethod(demux_class, nullfn, gensym("signal"), 0);

	class_addmethod(demux_class, (t_method)demux_helper, gensym("help"), 0);
	class_sethelpsymbol(demux_class, gensym("zexy/demultiplex~"));
}


/* ------------------------------------------------------------------------------ */

/* mux~ : multiplex a specified signal to the output */

static t_class *mux_class;

typedef struct _mux {
  t_object x_obj;

  int input;

  int n_in;
  t_float **in;
} t_mux;

static void mux_input(t_mux *x, t_floatarg f)
{
  if ((f>=0)&&(f<x->n_in)){
    x->input=f;
  } else
    error("multiplex: %d is channel out of range (0..%d)", (int)f, x->n_in);

}

static t_int *mux_perform(t_int *w)
{
  t_mux *x = (t_mux *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = (int)(w[3]);
  
  t_float *in = x->in[x->input];

  while(n--)*out++=*in++;

  return (w+4);
}

static void mux_dsp(t_mux *x, t_signal **sp)
{
  int n = 0;
  t_float **dummy=x->in;

  for(n=0;n<x->n_in;n++)*dummy++=sp[n]->s_vec;

  dsp_add(mux_perform, 3, x, sp[n]->s_vec, sp[0]->s_n);
}

static void mux_helper(void)
{
  post("\n%c mux~\t:: multiplex a one of various signals to one outlet", HEARTSYMBOL);
  post("<#out>\t : the inlet-number (counting from 0) witch is routed to the outlet"
       "'help'\t : view this");
  post("creation : \"mux~ [arg1 [arg2...]]\"\t: the number of arguments equals the number of inlets\n");
}

static void mux_free(t_mux *x)
{
  freebytes(x->in, x->n_in * sizeof(t_float *));
}

static void *mux_new(t_symbol *s, int argc, t_atom *argv)
{
	t_mux *x = (t_mux *)pd_new(mux_class);
	int i;

	if (!argc)argc=2;
	x->n_in=argc;
	x->input=0;

	argc--;
	while(argc--)inlet_new(&x->x_obj,&x->x_obj.ob_pd,&s_signal,&s_signal);

	x->in = (t_float **)getbytes(x->n_in * sizeof(t_float *));
	i=x->n_in;
	while(i--)x->in[i]=0;

	outlet_new(&x->x_obj, gensym("signal"));

	return (x);
}

void mux_setup(void)
{
	mux_class = class_new(gensym("multiplex~"), (t_newmethod)mux_new, (t_method)mux_free, sizeof(t_mux), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)mux_new, gensym("mux~"), A_GIMME, 0);

	class_addfloat(mux_class, mux_input);
	class_addmethod(mux_class, (t_method)mux_dsp, gensym("dsp"), 0);
	class_addmethod(mux_class, nullfn, gensym("signal"), 0);

	class_addmethod(mux_class, (t_method)mux_helper, gensym("help"), 0);
	class_sethelpsymbol(mux_class, gensym("zexy/multiplex~"));
}

/* ----------------------------------------------------------------------
 * main setup
 * ---------------------------------------------------------------------- */

void z_sigmatrix_setup(void)
{
  sigmtx_setup();
  demux_setup();
  mux_setup();
}
