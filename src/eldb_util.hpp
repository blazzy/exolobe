//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#ifndef _MCSTD_H_
#define _MCSTD_H_

int malloc_sprintf( char **str, const char *format, ... );

const char *malloc_strcpy( const char *str );

void  gen_uuid( char uuidStr[37] );

#endif
