#ifndef _MCDB_H_
#define _MCDB_H_

typedef struct mcdb_Connection mcdb_Connection;

typedef struct mcdb_ResultImplementation mcdb_ResultImplementation;

typedef struct mcdb_Result {
  const unsigned char *a;
  const unsigned char *b;
  const unsigned char *uuid;

  mcdb_ResultImplementation *imp;
} mcdb_Result;

int  mcdb_init      ( mcdb_Connection **conn );
void mcdb_shutdown  ( mcdb_Connection *conn );
int  mcdb_add       ( mcdb_Connection *conn, const char *a, const char *b );
/* call freeResult to clean up result */
void mcdb_find      ( mcdb_Connection *conn, const char *text, mcdb_Result **result);
int  mcdb_next      ( mcdb_Connection *conn, mcdb_Result *result );
void mcdb_freeResult( mcdb_Result *result );

#endif
