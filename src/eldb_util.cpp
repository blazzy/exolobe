//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>


int malloc_sprintf( char **str, const char * format, ... ) {
  va_list ap,apCopy;
  int length;

  va_start( ap, format );
    va_copy( apCopy, ap );
      length = vsnprintf( NULL, 0, format, ap );
      *str = (char *) malloc( length + 1 );
      vsnprintf( *str, length + 1, format, apCopy );
    va_end( apCopy );
  va_end( ap );
  return length;
}


const char *malloc_strcpy( const char *str ) {
  int length = strlen( str );

  char *ret = (char *) malloc( length + 1 );
  strcpy( ret, str );
  return ret;
}
