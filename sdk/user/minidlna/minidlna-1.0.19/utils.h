/* Utility functions
 *
 * Project : minidlna
 * Website : http://sourceforge.net/projects/minidlna/
 * Author  : Justin Maggard
 *
 * MiniDLNA media server
 * Copyright (C) 2008-2009  Justin Maggard
 *
 * This file is part of MiniDLNA.
 *
 * MiniDLNA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * MiniDLNA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MiniDLNA. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __UTILS_H__
#define __UTILS_H__

int
ends_with(const char * haystack, const char * needle);

char *
trim(char *str);

char *
strstrc(const char *s, const char *p, const char t);

char *
modifyString(char * string, const char * before, const char * after, short like);

char *
escape_tag(const char *tag);

void
strip_ext(char * name);

int
make_dir(char * path, mode_t mode);

int
is_video(const char * file);

int
is_audio(const char * file);

int
is_image(const char * file);

int
is_playlist(const char * file);

int
is_album_art(const char * name);

int
resolve_unknown_type(const char * path, enum media_types dir_type);

#endif
