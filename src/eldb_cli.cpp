//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#include <cstdio>
#include <cstring>

#include "eldb.hpp"

using namespace eldb;

void find( const Eldb &eldb, const char *text ) {
  auto result = eldb.find( text );

  if ( result->count == 0 ) {
    fprintf( stderr, "No Results\n" );
    return;
  }

  for ( auto key: result->keys ) {
    fprintf( stdout, "%s :\n", key->key.c_str() );
    
    for ( auto value: key->values ) {
      fprintf( stdout, "\t%s\n", value->value.c_str() );
    }
  }
}


int main( int argc, char **argv ) {
  Eldb eldb;

  if ( ! eldb.init( "" ) ) {
    return 1;
  }

  if ( argc > 1 ) {
    if ( strcmp( argv[1], "add" ) == 0 ) {
      if ( argc == 4 ) {
        eldb.insert( argv[2], argv[3] );
      }
    }
    if ( strcmp( argv[1], "find" ) == 0 ) {
      if ( argc == 3 ) {
        find( eldb, argv[2] );
      }
    }
  }

  return 0;
}
