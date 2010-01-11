#ifndef _DATABASE_H
#define _DATABASE_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sqlite3.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DATABASE_SCHEMA "\
  CREATE TABLE songs(\
    id INTEGER PRIMARY KEY,\
    track INTEGER,\
    disc INTEGER,\
    title TEXT,\
    artist TEXT,\
    album TEXT,\
    seconds INTEGER\
  );\
"

typedef struct {
  sqlite3 *s_db;
} p_database;

typedef enum {
  P_INTEGER,
  P_TEXT
} p_result_value_type;

typedef struct {
  p_result_value_type type;
  union {
    int integer;
    unsigned char *text;
  } value;
} p_result_value;

typedef struct {
  int count;
  p_result_value *values;
} p_result_row;

typedef struct {
  int count;
  p_result_row *rows;
} p_result_set;

p_database *database_open();
void database_close(p_database *);
char *database_escape(const char *);
int database_exec(p_database *, const char *);
p_result_set *database_select(p_database *, const char *);
void database_free_result_set(p_result_set *);

#endif
