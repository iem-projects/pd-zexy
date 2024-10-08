/*
 * freadln: reads messages continuously from the lines of a file
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 * (c) 2007 Franz Zotter <zotter@iem.at>, Institute of Electronic Music and Acoustics
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

#include "zexy.h"

#ifdef __WIN32__
#  define snprintf _snprintf
#endif

#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#ifdef __WIN32__
#  include <io.h>
#else
#  include <unistd.h>
#endif

#define MIN_FREADLN_LENGTH 10

/* freadln: reads messages continuously from the lines of
 * a file that doesn't necessarily need to fit
 * into the RAM of your system
 */

static t_class *freadln_class = NULL;

typedef struct freadln {
  t_object x_ob;
  FILE *x_file;
  char *x_filename;
  char *x_textbuf;
  size_t x_textbuf_length;
  t_outlet *x_message_outlet;
  t_outlet *x_readybang_outlet;

  char linebreak_chr[3];

  t_canvas *x_canvas;
} t_freadln;

static void freadln_close(t_freadln *x)
{
  if (x->x_file) {
    sys_fclose(x->x_file);
  }
  x->x_file = 0;
  if (x->x_filename) {
    freebytes(x->x_filename, sizeof(char) * MAXPDSTRING);
  }
  x->x_filename = 0;
  if (x->x_textbuf) {
    freebytes(x->x_textbuf, sizeof(char) * x->x_textbuf_length);
  }
  x->x_textbuf = 0;
  x->x_textbuf_length = 0;
}

static void freadln_open(t_freadln *x, t_symbol *s, t_symbol *type)
{
  char filenamebuf[MAXPDSTRING], *filenamebufptr;
  const char *dirname = canvas_getdir(x->x_canvas)->s_name;
  int fd, len;
  post("open: %s", s->s_name);

  freadln_close(x);

  /*
     if(type!=gensym("cr")) {
       pd_error(x, "currently only 'cr' type files are implemented!");
       return;
     }
  */
  if (type == gensym("cr")) {
    strcpy(x->linebreak_chr, "\n");
  } else {
    strcpy(x->linebreak_chr, ";\n");
  }

  /* directory, filename, extension, dirresult, nameresult, unsigned int size, int bin */
  if ((fd = open_via_path(dirname, s->s_name, "", filenamebuf, &filenamebufptr,
           MAXPDSTRING, 0)) < 0) {
    pd_error(x, "%s: failed to open %s", s->s_name, filenamebuf);
    return;
  }
  sys_close(fd);
  len = strlen(filenamebuf);
  if (!(x->x_filename =
              (char *)getbytes(sizeof(char) * (len + strlen(s->s_name) + 2)))) {
    pd_error(x, "out of memory");
    freadln_close(x);
    return;
  }
  strcpy(x->x_filename, filenamebuf);
  strcpy(x->x_filename + len, "/");
  strcpy(x->x_filename + len + 1, filenamebufptr);
  if (!(x->x_file = sys_fopen(x->x_filename, "r"))) {
    pd_error(x, "freadln: failed to fopen %s", x->x_filename);
    return;
  }
  if (!(x->x_textbuf = (char *)getbytes(MIN_FREADLN_LENGTH * sizeof(char)))) {
    pd_error(x, "out of memory!");
    freadln_close(x);
    return;
  }
  x->x_textbuf_length = MIN_FREADLN_LENGTH;
}

static size_t enlarge_cstr_if_required(
    const char **c_str, size_t *len, const size_t desired_min_length)
{
  size_t L;
  if ((!(*c_str)) || *len == 0) {
    *c_str = (char *)calloc(1, sizeof(char));
    return 1;
  }
  L = *len;
  if (L < desired_min_length) {
    do {
      L <<= 1;
    } while ((L < desired_min_length) && (L != 0));
    freebytes((char *)*c_str, sizeof(char) * L);
    if (!(*c_str = (char *)calloc(L, sizeof(char)))) {
      L = 0;
    }
  }
  *len = L;
  return L;
}

static int cstr_char_pos(const char *c_str, const char c)
{
  if (c_str) {
    int cnt = 1;
    do {
      if (*c_str == c) {
        return cnt;
      }
      cnt++;
    } while (*c_str++ != '\0');
  }
  return -1;
}

static void freadln_done(t_freadln *x)
{
  outlet_bang(x->x_readybang_outlet);
}

static void freadln_readline(t_freadln *x)
{
  int min_length = (x->x_textbuf_length < 1) ? 1 : x->x_textbuf_length;
  int linebreak_pos = 0;
  size_t items_read;
  t_binbuf *bbuf;
  t_atom *abuf;
  int abuf_length;

  if (!x->x_file) {
    pd_error(x, "no file opened for reading");
    freadln_done(x);
    return;
  }

  do {
    if (linebreak_pos == -1) {
      min_length <<= 1;
      fseek(x->x_file, -(long)(x->x_textbuf_length), SEEK_CUR);
    }
    if (!enlarge_cstr_if_required(
            (const char **)&x->x_textbuf, &x->x_textbuf_length, min_length)) {
      pd_error(x, "out of memory");
      x->x_textbuf_length = 0;
      freadln_close(x);
      freadln_done(x);
      return;
    }
    if (!(items_read = fread(
              x->x_textbuf, sizeof(char), x->x_textbuf_length, x->x_file))) {
      freadln_close(x);
      freadln_done(x);
      return;
    }
    x->x_textbuf[x->x_textbuf_length - 1] = 0;
  } while (((linebreak_pos =
                    cstr_char_pos(x->x_textbuf, x->linebreak_chr[0])) == -1) &&
           !(items_read < x->x_textbuf_length));

  if (linebreak_pos + strlen(x->linebreak_chr) < items_read + 1) {
    int rewind_after = items_read - linebreak_pos;
    fseek(x->x_file, -(long)(rewind_after), SEEK_CUR);
  }
  if (linebreak_pos == -1) {
    linebreak_pos = items_read;
  }
  x->x_textbuf[linebreak_pos - 1] = '\0';
  if (!(bbuf = binbuf_new())) {
    pd_error(x, "out of memory");
    freadln_close(x);
    freadln_done(x);
    return;
  }
  binbuf_text(bbuf, x->x_textbuf, linebreak_pos - 1);
  abuf = binbuf_getvec(bbuf);
  abuf_length = binbuf_getnatom(bbuf);
  if (abuf_length > 0) {
    if (abuf->a_type == A_SYMBOL) {
      outlet_anything(
          x->x_message_outlet, atom_getsymbol(abuf), abuf_length - 1, abuf + 1);
    } else {
      outlet_list(x->x_message_outlet, gensym("list"), abuf_length, abuf);
    }
  } else {
    outlet_list(x->x_message_outlet, atom_getsymbol(abuf), 0, abuf);
  }
  /* NOTE: the following line might be a problem in recursions
   * and could be performed before to outlet_* as well,
   * but(!) atom buffer abuf must be copied if doing so.
   */
  binbuf_free(bbuf);
}
static void freadln_free(t_freadln *x)
{
  freadln_close(x);
  outlet_free(x->x_message_outlet);
  outlet_free(x->x_readybang_outlet);
}

static void *freadln_new(void)
{
  t_freadln *x = (t_freadln *)pd_new(freadln_class);
  x->x_message_outlet = outlet_new(&x->x_ob, gensym("list"));
  x->x_readybang_outlet = outlet_new(&x->x_ob, gensym("bang"));
  x->x_filename = 0;
  x->x_file = 0;
  x->x_textbuf = 0;
  x->x_canvas = canvas_getcurrent();
  return (void *)x;
}

ZEXY_SETUP void freadln_setup(void)
{
  freadln_class = zexy_new(
      "freadln", freadln_new, freadln_free, t_freadln, CLASS_DEFAULT, "");
  zexy_addmethod(freadln_class, (t_method)freadln_open, "open", "sS");
  zexy_addmethod(freadln_class, (t_method)freadln_close, "close", "");
  class_addbang(freadln_class, (t_method)freadln_readline);

  zexy_register("freadln");
}
