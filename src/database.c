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
    result = sqlite3_exec(retval->s_db, DATABASE_SCHEMA, NULL, NULL, &errmsg);
    if (result != SQLITE_OK) {
      fprintf(stderr, "Couldn't create schema: %s\n", errmsg);
      sqlite3_free(errmsg);
      database_close(retval);
      return(NULL);
    }
  }

  return(retval);
}

void
database_close(p_db)
  p_database *p_db;
{
  if (p_db->s_db != NULL)
    sqlite3_close(p_db->s_db);
  free(p_db);
}

char *
database_escape(str)
  const char *str;
{
  char *retval = NULL, *retval_tail, *str_head, *ptr;
  int i, count, len, diff;

  len = strlen(str);
  count = 0;
  for (i = 0; i < len; i++) {
    if (str[i] == '\'')
      count++;
  }

  if (count == 0) {
    retval = (char *)malloc(sizeof(char) * len);
    strcpy(retval, str);
  }
  else {
    retval = (char *)malloc(sizeof(char) * (len + count));
    str_head = str;
    retval_tail = retval;
    while ((ptr = index(str_head, '\'')) != NULL) {
      diff = ptr - str_head;
      if (diff > 0) {
        strncpy(retval_tail, str_head, diff);
        retval_tail += diff;
      }
      strncpy(retval_tail, "''", 2);
      retval_tail += 2;
      str_head = ptr + 1;
    }
    strcpy(retval_tail, str_head);
  }

  return retval;
}
