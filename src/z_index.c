/* 
   (c) 2005:forum::für::umläute:2000

   "index" simulates an associative index :: that is : convert a symbol to an index

*/

#include "zexy.h"

#include <string.h>
#include <stdio.h>

#define MAXKEYLEN 16

/* ----------------------- index --------------------- */

static t_class *index_class;

typedef struct _index
{
  t_object x_obj;

  int entries, maxentries;
  int auto_mode; /* 1--add if key doesn't already exist; 0--do not add; */

  char **names;
} t_index;

static int find_last(char **names, int maxentries)
{  /* returns the index of the last entry (0..(maxentries-1)) */
  while (maxentries--) if (names[maxentries]) return maxentries;
  return 0;
}

static int find_item(const char *key, char **names, int maxentries)
{  /* returns index (0..[maxentries-1?]) on success; -1 if the item could not be found */
  int i=-1;
  int max = find_last(names, maxentries);
  
  while (++i<=max)
    if (names[i])
      if (!strcmp(key, names[i])) return i;
  
  return -1;
}

static int find_free(char **names, int maxentries)
{
  int i=0;

  while (i<maxentries) {
    if (!names[i]) return i;
    i++;
  }
  return -1;
}

static void index_float(t_index *x, t_float findex)
{
  int index = (int)findex;
  if ((index > 0) && (index <= x->maxentries) && (x->names[index-1])) post("index[%d] = %s", index, x->names[index-1]);
}

static void index_auto(t_index *x, t_float automod)
{
  x->auto_mode = !(!automod);
}


static void index_add(t_index *x, t_symbol *s)
{
  int newentry;
  int ok = 0;

  if (! (find_item(s->s_name, x->names, x->maxentries)+1) ) {
    if ( x->entries < x->maxentries ) {
      newentry=find_free(x->names, x->maxentries);
      if (newentry + 1) {
	char *buf = (char *)getbytes(sizeof(char) * MAXKEYLEN);
	x->entries++;
	strcpy(buf, s->s_name);
	x->names[newentry]=buf;

	ok=1;

	outlet_float(x->x_obj.ob_outlet, (t_float)newentry+1);

      } else post("index :: couldn't find any place for new entry");
    } else post("index :: max number of elements (%d) reached !", x->maxentries);
  } else post("index :: element already exists");

  if (!ok) outlet_float(x->x_obj.ob_outlet, -1.f);
}

static void index_delete(t_index *x, t_symbol *s)
{
  int element;
  t_float r = -1.f;

  if ( (element = find_item(s->s_name,x->names, x->maxentries))+1 ) {
    freebytes(x->names[element], sizeof(char) * MAXKEYLEN);
    x->names[element]=0;
    r = 0.f;
    x->entries--;
  } else post("index :: couldn't find element");

  outlet_float(x->x_obj.ob_outlet, r);
}

static void index_reset(t_index *x)
{
  int i=x->maxentries;

  while (i--)
    if (x->names[i]) {
      freebytes(x->names[i], sizeof(char) * MAXKEYLEN);
      x->names[i]=0;
    }

  x->entries=0;

  outlet_float(x->x_obj.ob_outlet, 0.f);
}

static void index_symbol(t_index *x, t_symbol *s)
{
  int element;
  if ( (element = find_item(s->s_name, x->names, x->maxentries)+1) )
    outlet_float(x->x_obj.ob_outlet, (t_float)element);
  else if (x->auto_mode)
    index_add(x, s);
  else outlet_float(x->x_obj.ob_outlet, 0.f);
}

static void *index_new(t_symbol *s, int argc, t_atom *argv)
{
  t_index *x = (t_index *)pd_new(index_class);
  char** buf;

  int maxentries = 0, automod=0;
  int i;

  if (argc--) {
    maxentries = (int)atom_getfloat(argv++);
    if (argc) automod = (int)atom_getfloat(argv++);
  }

  if (!maxentries) maxentries=128;

  buf = (char **)getbytes(sizeof(char *) * maxentries);

  i = maxentries;
  while (i--) buf[i]=0;

  x->entries = 0;
  x->maxentries = maxentries;
  x->names = buf;
  x->auto_mode = !(!automod);

  outlet_new(&x->x_obj, &s_float);

  return (x);
}

static void index_free(t_index *x)
{
  index_reset(x);
  freebytes(x->names, sizeof(char *) * x->maxentries);
}


static void helper(t_index *x)
{
  post("\n%c index :: index symbols to indices", HEARTSYMBOL);
  post("<symbol>\t : look up the <symbol> in the index and return it's index\n"
       "'add <symbol>'\t: add a new symbol to the index\n"
       "'delete <symbol>' : delete a symbol from the index\n"
       "'reset'\t\t : delete the whole index\n"
       "'auto <1/0>\t : if auto is 1 and a yet unknown symbol is looked up it is\n\t\t\t automatically added to the index\n"
       "'help'\t\t : view this"
       "\noutlet : <n>\t : index of the <symbol>");
  post("\ncreation:\"index [<maxelements> [<auto>]]\": creates a <maxelements> sized index");
}

void z_index_setup(void)
{
  index_class = class_new(gensym("index"),
			  (t_newmethod)index_new, (t_method)index_free,
			  sizeof(t_index), 0, A_GIMME, 0);

  class_addsymbol(index_class, index_symbol);

  class_addmethod(index_class, (t_method)index_reset,  gensym("reset"), 0);
  class_addmethod(index_class, (t_method)index_delete, gensym("delete"), A_SYMBOL, 0);
  class_addmethod(index_class, (t_method)index_add,	 gensym("add"), A_SYMBOL, 0);

  class_addmethod(index_class, (t_method)index_auto,	 gensym("auto"), A_FLOAT, 0);

  class_addfloat(index_class, (t_method)index_float);

  class_addmethod(index_class, (t_method)helper, gensym("help"), 0);
  class_sethelpsymbol(index_class, gensym("zexy/index"));
}
