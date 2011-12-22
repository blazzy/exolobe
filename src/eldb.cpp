//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <glib.h>
#include <sys/stat.h>

#include "eldb.hpp"
#include "eldb_sqlite.hpp"
#include "eldb_util.hpp"


namespace eldb {


Eldb::Eldb(): db( NULL ) {
}


Eldb::~Eldb() {
  if ( db )
    delete db;
}


int Eldb::init( const char *uri ) {
  char *dbDir;
  char *dbFile;
  SQLiteDatabase *sqlite;

  dbDir = g_strconcat( g_get_user_data_dir(), "/exolobe", (void *)0 );
  if ( !g_file_test( dbDir, (GFileTest)( G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR )) ) {
    if ( -1 == g_mkdir_with_parents( dbDir, S_IRUSR | S_IWUSR | S_IXUSR ) ) {
      fprintf( stderr, "error: could not create data folder %s\n", dbDir );
      g_free( dbDir );
      return 0;
    }
  }

  dbFile = g_build_filename( dbDir, "db", (void*)0 );
  g_free( dbDir );

  db = sqlite = new SQLiteDatabase ();
  int dbInitialized = sqlite->init( dbFile );

  g_free( dbFile );
  return dbInitialized;
}
  

void FindResult::addResult( const char *key, const char *value, const char *uuid ) {

  auto keyR = Ptr<Key>( new FindResult::Key( key ) );
  keys.push_back( keyR );

  auto valueR = Ptr<Key::Value>( new Key::Value( key, value, uuid ) );

  keyR->values.push_back( valueR );

  ++count;
}


Ptr<FindResult> Eldb::find( const char *text ) const {
  auto f = Ptr<FindResult>( new FindResult() );

  auto dbResult = db->findExactKey( text );
  if ( dbResult.get() ) {
    while ( dbResult->next() ) {
      f->addResult( dbResult->key(), dbResult->values(), dbResult->uuid() );
    }
  }
  //dbResult = db->findExactTag( text );
  //dbResult = db->findValue( text );

  return f;
}


void Eldb::insert( const char *a, const char *b ) {
  db->insert( a, b );
}


} //namespace eldb
