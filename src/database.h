#ifndef _DATABASE_H
#define _DATABASE_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sqlite3.h>
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

p_database *database_open();
void database_close(p_database *);
char *database_escape(const char *);

#endif
