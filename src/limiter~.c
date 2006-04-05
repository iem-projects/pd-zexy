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
	--------------------------------- limiter/compressor ---------------------------------
	
	for details on how it works watch out for   "http://iem.kug.ac.at/~zmoelnig/pd"
	...and search for "limiter"

	mail2me4more!n4m8ion : zmoelnig@iem.kug.ac.at
*/

/* 
   this is a limiter/compressor-object 
   the limiter is based on Falkner's thesis 
   "Entwicklung eines digitalen Stereo-limiters mit Hilfe des Signalprozessors DSP56001" pp.14

   2108:forum::für::umläute:1999		all rights reserved and no warranties...

   see GNU-license for details
*/

#define LIMIT0 0
#define LIMIT1 1
#define COMPRESS 2

#include "zexy.h"

#define LN2 .69314718056
#define SINC1 .822462987
#define SINC2 .404460777
#define SINC3 -.188874003
#define SINC4 -.143239449
#define SINC5 .087796546
#define SINC6 .06917082
#define SINC7 -.041349667
#define SINC8 -.030578954
#define SINC9 .013226276

#define BUFSIZE 128
#define XTRASAMPS 9
#define TABLESIZE 512 /* compressor table */

/* ------------------------------------------------------------------------------------ */
// first define the structs...

static t_class *limiter_class;

typedef struct _limctl
{		// variables changed by user
  float limit, hold_samples, change_of_amplification;
} t_limctl;

typedef struct _cmpctl
{
  float treshold, ratio;		// uclimit is the very same is the limiter1-limit (decalculated relative to our treshold)
  float uclimit, climit_inverse;	// climit == compressed limit (uclimit == uncompressed limit)

  float limiter_limit;		// start limiting (stop compressing); == tresh/limit;

  float treshdB, oneminusratio;
} t_cmpctl;

typedef struct _inbuf
{
  float*		ringbuf;
  int		buf_position;
} t_inbuf;

typedef struct _limiter
{
  t_object	x_obj;

  int		number_of_inlets, s_n;

  // variables changed by process

  float		amplification;
  float		samples_left, still_left;

  int		mode;

  t_limctl	*val1, *val2;
  t_cmpctl	*cmp;

  // note :	limit is not the same for val1 & val2 :
  //			at val1 it is the limit of the INPUT_VALUE
  //			at val2 it is the limit for the AMPLIFICATION (in fact it is abs_limit1/abs_limit2)

  t_inbuf*	in;
  int		buf_size;

} t_limiter;

/* ------------------------------------------------------------------------------------ */
// then do the message - thing

// do the user settings

// calcs
static t_float calc_holdsamples(t_float htime, int buf)
{	// hold_time must be greater than buffer_time to make sure that any peak_sample is amplified with its own factor
  float min_hold = buf / sys_getsr();
  return (0.001 * sys_getsr() * ((htime > min_hold)?htime:((min_hold > 50)?min_hold:50)));
}

static t_float calc_coa(t_float hlife)
{
  return (exp(LN2 * 1000 / (((hlife > 0)?hlife:15) * sys_getsr())));
}

static void set_uclimit(t_limiter *x)
{
  t_cmpctl *c = x->cmp;
  t_float limit = x->val1->limit, limitdB = rmstodb(limit), ratio = c->ratio, tresh = c->treshold, treshdB = rmstodb(tresh);

  c->climit_inverse  = limit / tresh;
  c->uclimit = tresh / dbtorms(treshdB+(limitdB - treshdB)/ratio);

  c->treshdB = treshdB;
  c->oneminusratio = 1. - ratio;
}

// settings

static void set_treshold(t_limiter *x, float treshold)
{
  t_cmpctl *c = x->cmp;
  float tresh = dbtorms (treshold);
  if (tresh > x->val1->limit) tresh = x->val1->limit;

  c->treshold = tresh;

  set_uclimit(x);
}

static void set_ratio(t_limiter *x, float ratio)
{
  if (ratio < 0) ratio = 1;
  x->cmp->ratio = ratio;

  set_uclimit(x);
}

static void set_mode(t_limiter *x, float  mode)
{
  int modus = mode;

  switch (modus) {
  case LIMIT0:
    x->mode = LIMIT0;
    break;
  case LIMIT1:
    x->mode = LIMIT1;
    break;
  case COMPRESS:
    x->mode = COMPRESS;
    break;
  default:
    x->mode = LIMIT0;
    break;
  }
  //  post("mode set to %d", x->mode);
}

static void set_LIMIT(t_limiter *x)
{
  set_mode(x, LIMIT0);
}

static void set_CRACK(t_limiter *x)
{
  set_mode(x, LIMIT1);
}

static void set_COMPRESS(t_limiter *x)
{
  set_mode(x, COMPRESS);
}

static void set_bufsize(t_limiter *x, float size)
{ // this is really unneeded...and for historical reasons only
  if (size < BUFSIZE) size = BUFSIZE;
  x->buf_size = size + XTRASAMPS;
}

static void set_limit(t_limiter *x, t_floatarg limit)
{
  if (limit < 0.00001) limit = 100;
  x->val1->limit = dbtorms(limit);

  if (x->val1->limit < x->cmp->treshold) x->cmp->treshold = x->val1->limit;
  set_uclimit(x);
}

static void set_limits(t_limiter *x, t_floatarg limit1, t_floatarg limit2)
{
  t_float lim1, lim2;

  if (limit1 < 0.00001) limit1 = 100;

  lim1 = dbtorms(limit1);
  lim2 = dbtorms(limit2);

  if (lim2 < lim1)
    {
      lim2 = 2*lim1;	// this is to prevent lim2 (which should trigger the FAST regulation)
      x->mode = 0;	// to underrun the SLOW regulation; this would cause distortion
    }

  x->val1->limit = lim1;
  x->val2->limit = lim1/lim2;

  if (lim1 < x->cmp->treshold) x->cmp->treshold = lim1;
  set_uclimit(x);
}

static void set1(t_limiter *x, t_floatarg limit, t_floatarg hold, t_floatarg release)
{
  t_float lim = dbtorms(limit);

  x->val1->limit = (lim > 0)?lim:1;
  x->val1->hold_samples = calc_holdsamples(hold, x->buf_size);
  x->val1->change_of_amplification = calc_coa(release);

  if (lim < x->cmp->treshold) x->cmp->treshold = lim;
  set_uclimit(x);
}


static void set2(t_limiter *x, t_floatarg limit, t_floatarg hold, t_floatarg release)
{
  t_float lim = dbtorms(limit);
  x->val2->limit = (lim > x->val1->limit)?(x->val1->limit/lim):.5;
  x->val2->hold_samples = calc_holdsamples(hold, x->buf_size);
  x->val2->change_of_amplification = calc_coa(release);
}



static void set_compressor(t_limiter *x, t_floatarg limit, t_floatarg treshold, t_floatarg ratio)
{
  t_cmpctl *c = x->cmp;
  t_float lim = dbtorms(limit);
  t_float tresh = dbtorms(treshold);

  if ((limit == 0) && (treshold == 0) && (ratio == 0)) {set_mode(x, COMPRESS); return;}

  if (tresh > lim) tresh = lim;
  if (ratio < 0.) ratio = 1.;

  c->ratio = ratio;
  x->val1->limit = lim;
  c->treshold = tresh;
  set_uclimit(x);

  set_mode(x, COMPRESS);
}

static void reset(t_limiter *x)
{
  x->amplification = 1.;
}

// verbose
static void status(t_limiter *x)
{
  t_limctl *v1 = x->val1;
  t_limctl *v2 = x->val2;
  t_cmpctl *c = x->cmp;

  t_float sr = sys_getsr() / 1000.;

  switch (x->mode) {
  case LIMIT1:
    post("%d-channel crack-limiter @ %fkHz\n"
	 "\noutput-limit\t= %fdB\nhold1\t\t= %fms\nrelease1\t= %fms\ncrack-limit\t= %fdB\nhold2\t\t= %fms\nrelease2\t= %fms\n"
	 "\namplify\t\t= %fdB\n",
	 x->number_of_inlets, sr,
	 rmstodb(v1->limit), (v1->hold_samples) / sr, LN2 / (log(v1->change_of_amplification) * sr),
	 rmstodb(v1->limit / v2->limit), (v2->hold_samples) / sr, LN2 / (log(v2->change_of_amplification) * sr),
	 x->amplification);
    break;
  case LIMIT0:
    post("%d-channel limiter @ %fkHz\n"
	 "\noutput-limit\t= %fdB\nhold\t\t= %fms\nrelease\t\t= %fms\n"
	 "\namplify\t\t= %fdB\n",
	 x->number_of_inlets, sr,
	 rmstodb(v1->limit), (v1->hold_samples) / sr, LN2 / (log(v1->change_of_amplification) * sr),
	 rmstodb(x->amplification));
    break;
  case COMPRESS:
    post("%d-channel compressor @ %fkHz\n"
	 "\noutput-limit\t= %fdB\ntreshold\t= %fdB\ninput-limit\t= %f\nratio\t\t= 1:%f\n"
	 "\nhold\t\t= %fms\nrelease\t\t= %fms\n"
	 "\namplify\t\t= %fdB\n",
	 x->number_of_inlets, sr,
	 rmstodb(c->treshold * c->climit_inverse), rmstodb(c->treshold), rmstodb(c->treshold / c->uclimit), 1./c->ratio,
	 (v1->hold_samples) / sr, LN2 / (log(v1->change_of_amplification) * sr),
	 rmstodb(x->amplification));
  }
}

static void limiter_tilde_helper(t_limiter *x)
{
  post("\n\n%c %d-channel limiter-object: mode %d", HEARTSYMBOL, x->number_of_inlets, x->mode);
  poststring("\n'mode <mode>'\t\t\t: (0_limiter, 1_crack-limiter, 2_compressor)");
  poststring("\n'LIMIT'\t\t\t\t: set to LIMITer");
  poststring("\n'CRACK'\t\t\t\t: set to CRACK-limiter");
  poststring("\n'COMPRESS'\t\t\t\t: set to COMPRESSor");

  switch (x->mode) {
  case LIMIT0:
    poststring("\n'limit <limit>'\t\t\t: set limit (in dB)"
	       "\n'set <limit><htime><rtime>'\t: set limiter");
    break;
  case LIMIT1:
    poststring("\n'limits <limit1><limit2>'\t: set limits (in dB)"
	       "\n'set  <limit1><htime1><rtime1>'\t: set limiter 1"
	       "\n'set2 <limit2><htime2><rtime2>'\t: set crack-limiter");
    break;
  case COMPRESS:
    poststring("\n'ratio <compressratio>'\t\t: set compressratio (´0.5´ instead of ´1:2´)"
	       "\n'treshold <treshold>'\t\t: set treshold of the compressor"
	       "\n'compress <limit><treshold><ratio>'\t: set compressor"
	       "\n..........note that <limit> is the same for COMPRESSOR and LIMITER..........");
    break;
  default:
    break;
  }
  poststring("\n'print'\t\t\t\t: view actual settings"	
	     "\n'help'\t\t\t\t: view this\n");
  poststring("\ncreating arguments are :\n"
	     "\"limiter~ [<in1> [<in2> [<in3> [...]]]]\":	<in*> may be anything\n");
  endpost();
}


/* ------------------------------------------------------------------------------------ */
// now do the dsp - thing							        //
/* ------------------------------------------------------------------------------------ */

static t_int *oversampling_maxima(t_int *w)
{
  t_limiter *x = (t_limiter *)w[1];
  t_inbuf *buf = (t_inbuf *)w[2];
  t_float *in	 = (t_float *)w[3];
  t_float *out = (t_float *)w[4];

  int n = x->s_n;
  int bufsize = x->buf_size;

  int i = buf->buf_position;

  t_float *vp = buf->ringbuf, *ep = vp + bufsize, *bp = vp + XTRASAMPS + i;

  i += n;

  while (n--)
    {
      t_float os1, os2, max;
      t_float last4, last3, last2, last1, sinccurrent, current, next1, next2, next3, next4;

      if (bp == ep)
	{
	  vp[0] = bp[-9];
	  vp[1] = bp[-8];
	  vp[2] = bp[-7];
	  vp[3] = bp[-6];
	  vp[4] = bp[-5];
	  vp[5] = bp[-4];
	  vp[6] = bp[-3];
	  vp[7] = bp[-2];
	  vp[8] = bp[-1];

	  bp = vp + XTRASAMPS;
	  i -= bufsize - XTRASAMPS;
	}

      os1= fabsf(SINC8 * (last4 = bp[-8]) +
		 SINC6 * (last3 = bp[-7]) +
		 SINC4 * (last2 = bp[-6]) +
		 SINC2 * (last1 = bp[-5]) +
		 (sinccurrent = SINC1 * (current = bp[-4])) +
		 SINC3 * (next1 = bp[-3]) +
		 SINC5 * (next2 = bp[-2]) +
		 SINC7 * (next3 = bp[-1]) +
		 SINC9 * (next4 = bp[0]));

      os2= fabsf(SINC8 * next4 +
		 SINC4 * next3 +
		 SINC6 * next2 +
		 SINC2 * next1 +
		 sinccurrent +
		 SINC3 * last1 +
		 SINC5 * last2 +
		 SINC7 * last3 +
		 SINC9 * last4);

      max = fabsf(current);

      if (max < os1) 
	{
	  max = os1;
	}
      if (max < os2) 
	{
	  max = os2;
	}
		
      *bp++ = *in++;
      if (*out++ < max) *(out-1) = max;
    }
  buf->buf_position = i;

  return (w+5);
}


static t_int *limiter_perform(t_int *w)
{
  t_limiter *x=(t_limiter *)w[1];
  int n = x->s_n;

  t_float *in	= (t_float *)w[2];
  t_float *out= (t_float *)w[3];

  t_limctl *v1	= (t_limctl *)(x->val1);
  t_limctl *v2	= (t_limctl *)(x->val2);
  t_cmpctl *c     = (t_cmpctl *)(x->cmp);

  // now let's make things a little bit faster

  // these must not be changed by process
  const t_float limit		= v1->limit;
  const t_float holdlong	= v1->hold_samples;
  const t_float coa_long	= v1->change_of_amplification;

  const t_float alimit		= v2->limit;
  const t_float holdshort	= v2->hold_samples;
  const t_float coa_short	= v2->change_of_amplification;

  t_float tresh  = c->treshold;
  t_float uclimit = c->uclimit;
  t_float climit_inv = c->climit_inverse;

  t_float oneminusratio = c->oneminusratio;

  // these will be changed by process
  t_float amp				= x->amplification;
  t_float samplesleft		= x->samples_left;
  t_float stillleft		= x->still_left;

  // an intern variable...
  t_float max_val;
	
  switch (x->mode) {
  case LIMIT0:
    while (n--)
      {
	max_val = *in;

	// the MAIN routine for the 1-treshold-limiter

	if ((max_val * amp) > limit)
	  {
	    amp = limit / max_val;
	    samplesleft = holdlong;
	  } else
	    {
	      if (samplesleft > 0)
		{
		  samplesleft--;
		} else
		  {
		    if ((amp *= coa_long) > 1) amp = 1;
		  }
	    }

	*out++ = amp;
	*in++ = 0;
      }
    break;
  case LIMIT1:
    while (n--)
      {
	max_val = *in;
	// the main routine 2

	if ((max_val * amp) > limit)
	  {
	    samplesleft = ((amp = (limit / max_val)) < alimit)?holdshort:holdlong;
	    stillleft = holdlong;
	  } else
	    {
	      if (samplesleft > 0)
		{
		  samplesleft--;
		  stillleft--;
		} else
		  {
		    if (amp < alimit)
		      {
			if ((amp *= coa_short) > 1) amp = 1;
		      } else
			{
			  if (stillleft > 0)
			    {
			      samplesleft = stillleft;
			    } else
			      {
				if ((amp *= coa_long) > 1) amp = 1;
			      }
			}
		  }
	    }
	*out++ = amp;
	*in++ = 0;
      }
    x->still_left = stillleft;
    break;
  case COMPRESS:
    while (n--)
      {
	max_val = *in;

	// the MAIN routine for the compressor (very similar to the 1-treshold-limiter)

	if (max_val * amp > tresh) {
	  amp = tresh / max_val;
	  samplesleft = holdlong;
	} else
	  if (samplesleft > 0) samplesleft--;
	  else if ((amp *= coa_long) > 1) amp = 1;

	if (amp < 1.)
	  if (amp > uclimit)  // amp is still UnCompressed  uclimit==limitIN/tresh;
	    *out++ = pow(amp, oneminusratio);
	  else *out++ = amp * climit_inv; // amp must fit for limiting : amp(new) = limit/maxval; = amp(old)*limitOUT/tresh;
	else *out++ = 1.;

	*in++ = 0.;
      }
    break;
  default:
    while (n--) *out++ = *in++ = 0.;
    break;
  }

  // now return the goodies
  x->amplification	= amp;
  x->samples_left		= samplesleft;

  return (w+4);
}


static void limiter_dsp(t_limiter *x, t_signal **sp)
{
  int i = 0;
  t_float* sig_buf = (t_float *)getbytes(sizeof(t_float) * sp[0]->s_n);

  x->s_n = sp[0]->s_n;

  if (x->amplification == 0) x->amplification = 0.0000001;

  if (x->val2->limit >= 1) x->mode = 0;

  while (i < x->number_of_inlets)
    {
      dsp_add(oversampling_maxima, 4, x, &(x->in[i]), sp[i]->s_vec, sig_buf);
      i++;
    }

  dsp_add(limiter_perform, 3, x, sig_buf, sp[i]->s_vec);
}



/* ------------------------------------------------------------------------------------ */
// finally do the creation - things

static void *limiter_new(t_symbol *s, int argc, t_atom *argv)
{
  t_limiter *x = (t_limiter *)pd_new(limiter_class);
  ZEXY_USEVAR(s);

  int i = 0;

  if (argc) set_bufsize(x, atom_getfloat(argv));
  else
    {
      argc = 1;
      set_bufsize(x, 0);
    }

  if (argc > 64)	argc=64;
  if (argc == 0)	argc=1;

  x->number_of_inlets = argc--;

  while (argc--)
    {
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    }

  outlet_new(&x->x_obj, &s_signal);

  x->in = (t_inbuf*)getbytes(sizeof(t_inbuf) * x->number_of_inlets);
  while (i < x->number_of_inlets)
    {
      int n;
      t_float* buf = (float *)getbytes(sizeof(float) * x->buf_size);
      x->in[i].ringbuf		= buf;
      x->in[i].buf_position	= 0;
      for (n = 0; n < x->buf_size; n++) x->in[i].ringbuf[n] = 0.;
      i++;
    }

  x->val1 = (t_limctl *)getbytes(sizeof(t_limctl));
  x->val2 = (t_limctl *)getbytes(sizeof(t_limctl));
  x->cmp = (t_cmpctl *)getbytes(sizeof(t_cmpctl));

  x->cmp->ratio = 1.;
  x->cmp->treshold = 1;

  set1(x, 100, 30, 139);
  set2(x, 110, 5, 14.2);

  x->amplification= 1;
  x->samples_left	= x->still_left = x->mode = 0;

  return (x);
}

static void limiter_free(t_limiter *x)
{
  int i=0;

  freebytes(x->val1, sizeof(t_limctl));
  freebytes(x->val2, sizeof(t_limctl));
  freebytes(x->cmp , sizeof(t_cmpctl));

  while (i < x->number_of_inlets)	freebytes(x->in[i++].ringbuf, x->buf_size * sizeof(t_float));

  freebytes(x->in, x->number_of_inlets * sizeof(t_inbuf));
}



/* ------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------ */



void limiter_tilde_setup(void)
{
  limiter_class = class_new(gensym("limiter~"), (t_newmethod)limiter_new, (t_method)limiter_free,
			    sizeof(t_limiter), 0, A_GIMME, 0);

  class_addmethod(limiter_class, nullfn,					gensym("signal"), 0);
  class_addmethod(limiter_class, (t_method)limiter_dsp,	gensym("dsp"), 0);

  class_addmethod(limiter_class, (t_method)limiter_tilde_helper,	gensym("help"), 0);
  class_addmethod(limiter_class, (t_method)status,	gensym("print"), 0);
  class_sethelpsymbol(limiter_class, gensym("zexy/limiter~"));

  class_addmethod(limiter_class, (t_method)set_mode,	gensym("mode"), A_FLOAT, 0);
  class_addmethod(limiter_class, (t_method)set_LIMIT,	gensym("LIMIT"),  0);
  class_addmethod(limiter_class, (t_method)set_CRACK,	gensym("CRACK"),  0);
  class_addmethod(limiter_class, (t_method)set_COMPRESS,	gensym("COMPRESS"), 0);


  class_addmethod(limiter_class, (t_method)set_treshold,	gensym("tresh"), A_FLOAT, 0);
  class_addmethod(limiter_class, (t_method)set_treshold,	gensym("treshold"), A_FLOAT, 0);
  class_addmethod(limiter_class, (t_method)set_ratio,	gensym("ratio"), A_FLOAT, 0);
  class_addmethod(limiter_class, (t_method)set1,		gensym("set"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
  class_addmethod(limiter_class, (t_method)set2,		gensym("set2"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
  class_addmethod(limiter_class, (t_method)set_compressor,gensym("compress"), A_FLOAT, A_FLOAT, A_FLOAT, 0);

  class_addmethod(limiter_class, (t_method)set_limits,	gensym("limits"), A_FLOAT, A_FLOAT, 0);
  class_addmethod(limiter_class, (t_method)set_limit,	gensym("limit"), A_FLOAT, 0);
  class_addfloat (limiter_class, set_limit);

  class_addmethod(limiter_class, (t_method)reset,		gensym("reset"), 0);

  zexy_register("limiter~");
}
