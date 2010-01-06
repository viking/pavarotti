#include "database.h"

p_database *
database_open()
{
  char path[2048], *home, *errmsg;
  int exists, result;
  FILE *fp;
  p_database *retval;

  /* create pavarotti directory if it doesn't exist */
  home = getenv("HOME");
  if (home == NULL) {
    fprintf(stderr, "$HOME doesn't exist!\n");
    return(NULL);
  }
  sprintf(path, "%s/%s", home, ".pavarotti");

  if (chdir(path) != 0) {
    switch(errno) {
      case ENOENT:
        if (mkdir(path, 0755) == 0) {
          break;
        }
      default:
        fprintf(stderr, "Couldn't chdir to %s!\n", path);
        return(NULL);
    }
  }

  retval = (p_database *)malloc(sizeof(p_database));

  /* check to see if database exists yet */
  sprintf(path, "%s/database.sqlite3", path);
  exists = 0;
  if ((fp = fopen(path, "r")) != NULL) {
    exists = 1;
    fclose(fp);
  }

  if (sqlite3_open(path, &retval->s_db) != SQLITE_OK) {
    database_close(retval);
    fprintf(stderr, "Couldn't open database!\n");
    return(NULL);
  }

  if (exists == 0) {
    result = sqlite3_exec(retval->s_db, "CREATE TABLE songs(id INTEGER PRIMARY KEY);", NULL, NULL, &errmsg);
    if (result != SQLITE_OK) {
      fprintf(stderr, "Couldn't run query: %s\n", errmsg);
      sqlite3_free(errmsg);
      database_close(retval);
      return(NULL);
    }
  }

  return(retval);
}

void
database_close(database)
  p_database *database;
{
  if (database->s_db != NULL)
    sqlite3_close(database->s_db);
  free(database);
}
