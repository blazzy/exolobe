//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#ifndef EL_ELDB_HPP
#define EL_ELDB_HPP

#include <list>
#include <string>

#include "eldb_ptr.hpp"

using std::list;
using std::string;

namespace eldb {

struct Database;
struct FindResult;


struct Eldb {
  Eldb();
  virtual ~Eldb();

  int                    init   ( const char *uri );
  void                   insert ( const char *a, const char *b );
  int                    modify ( const char *uuid, const char *a, const char *b );
  Ptr<FindResult>        find   ( const char *text ) const;

  private:
    Database *db;
};


struct FindResult: Object {

  struct Key: Object {

    struct Value: Object {
      string key;
      string value;
      string uuid;

      Value( const char *key_, const char *value_, const char *uuid_ ):
        key( key_ ), value( value_ ), uuid( uuid_ ) {}
    };

    string key;
    list<Ptr<Value>> values;
    Key( const char *key_ ): key( key_ ) {}
  };

  list<Ptr<Key>> keys;
  int count;

  void addResult( const char *key, const char *value, const char *uuid );
};


}//namespace eldb

#endif //EL_ELDB_HPP
