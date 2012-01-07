//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#include <cstdio>
#include <cstdlib>

extern "C" {
#include <token.h>
}

#include "eldb_sqlite.hpp"
#include "eldb_util.hpp"


namespace eldb {

#define CREATE_TABLES "CREATE TABLE pair ( uuid, a, b, creation_date, modification_date );"

SQLiteDatabase::SQLiteDatabase(): conn( NULL ), addPStmt( NULL ) {
}


SQLiteDatabase::~SQLiteDatabase() {
  fprintf( stderr, "shutting down database\n" );
  if ( conn ) {
    if ( addPStmt )
      sqlite3_finalize( addPStmt );
    sqlite3_close( conn );
  }
}


int SQLiteDatabase::init( const char *file ) {
  fprintf( stderr, "initializing database\n" );

  int dbNotOpened = sqlite3_open( file, &conn );

  if ( dbNotOpened ) {
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn ) );
    return 0;
  }

  if ( ! tableExists( "pair" ) ) {
    if ( ! createTables() )
      return 0;
  }

  if ( prepare( "INSERT INTO pair ( uuid, a, b, creation_date, modification_date ) "
               "VALUES( ?, ?, ?, datetime( 'now' ), datetime( 'now' ) );",
               &addPStmt ) ) {
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn ) );
    return 0;
  }

  return 1;
}


int SQLiteDatabase::tableExists( const char *tableName ) {
  sqlite3_stmt *ppStmt;
  char *query;

  int queryLength = malloc_sprintf( &query, "SELECT 1 FROM %s WHERE 1 = 0", tableName );
  int err = sqlite3_prepare_v2( conn, query, queryLength, &ppStmt, 0 );

  sqlite3_finalize( ppStmt );
  free( query );
   
  if ( err ) {
    return 0;
  }

  return 1;
}


int SQLiteDatabase::createTables()  {
  sqlite3_stmt *statement;
  int err, step;

  fprintf( stderr, "creating tables\n" );
  err = prepare( CREATE_TABLES, &statement );
  if ( !err ) step = sqlite3_step( statement );

  sqlite3_finalize( statement );

  if ( err || step != SQLITE_DONE ) {
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn ) );
    return 0;
  }

  return 1;
}


int SQLiteDatabase::prepare( const char *query, sqlite3_stmt **statement ) {
  return sqlite3_prepare_v2( conn, query, -1, statement, 0 );
}


struct SQLiteDatabaseSearchResult: DatabaseSearchResult {

  sqlite3_stmt *stmt;
  sqlite3      *conn;

  SQLiteDatabaseSearchResult( sqlite3_stmt *stmt_, sqlite3 *conn_ ):
    stmt( stmt_ ), conn( conn_ ) {}

  ~SQLiteDatabaseSearchResult() {
    sqlite3_finalize( stmt );
  }

  int next() {
    int step = sqlite3_step( stmt );

    if ( step == SQLITE_ROW ) {
      return 1;
    }
    if ( step == SQLITE_DONE ) {
      return 0;
    }

    fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn ) );

    return 0;
  }

  const char *uuid() {
    return (const char*)sqlite3_column_text( stmt, 0 );
  }

  const char *key() {
    return (const char*)sqlite3_column_text( stmt, 1 );
  }

  const char *values() {
    return (const char*)sqlite3_column_text( stmt, 2 );
  }

  const char *tag() {
    return 0;
  }
};


Ptr<DatabaseSearchResult> SQLiteDatabase::findExactKey( const char *key ) {
  int err;
  sqlite3_stmt *stmt;
  char *param;

  err = prepare( "SELECT uuid, a, b FROM pair WHERE a LIKE ?;", &stmt );

  malloc_sprintf( &param, "%%%s%%", key );
  if ( !err ) sqlite3_bind_text( stmt, 1, param, -1, free );

  if ( err ) {
    sqlite3_finalize( stmt );
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn ) );
    return NULL;
  }

  return new SQLiteDatabaseSearchResult( stmt, conn );
}


Ptr<DatabaseSearchResult> SQLiteDatabase::findExactTag( const char * ) {
  return NULL;
}


Ptr<DatabaseSearchResult> SQLiteDatabase::findValue( const char * ) {
  return NULL;
}


int SQLiteDatabase::insert( const char *key, const char *val ) {
  int step;
  int err;
  char uuidStr[37];

  uuid_state state;
  uuid_t uuid;
  create_uuid_state( &state );
  create_token( &state, &uuid );
  format_token( uuidStr, &uuid );

  err =  sqlite3_bind_text( addPStmt, 1, uuidStr, -1, NULL ) ||
         sqlite3_bind_text( addPStmt, 2, key, -1, NULL )     ||
         sqlite3_bind_text( addPStmt, 3, val, -1, NULL );

  if ( !err ) {
    step = sqlite3_step( addPStmt );
  }

  if ( err || SQLITE_DONE != step ) {
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn ) );
    return 0;
  }

  fprintf( stderr, "added %s\n", key );
  return 1;
}

} //namespace eldb
