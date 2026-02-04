/*
 * z_strnlen: strnlen() replacement
 *
 * (c) 2026 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
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
#ifndef INCLUDE_Z_STRNLEN_H__
#define INCLUDE_Z_STRNLEN_H__
#include <stddef.h>
static size_t z_strnlen(const char*s, size_t maxlen) {
    size_t i;
    for(i=0; i<maxlen && *s++; i++);
    return i;
}
#endif /* INCLUDE_Z_STRNLEN_H__ */
