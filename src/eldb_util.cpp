//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>


int new_sprintf( char **str, const char * format, ... ) {
  va_list ap,apCopy;
  int length;

  va_start( ap, format );
    length = vsnprintf( NULL, 0, format, ap );
    *str = new char[length + 1];
    va_copy( apCopy, ap );
      vsnprintf( *str, length + 1, format, ap );
    va_end( apCopy );
  va_end( ap );
  return length;
}

const char *new_strcpy( const char *str ) {
  int length = strlen( str );

  char *ret = new char[length + 1];
  strcpy( ret, str );
  return ret;
}
