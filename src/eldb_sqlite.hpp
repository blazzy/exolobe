//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#ifndef EL_SQLITE_HPP
#define EL_SQLITE_HPP

#include <sqlite3.h>

#include "eldb_ptr.hpp"

namespace eldb {


struct DatabaseSearchResult: Object {
  virtual int next()            = 0;
  virtual const char *uuid()    = 0;
  virtual const char *key()     = 0;
  virtual const char *values()  = 0;
  virtual const char *tag()     = 0;
  virtual ~DatabaseSearchResult() {}
};


struct Database {
  virtual Ptr<DatabaseSearchResult> findExactKey( const char * ) = 0;
  virtual Ptr<DatabaseSearchResult> findExactTag( const char * ) = 0;
  virtual Ptr<DatabaseSearchResult> findValue( const char * )    = 0;
  virtual int insert( const char *key, const char *val )         = 0;
  virtual ~Database() {}
};


struct SQLiteDatabase: Database {
  SQLiteDatabase();
  ~SQLiteDatabase();
  int init ( const char *file );

  Ptr<DatabaseSearchResult> findExactKey( const char * );
  Ptr<DatabaseSearchResult> findExactTag( const char * );
  Ptr<DatabaseSearchResult> findValue( const char * );
  int insert( const char *key, const char *val );

  private:
    sqlite3      *conn;
    sqlite3_stmt *addPStmt;
    int tableExists( const char *tableName );
    int createTables( );
    int prepare( const char *query, sqlite3_stmt **statement );
};


} //namespace eldb

#endif //EL_SQLITE_HPP
