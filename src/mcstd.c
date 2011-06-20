#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

int malloc_sprintf( char **str, char * format, ... ) {
  va_list ap,apCopy;
  int length;

  va_start( ap, format );
    length = vsnprintf( NULL, 0, format, ap );
    *str = (char *) malloc( length + 1);
    va_copy( apCopy, ap );
      vsnprintf( *str, length + 1, format, ap );
    va_end( apCopy );
  va_end( ap );
  return length;
}
