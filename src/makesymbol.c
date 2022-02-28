/*
 * makesymbol: yet another (formatted) symbol creation mechanism
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



/*
(l) 1210:forum::für::umläute:1999

"makesymbol" is something between "symbol" and "makefilename",
   thus storing and creating (formatted) symbols...


   TODO:
    the use of only "%s" is really not very satisfying
   LATER:
   split the entire format string into subformats with only
   one format-placeholder ("%[^%]*[diufFgGxXoscpn]") .
   split the entire format string into subformats with only one format-placeholder,
   store the types
   when a list comes in, evaluate each subformat (with prior type-conversion)

*/

#include "zexy.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ----------------------- makesymbol --------------------- */

static t_class *makesymbol_class=NULL;

typedef enum {
  NONE = 0,
  INT,
  FLOAT,
  STRING,
  POINTER,
} t_printtype;

typedef struct _formatspec {
  char*fs_format;
  t_printtype fs_accept;
  struct _formatspec*fs_next;
} t_formatspec;


static const char* _formatscan(const char*str, t_printtype*typ)
{
  int infmt=0;
  for (; *str; str++) {
    if (!infmt && *str=='%') {
      infmt=1;
      continue;
    }
    if (infmt) {
      if (*str=='%') {
        infmt=0;
        continue;
      }
      if (strchr("-.#0123456789",*str)!=0) {
        continue;
      }
      if (*str=='s') {
        *typ = STRING;
        return str+1;
      }
      if (strchr("fgGeE",*str)!=0) {
        *typ = FLOAT;
        return str+1;
      }
      if (strchr("xXdiouc",*str)!=0) {
        *typ = INT;
        return str+1;
      }
      if (strchr("p",*str)!=0) {
        *typ = POINTER;
        return str+1;
      }
    }
  }
  *typ = NONE;
  return str;
}
static void print_formatspecs(t_formatspec*fs)
{
  int i=0;
  while(fs) {
    post("[%d] '%s'[%d]", i, fs->fs_format, fs->fs_accept);
    i++;
    fs=fs->fs_next;
  }
}
static void delete_formatspecs(t_formatspec*fs)
{
  while(fs) {
    t_formatspec*ptr=fs;
    fs=fs->fs_next;
    free(ptr->fs_format);
    freebytes(ptr, sizeof(*ptr));
  }
}
static t_formatspec*parse_formatstring(const char*str)
{
  t_formatspec*result=NULL, *last=NULL;
  while(*str) {
    t_printtype typ=NONE;
    const char*nextstr = _formatscan(str, &typ);
    t_formatspec *fs = getbytes(sizeof(*fs));
    int len=nextstr - str;
    fs->fs_format = malloc(len+1);
    strncpy(fs->fs_format, str, len);
    fs->fs_format[len] = 0;
    fs->fs_accept = typ;
    if(!result) {
      result = fs;
    }
    if(last) {
      last->fs_next = fs;
    }
    last = fs;
    str = nextstr;
  }
  return result;
}

static char* format_float(char*buf, ssize_t buflen, t_formatspec*fs,
                          t_floatarg f)
{
  if(!fs->fs_format) {
    return NULL;
  }
  switch(fs->fs_accept) {
  case NONE:
    snprintf(buf, buflen, "%s",  fs->fs_format);
    break;
  case INT:
  case POINTER:
    snprintf(buf, buflen, fs->fs_format, (int)f);
    break;
  case FLOAT:
    snprintf(buf, buflen, fs->fs_format, f);
    break;
  case STRING: {
    char buf2[MAXPDSTRING];
    sprintf(buf2, "%g", f);
    snprintf(buf, buflen, fs->fs_format, buf2);
    break;
  }
  default:
    snprintf(buf, buflen, "%s", fs->fs_format);
  }
  return buf;
}

static char* format_symbol(char*buf, ssize_t buflen, t_formatspec*fs,
                           t_symbol *s)
{
  if(!fs->fs_format) {
    return NULL;
  }
  switch(fs->fs_accept) {
  case STRING:
  case POINTER:
    snprintf(buf, buflen, fs->fs_format, s->s_name);
    break;
  case INT:
    snprintf(buf, buflen, fs->fs_format, 0);
    break;
  case FLOAT:
    snprintf(buf, buflen, fs->fs_format, 0.);
    break;
  case NONE:
    snprintf(buf, buflen, "%s", fs->fs_format);
    break;
  default:
    snprintf(buf, buflen, "%s", fs->fs_format);
  }
  return buf;
}

static char* format_bang(char*buf, ssize_t buflen, t_formatspec*fs)
{
  if(!fs->fs_format) {
    return NULL;
  }
  switch(fs->fs_accept) {
  case INT:
    snprintf(buf, buflen, fs->fs_format, 0);
    break;
  case FLOAT:
    snprintf(buf, buflen, fs->fs_format, 0.);
    break;
  case NONE:
    snprintf(buf, buflen, "%s", fs->fs_format);
    break;
  default:
    snprintf(buf, buflen, "%s", fs->fs_format);
  }
  return buf;
}



typedef struct _makesymbol {
  t_object x_obj;
  t_symbol *x_sym;


  t_formatspec*x_fs;
  t_symbol *x_format;
} t_makesymbol;

static void reset_mask(t_makesymbol *x, t_symbol *s)
{
  delete_formatspecs(x->x_fs);
  x->x_fs = parse_formatstring(s->s_name);
  x->x_format = s;
  x->x_sym = s;
}

static t_symbol* list2symbol(t_formatspec*fs, int argc, t_atom *argv)
{
  char buf[MAXPDSTRING];
  int len = 0;
  buf[0] = 0;
  int i=0;
  while(fs && (len<MAXPDSTRING)) {
    t_atom*a = (i >= argc)?0:argv+i;
    t_atomtype typ = a?a->a_type:A_NULL;
    char*argbuf = buf+len;
    const char*b = NULL;
    switch(typ) {
    case A_SYMBOL:
      b=format_symbol(argbuf, MAXPDSTRING-len, fs, a->a_w.w_symbol);
      break;
    case A_FLOAT:
      b=format_float(argbuf, MAXPDSTRING-len, fs, a->a_w.w_float);
      break;
    default:
      b=format_bang(argbuf, MAXPDSTRING-len, fs);
      break;
    }
    if(!b) {
      return NULL;
    }
    len += strlen(argbuf);
    fs=fs->fs_next;
    i++;
  }
  return (gensym(buf));
}

static void makesymbol_list(t_makesymbol *x, t_symbol* UNUSED(s), int argc,
                            t_atom *argv)
{
  t_symbol*s = 0;
  if (x->x_fs) {
    s = list2symbol(x->x_fs, argc, argv);
  } else {
    size_t bufpos=0;
    char buf[MAXPDSTRING];
    memset(buf, 0, sizeof(buf));
    buf[0]=buf[1]=0;
    while(argc--) {
      char argbuf[MAXPDSTRING];
      size_t len;
      atom_string(argv++, argbuf+1, sizeof(argbuf)-1);
      argbuf[0] = ' ';
      argbuf[sizeof(argbuf)-1] = 0;

      len = strlen(argbuf);
      if((bufpos+len)>=sizeof(buf)) {
        len = sizeof(buf) - bufpos;
      }
      strncat(buf+bufpos, argbuf, sizeof(buf)-bufpos);
      bufpos+=len;
      if(bufpos>=sizeof(buf)) {
        break;
      }
    }
    //buf[sizeof(buf)-1] = 0;
    s = gensym(buf+1); /* +1 to drop the leading space */
  }
  if(!s) {
    pd_error(x, "illegal format specifier '%s'", x->x_format->s_name);
    return;
  }
  x->x_sym = s;
  outlet_symbol(x->x_obj.ob_outlet, x->x_sym);
}

static void makesymbol_bang(t_makesymbol *x)
{
  outlet_symbol(x->x_obj.ob_outlet, x->x_sym);
}


static void *makesymbol_new(t_symbol* UNUSED(s), int argc, t_atom *argv)
{
  t_makesymbol *x = (t_makesymbol *)pd_new(makesymbol_class);

  x->x_sym = gensym("");
  if (argc) {
    char buf[MAXPDSTRING];
    atom_string(argv, buf, MAXPDSTRING);
    buf[MAXPDSTRING-1] = 0;
    x->x_sym = gensym(buf);
    x->x_format = x->x_sym;
    x->x_fs = parse_formatstring(buf);
  }

  outlet_new(&x->x_obj, gensym("symbol"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym("sym1"));

  return (x);
}

static void makesymbol_free(t_makesymbol *x)
{
  delete_formatspecs(x->x_fs);
}


static void makesymbol_helper(t_makesymbol* UNUSED(x))
{
  post("\n"HEARTSYMBOL " makesymbol :: create a formatted symbol");
  post("<list of anything>\t: format list into a symbol\n"
       "'bang'\t\t\t: re-output\n"
       "'help'\t\t\t: view this"
       "\ninlet2 : <format-string>: new format-string (symbol !)"
       "\noutlet : <symbol>\t: formatted concatenation");
  post("\ncreation:\"makesymbol [<format-string>]\": C-style format-string (%s only)",
       "%s");
}

ZEXY_SETUP void makesymbol_setup(void)
{
  makesymbol_class = zexy_new("makesymbol",
                              makesymbol_new, makesymbol_free, t_makesymbol, CLASS_DEFAULT, "*");

  class_addlist(makesymbol_class, makesymbol_list);
  class_addbang(makesymbol_class, makesymbol_bang);

  zexy_addmethod(makesymbol_class, (t_method)reset_mask, "sym1", "s");

  zexy_addmethod(makesymbol_class, (t_method)makesymbol_helper, "help", "");
  zexy_register("makesymbol");
}
