/*
 * z_strnlen: strndup() replacement
 *
 * (c) 2018 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
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

#ifndef INCLUDE_Z_STRNDUP_H__
#define INCLUDE_Z_STRNDUP_H__

#include <stdlib.h>
#include <string.h>
static char *z_strndup(const char *s, size_t n)
{
  char *result = 0;
  size_t len = strlen(s) + 1;
  if (len > n) {
    len = n + 1;
  }

  result = malloc(len);
  if (!result) {
    return result;
  }
  memcpy(result, s, len);
  result[len - 1] = 0;
  return result;
}
#endif /* INCLUDE_Z_STRNDUP_H__ */
