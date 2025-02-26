/* MiniDLNA media server
 * Copyright (C) 2008-2010 NETGEAR, Inc. All Rights Reserved.
 *
 * This file is part of MiniDLNA.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ERR_H__
#define __ERR_H__

#define E_OFF		0
#define E_FATAL         1
#define E_ERROR		2
#define E_WARN          3
#define E_INFO          4
#define E_DEBUG		5

enum _log_facility
{
  L_GENERAL=0,
  L_ARTWORK,
  L_DB_SQL,
  L_INOTIFY,
  L_SCANNER,
  L_METADATA,
  L_HTTP,
  L_SSDP,
  L_TIVO,
  L_MAX
};

extern int log_level[L_MAX];
extern int log_init(const char *fname, const char *debug);
extern void log_err(int level, enum _log_facility facility, char *fname, int lineno, char *fmt, ...);

#define DPRINTF(level, facility, fmt, arg...) { log_err(level, facility, __FILE__, __LINE__, fmt, ##arg); }


#endif /* __ERR_H__ */
