#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#include "mcdb.h"


void find( mcdb_Connection *conn, const char *text ) {
  mcdb_Result *result;

  mcdb_find( conn, text, &result );


  if( result ) {
    while( !mcdb_next( conn, result ) ) {
      fprintf( stderr, "%s : %s\n", result->a, result->b );
    }
  } else {
    fprintf( stderr, "No Results\n" );
  }

  mcdb_freeResult(result);
}


int main( int argc, char **argv ) {
  mcdb_Connection *conn;

  if( ! mcdb_init( &conn ) ) {
    return 1;
  }

  if( argc > 1 ) {
    if( strcmp( argv[1], "add" ) == 0) {
      if( argc == 4 ) {
        mcdb_add( conn, argv[2], argv[3] );
      }
    }
    if( strcmp( argv[1], "find" ) == 0) {
      if( argc == 3 ) {
        find( conn, argv[2] );
      }
    }
  }

  mcdb_shutdown( conn );

  return 0;
}
