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

/* 2305:forum::für::umläute:2001 */


#include "zexy.h"
#include <string.h>

/* ------------------------- fifop ------------------------------- */

/*
 * a FIFO (first-in first-out) with priorities
 *
 * an incoming list is added to a fifo (based on its priority)
 * "bang" outputs the next element of the non-empty fifo with the highest priority
 *
 * high priority means low numeric value
 */

static t_class *fifop_class;

typedef struct _fifop_list {
  int                 argc;
  t_atom             *argv;
  struct _fifop_list *next;
} t_fifop_list;

typedef struct _fifop_prioritylist {
  t_float                     priority;
  t_fifop_list               *fifo_start;
  t_fifop_list               *fifo_end;
  struct _fifop_prioritylist *next;
} t_fifop_prioritylist;

typedef struct _fifop
{
  t_object              x_obj;
  t_fifop_prioritylist *fifo_list;
  t_float               priority; /* current priority */
} t_fifop;

static t_fifop_prioritylist*fifop_genprioritylist(t_fifop*x, t_float priority)
{
  int i;
  t_fifop_prioritylist*result=0, *dummy=0;

  if(x->fifo_list!=0)
    {
      /*
       * do we already have this priority ?
       * if so, just return a pointer to that fifo
       * else set the dummy-pointer to the fifo BEFORE the new one
       */
      dummy=x->fifo_list;
      while(dummy!=0){
        t_float prio=dummy->priority;
        if(prio==priority)return dummy;
        if(prio>priority)break;
        result=dummy;
        dummy=dummy->next;
      }
      dummy=result; /* dummy points to the FIFO-before the one we want to insert */
    }
  /* create a new priority list */
  result = (t_fifop_prioritylist*)getbytes(sizeof( t_fifop_prioritylist));
  result->priority=priority;
  result->fifo_start=0;
  result->fifo_end=0;
  result->next=0;

  /* insert it into the list of priority lists */
  if(dummy==0){
    /* insert at the beginning */
    result->next=x->fifo_list;
    x->fifo_list=result;
  } else {
    /* post insert into the list of FIFOs */
    result->next=dummy->next;
    dummy->next =result;
  }

  /* return the result */
  return result;
}

static int add2fifo(t_fifop_prioritylist*fifoprio, int argc, t_atom *argv)
{
  t_atom*buffer=0;
  t_fifop_list*fifo=0;
  t_fifop_list*entry=0;

  if(fifoprio==0){
    error("pfifo: no fifos available");
    return -1;
  }

  /* create an entry for the fifo */
  if(!(entry = (t_fifop_list*)getbytes(sizeof(t_fifop_list))))
    {
      error("pfifo: couldn't add entry to end of fifo");
      return -1;
    }
  if(!(entry->argv=(t_atom*)getbytes(argc*sizeof(t_atom)))){
    error("pfifo: couldn't add list to fifo!");
    return -1;
  }
  memcpy(entry->argv, argv, argc*sizeof(t_atom));
  entry->argc=argc;
  entry->next=0;

  /* insert entry into fifo */
  if(fifoprio->fifo_end){
    /* append to the end of the fifo */
    fifo=fifoprio->fifo_end;

    /* add new entry to end of fifo */
    fifo->next=entry;
    fifoprio->fifo_end=entry;    
  } else {
    /* the new entry is the 1st entry of the fifo */
    fifoprio->fifo_start=entry;
    /* and at the same time, it is the last entry */
    fifoprio->fifo_end  =entry;
  }
  return 0;
}
static t_fifop_prioritylist*getFifo(t_fifop_prioritylist*pfifo)
{
  if(pfifo==0)return 0;
  /* get the highest non-empty fifo */
  while(pfifo->fifo_start==0 && pfifo->next!=0)pfifo=pfifo->next;
  return pfifo;
}

static void fifop_list(t_fifop *x, t_symbol *s, int argc, t_atom *argv)
{
  t_fifop_prioritylist*pfifo=0;
  if(!(pfifo=fifop_genprioritylist(x, x->priority))) {
    error("[fifop]: couldn't get priority fifo");
    return;
  }
  add2fifo(pfifo, argc, argv);
}
static void fifop_bang(t_fifop *x)
{
  t_fifop_prioritylist*pfifo=0;
  t_fifop_list*fifo=0;
  t_atom*argv=0;
  int argc=0;

  if(!(pfifo=getFifo(x->fifo_list))){
    return;
  }
  if(!(fifo=pfifo->fifo_start)){
    return;
  }

  pfifo->fifo_start=fifo->next;
  if(0==pfifo->fifo_start){
    pfifo->fifo_end=0;
  }
  /* get the list from the entry */
  argc=fifo->argc;
  argv=fifo->argv;

  fifo->argc=0;
  fifo->argv=0;
  fifo->next=0;

  /* destroy the fifo-entry (important for recursion! */
  freebytes(fifo, sizeof(t_fifop_list));

  /* output the list */
  outlet_list(x->x_obj.ob_outlet, &s_list, argc, argv);

  /* free the list */
  freebytes(argv, argc*sizeof(t_atom));
}

static void fifop_free(t_fifop *x)
{
  t_fifop_prioritylist *fifo_list=x->fifo_list;
  while(fifo_list){
    t_fifop_prioritylist *fifo_list2=fifo_list;

    t_fifop_list*fifo=fifo_list2->fifo_start;
    fifo_list=fifo_list->next;

    while(fifo){
      t_fifop_list*fifo2=fifo;
      fifo=fifo->next;

      if(fifo2->argv)freebytes(fifo2->argv, fifo2->argc*sizeof(t_atom));
      fifo2->argv=0;
      fifo2->argc=0;
      fifo2->next=0;
      freebytes(fifo2, sizeof(t_fifop_list));
    }
    fifo_list2->priority  =0;
    fifo_list2->fifo_start=0;
    fifo_list2->fifo_end  =0;
    fifo_list2->next      =0;
    freebytes(fifo_list2, sizeof( t_fifop_prioritylist));
  }
  x->fifo_list=0;
}

static void *fifop_new(t_symbol *s, int argc, t_atom *argv)
{
  t_fifop *x = (t_fifop *)pd_new(fifop_class);

  outlet_new(&x->x_obj, 0);
  floatinlet_new(&x->x_obj, &x->priority);

  x->fifo_list = 0;
  x->priority=0;

  return (x);
}

void fifop_setup(void)
{
  fifop_class = class_new(gensym("fifop"), (t_newmethod)fifop_new,
                             (t_method)fifop_free, sizeof(t_fifop), 0, A_GIMME, 0);

  class_addbang    (fifop_class, fifop_bang);
  class_addlist    (fifop_class, fifop_list);

  class_sethelpsymbol(fifop_class, gensym("zexy/fifop"));
  zexy_register("fifop");
}

void z_fifop_setup(void)
{
  fifop_setup();
}
