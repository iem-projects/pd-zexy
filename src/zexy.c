/******************************************************
 *
 * zexy - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   1999:forum::für::umläute:2005
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


/* ...this is a very ZEXY external ...
   so have fun
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
       "\noperating_system\t:: information on the OS"

       "\n\n(l) forum::für::umläute except where indicated (%c)\n"
       "this software is under the GnuGPL that is provided with these files", HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL, HEARTSYMBOL);
}

static void *zexy_new(void)
{
  t_zexy *x = (t_zexy *)pd_new(zexy_class);
  return (x);
}

void z_zexy_setup(void); /* defined in z_zexy.c */

void zexy_setup(void) 
{
  int i;
  z_zexy_setup();
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

  zexy_register("zexy");
}
