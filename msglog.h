/*
    This file is part of telegram-purple

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA

    Copyright Matthias Jentsch 2014-2015
*/
#include <stdarg.h>

void debug(const char* format, ...);
void info(const char* format, ...);
void warning(const char* format, ...);
void failure(const char* format, ...);
void fatal(const char* format, ...);
const char *print_flags_update (unsigned update);
const char *print_flags_channel (unsigned flags);
const char *print_flags_peer (unsigned flags);
const char *print_flags_user (unsigned flags);
