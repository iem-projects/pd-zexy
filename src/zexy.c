/* ...this is a very ZEXY external ...
   so have fun
	
   1999:forum::für::umläute:2001
*/

#include "zexy.h"

/* do a little help thing */

typedef struct zexy 
{
  t_object t_ob;
} t_zexy;

t_class *zexy_class;

static void zexy_help(void)
{
  post("\n\n...this is the zexy %c external "VERSION"...\n", HEARTSYMBOL);
  post("\n%c handling signals"
#if 0
       "\nstreamout~\t:: stream signals via a LAN : (%c) gige 1999"
       "\nstreamin~\t:: catch signals from a LAN : based on gige"
#endif
       "\nsfplay\t\t:: play back a (multichannel) soundfile : (%c) ritsch 1999"
       "\nsfrecord\t:: record a (multichannel) soundfile : based on ritsch"
       "\n%c generating signals"
       "\nnoish~\t\t:: generate bandlimited noise"
       "\nnoisi~\t\t:: generate bandlimited noise"
       "\ndirac~\t\t:: generate a dirac-pulse"
       "\nstep~\t\t:: generate a unity-step"
       "\ndfreq~\t\t:: detect frequency by counting zero-crossings : (%c) ritsch 1998"
       "\n%c manipulating signals"
       "\nlimiter~\t:: limit/compress one or more signals"
       "\nnop~\t\t:: pass through a signal (delay 1 block)"
       "\nz~\t\t:: samplewise delay"
       "\nswap~\t\t:: byte-swap a signal"
       "\nquantize~\t:: quantize a signal"

       "\n%c binary operations on signals"
       "\nabs~, sgn~, >~, <~, ==~, &&~, ||~"

       "\n%c multary operations on signals"

       "\nmatrix~\t\t:: handle matrices"
       "\nmultiline~\t:: multiple line~ multiplication"
       "\nmultiplex~\t:: multiplex 1 inlet~ to 1-of-various outlet~s"
       "\ndemultiplex~\t:: demultiplex 1-of-various inlet~s to 1 outlet~"

       "\n%c investigating signals in message-domain"
       "\npack~\t\t:: convert a signal into a list of floats"
       "\nunpack~\t\t:: convert packages of floats into a signal"

       "\nsigzero~\t:: indicates whether a signal is zero throughout the block"
       "\navg~\t\t:: outputs average of a signal as float"
       "\ntavg~\t\t:: outputs average of a signal between two bangs"
       "\nenvrms~\t\t:: an env~-object that ouputs rms instead of db"
       "\npdf~\t\t:: power density function"
       
       "\n%c basic message objects"
       "\nnop\t\t:: a no-operation"
       "\nlister\t\t:: stores lists"
       "\nany2list\t\t:: converts \"anything\" to lists"
       "\nlist2int\t:: cast each float of a list to integer"
       "\natoi\t\t:: convert ascii to integer"
       "\nlist2symbol\t:: convert a list into a single symbol"
       "\nsymbol2list\t:: split a symbol into a list"
       "\nstrcmp\t\t:: compare 2 lists as if they where strings"
       "\nrepack\t\t:: (re)packs atoms to packages of a given size"
       "\npackel\t\t:: element of a package"
       "\nlength\t\t:: length of a package"
       "\nniagara\t\t:: divide a package into 2 sub-packages"
       "\nglue\t\t:: append a list to another"
       "\nrepeat\t\t:: repeat a message"
       "\nsegregate\t:: sort inputs by type"
       "\n.\t\t:: scalar multiplication of vectors (lists of floats)"

       "\n%c advanced message objects"

       "\ntabread4\t:: 4-point interpolating table-read object"
       "\ntabdump\t\t:: dump the table as a list"
       "\ntabset\t\t:: set a table with a list"
       "\nmavg\t\t:: a variable moving average filter"
       "\nmean\t\t:: get the arithmetic mean of a vector"
       "\nminmax\t\t:: get the minimum and the maximum of a vector"
       "\nmakesymbol\t:: creates (formatted) symbols"
       "\ndate\t\t:: get the current system date"
       "\ntime\t\t:: get the current system time"
       "\nindex\t\t:: convert symbols to indices"
       "\ndrip\t\t:: converts a package to a sequence of atoms"
       "\nsort\t\t:: shell-sort a package of floats"
       "\ndemux\t\t:: demultiplex the input to a specified output"
       "\nmsgfile\t\t:: store and handles lists of lists"
       "\nlp\t\t:: write to the (parallel) port"
       "\nwrap\t\t:: wrap a floating number between 2 limits"
       "\nurn\t\t:: unique random numbers"
#if 0
       "\nexecute\t\t:: execute an application"
#endif

       "\n%c matrix message objects"

       "\nmatrix\t\t:: create/store/edit matrices"

       "\nmtx_element\t:: change elements of a matrix"
       "\nmtx_row\t\t:: change rows of a matrix"
       "\nmtx_col\t\t:: change columns of a matrix"
       "\nmtx_eye\t\t:: create an identity matrix"
       "\nmtx_egg\t\t:: create an identity matrix that is rotated by 90deg"
       "\nmtx_zeros\t:: create a matrix whose elements are all 1"
       "\nmtx_ones\t:: create a matrix whose elements are all 0"
       "\nmtx_diag\t:: returns the diagonal of a matrix or create a diagonal matrix"
       "\nmtx_dieggt:: like mtx_diag but rotated by 90deg"
       "\nmtx_trace\t:: calculate the trace of a matrix"
       "\nmtx_size\t:: returns the size of a matrix"
       "\nmtx_resize\t:: resize a matrix (evt. with zero-padding)"
       "\nmtx_transpose\t:: transpose a matrix"
       "\nmtx_scroll\t:: shift the rows of a matrix"
       "\nmtx_roll\t:: shift the columns of a matrix"
       "\nmtx_pivot\t:: pivot-transform a matrix"
       "\nmtx_add\t\t:: add matrices"
       "\nmtx_mul\t\t:: multiply matrices"
       "\nmtx_.*\t\t:: multiply matrices element by element"
       "\nmtx_./\t\t:: divide matrices element by element"
       "\nmtx_inverse\t:: calculate the inverse of a matrix"
       "\nmtx_mean\t:: return the mean value of each column"
       "\nmtx_rand\t:: fill a matrix with random values"
       "\nmtx_check\t:: check the consistency of a matrix and correct if necessary"
       "\nmtx_print\t:: print formatted matrix to the stdout (for debug)"

       "\n\n(l) forum::für::umläute except where indicated (%c)\n"
       "this software is under the GnuGPL that is provided with these files", HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL);
}

void *zexy_new(void)
{
  t_zexy *x = (t_zexy *)pd_new(zexy_class);
  return (void *)x;
}

/* include some externals */
#if 0
void z_streamin_setup();	/* urps, i THINK this will be linux only */
void z_streamout_setup();
void z_stdinout_setup();	// not yet...
#endif // 0
void z_sfplay_setup();
void z_sfrecord_setup();
void z_noise_setup();
void z_testfun_setup();
void z_nop_setup();
void z_zdelay_setup();
void z_limiter_setup();
void z_swap_setup();
void z_quantize_setup();
void z_sigzero_setup();
void z_tabread4_setup();
void z_makefilenamen_setup();
void z_makesymbol_setup();

void z_pdf_setup();
void z_dfreq_setup();
void z_sigaverage_setup();
void z_sigpack_setup();

void z_datetime_setup();

void z_sigbin_setup();

#if 0  // used to be Win32 only, but i somehow lost the fine code
void z_execute_setup();
#endif

/* lp ports are only on i386 machines  */
#ifdef __i386__
void z_lp_setup();
#endif

void z_index_setup();
void z_connective_setup();
void z_sort_setup();
void z_multiplex_setup();
void z_average_setup();
void z_coordinates_setup();
void z_stat_setup();

void z_pack_setup();
void z_drip_setup();

void z_stdinout_setup();
void z_msgfile_setup();
void z_multiline_setup();
void z_matrix_setup();
void z_sigmatrix_setup();

void z_strings_setup();

void z_prime_setup();
void z_random_setup();
void z_wrap_setup();
/*
  waiting to be released in near future:
  make stdin~ and stdout~ work
  MAKE streamin~ work !!!
  sql
  ...
*/

void zexy_setup(void) 
{
  int i;
#if 0
#ifdef linux
  z_streamin_setup();
#endif
  z_streamout_setup();
  z_stdinout_setup();
#endif
  z_sfplay_setup();
  z_sfrecord_setup();
  z_noise_setup();
  z_testfun_setup();
  z_limiter_setup();
  z_nop_setup();
  z_zdelay_setup();
  z_swap_setup();
  z_quantize_setup();

  z_sigzero_setup();
  z_pdf_setup();
  z_dfreq_setup();
  z_sigaverage_setup();
  z_sigbin_setup();

  z_sigpack_setup();

  z_tabread4_setup();
  z_average_setup();
  z_coordinates_setup();
  z_stat_setup();
  z_makesymbol_setup();

  z_datetime_setup();

  z_index_setup();
  z_connective_setup();
  z_sort_setup();
  z_multiplex_setup();
  z_pack_setup();
  z_drip_setup();

  z_prime_setup();
  z_random_setup();
  z_wrap_setup();
#if 0
  z_stdinout_setup();

  // we'll do this the next days
  z_execute_setup();
#endif
  z_msgfile_setup();

  z_multiline_setup();
  z_matrix_setup();
  z_sigmatrix_setup();

  z_strings_setup();

/* lp ports are only on i386 machines  */
#ifdef __i386__
  z_lp_setup();
#endif

  /* ************************************** */
  startpost("\n\t");
  for (i=0; i<28; i++) startpost("%c", HEARTSYMBOL);
  endpost();
  post("\t%c  the zexy external  "VERSION"  %c", HEARTSYMBOL, HEARTSYMBOL);
  post("\t%c (l)  forum::für::umläute %c", HEARTSYMBOL, HEARTSYMBOL);
  post("\t%c  compiled:  "__DATE__"  %c", HEARTSYMBOL, HEARTSYMBOL);
  post("\t%c send me a 'help' message %c", HEARTSYMBOL, HEARTSYMBOL);
  startpost("\t");
  for (i=0; i<28; i++) startpost("%c", HEARTSYMBOL);
  endpost(); endpost();
  
  zexy_class = class_new(gensym("zexy"), zexy_new, 0, sizeof(t_zexy), 0, 0);
  class_addmethod(zexy_class, zexy_help, gensym("help"), 0);
}
