/*
 * Copyright (C) 1996-8 Michael R. Elkins <me@cs.hmc.edu>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software Foundation,
 *     Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,, USA.
 */ 

/* $Id: rfc2047.h,v 1.4 2007-10-28 16:33:36 roland Exp $ */

#define HAVE_ICONV 1

#if defined(_WIN32)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#include "iconv.h"

extern const char *Charset;
void rfc2047_encode_string (char *, size_t, const unsigned char *);
void rfc2047_encode_adrlist (ADDRESS *);

void rfc2047_decode (char *, const char *, size_t);
void rfc2047_decode_adrlist (ADDRESS *);
