#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sqlite3.h>
#include <glib.h>
#include <uuid/uuid.h>
#include <sys/stat.h>

#include "mcdb.h"
#include "mcstd.h"


struct mcdb_Connection {
  sqlite3      *db;
  sqlite3_stmt *addPStmt;

};


struct mcdb_ResultImplementation {
  sqlite3_stmt *stmt;
};


static int createTables( mcdb_Connection *conn );
static int tableExists( mcdb_Connection *conn, const char *tableName );
static int prepare( mcdb_Connection *conn, const char *query, sqlite3_stmt **statement );


int mcdb_init( mcdb_Connection **conn ) {
  char *dbDir;
  char *dbFile;

  fprintf( stderr, "initializing database\n" );

  *conn = ( mcdb_Connection* )malloc( sizeof( mcdb_Connection ) );

  dbDir = g_strconcat( g_get_user_data_dir(), "/metacortex", NULL );
  if ( !g_file_test ( dbDir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR ) ) {
    if( -1 == g_mkdir_with_parents( dbDir, S_IRUSR | S_IWUSR | S_IXUSR ) ) {
      fprintf( stderr, "error: could not create data folder %s\n", dbDir );
      g_free( dbDir );
      return 0;
    }
  }
  dbFile = g_build_filename( dbDir, "db", NULL );
  g_free( dbDir );

  int dbNotOpened = sqlite3_open( dbFile, &( ( *conn )->db ) );
  g_free( dbFile );

  if( dbNotOpened ) {
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( ( *conn )->db ) );
    mcdb_shutdown( *conn );
    return 0;
  }

  if( ! tableExists( *conn, "pair" ) ) {
    if ( ! createTables( *conn ) ) {
      mcdb_shutdown( *conn );
      return 0;
    }
  }

  if( prepare( *conn,
               "INSERT INTO pair ( uuid, a, b, creation_date, modification_date ) "
               "VALUES( ?, ?, ?, datetime( 'now' ), datetime( 'now') );",
               &( *conn )->addPStmt ) ) {
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( ( *conn )->db ) );
    mcdb_shutdown( *conn );
    return 0;
  }

  return 1;
}


void mcdb_shutdown( mcdb_Connection *conn ) {
  fprintf( stderr, "shutting down database\n" );
  if( conn->addPStmt ) {
    sqlite3_finalize( conn->addPStmt );
  }
  sqlite3_close( conn->db );
}


int mcdb_add( mcdb_Connection *conn, const char *a, const char *b ) {
  int step;
  int err;
  char uuidStr[36];

  uuid_t uuid;
  uuid_generate_random( uuid );
  uuid_unparse( uuid, uuidStr );

  err             = sqlite3_bind_text( conn->addPStmt, 1, uuidStr, -1, NULL );
  if( !err ) err  = sqlite3_bind_text( conn->addPStmt, 2, a, -1, NULL );
  if( !err ) err  = sqlite3_bind_text( conn->addPStmt, 3, b, -1, NULL );
  if( !err ) step = sqlite3_step( conn->addPStmt );

  if( err || SQLITE_DONE != step ) {
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn->db ) );
    return 0;
  }

  fprintf( stderr, "added %s\n", a );
  return 1;
}


void mcdb_find( mcdb_Connection *conn, const char *text, mcdb_Result **result ) {
  int err;
  sqlite3_stmt *stmt;
  char *param;

  err = prepare( conn, "SELECT uuid, a, b FROM pair WHERE a LIKE ?;", &stmt );

  malloc_sprintf( &param, "%%%s%%", text );
  if( !err ) sqlite3_bind_text( stmt, 1, param, -1, free );

  if( err ) {
    sqlite3_finalize( stmt);
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn->db ) );
    *result = NULL;
    return;
  }

  *result = malloc( sizeof( mcdb_Result ) );
  (*result)->imp = malloc( sizeof( mcdb_ResultImplementation ) );
  (*result)->imp->stmt = stmt;
}


void mcdb_freeResult( mcdb_Result *result ) {
  if( result ) {
    sqlite3_finalize( result->imp->stmt );
    free( result->imp );
    free( result );
  }
}


int mcdb_next( mcdb_Connection *conn, mcdb_Result *result ) {
  int step;

  if( !result )
    return 1;

  step =  sqlite3_step( result->imp->stmt );
  if( step == SQLITE_ROW ) {
    result->uuid = sqlite3_column_text( result->imp->stmt, 0 );
    result->a    = sqlite3_column_text( result->imp->stmt, 1 );
    result->b    = sqlite3_column_text( result->imp->stmt, 2 );
    return 0;
  }

  if( step == SQLITE_DONE ) {
    return 1;
  }

  fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn->db ) );

  return 2;
}


#define CREATE_TABLES "CREATE TABLE pair (uuid, a, b, creation_date, modification_date );"

static int createTables( mcdb_Connection *conn )  {
  sqlite3_stmt *statement;
  int err, step;

  fprintf( stderr, "creating tables\n" );
  err = prepare( conn, CREATE_TABLES, &statement );
  if( !err ) step = sqlite3_step( statement );

  sqlite3_finalize( statement );

  if( err || step != SQLITE_DONE ) {
    fprintf( stderr, "error: %s\n", sqlite3_errmsg( conn->db ) );
    return 0;
  }

  return 1;
}


static int tableExists( mcdb_Connection *conn, const char *tableName ) {
  sqlite3_stmt *ppStmt;
  char *query;

  int queryLength = malloc_sprintf( &query, "SELECT 1 FROM %s WHERE 1 = 0", tableName );
  int err = sqlite3_prepare_v2( conn->db, query, queryLength, &ppStmt, 0 );

  sqlite3_finalize( ppStmt );
  free( query );
   
  if( err ) {
    return 0;
  }

  return 1;
}


static int prepare( mcdb_Connection *conn, const char *query, sqlite3_stmt **statement ) {
  return sqlite3_prepare_v2( conn->db, query, -1, statement, 0 );
}
