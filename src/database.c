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
  const char *str_head;
  char *retval = NULL, *retval_tail, *ptr;
  int i, count, len, diff;

  len = strlen(str);
  count = 0;
  for (i = 0; i < len; i++) {
    if (str[i] == '\'')
      count++;
  }

  if (count == 0) {
    retval = (char *)malloc(sizeof(char) * (len + 1));
    strcpy(retval, str);
  }
  else {
    retval = (char *)malloc(sizeof(char) * (len + count + 1));
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

int
database_exec(p_db, sql)
  p_database *p_db;
  const char *sql;
{
  return sqlite3_exec(p_db->s_db, sql, NULL, NULL, NULL);
}

p_result_set *
database_select(p_db, sql)
  p_database *p_db;
  const char *sql;
{
  p_result_set *p_rs;
  p_result_value *p_rv;
  p_result_row *p_rr;
  sqlite3_stmt *s_stmt;
  int result, i, size, s_type;
  const unsigned char *s_text;

  sqlite3_prepare_v2(p_db->s_db, sql, -1, &s_stmt, NULL);
  result = sqlite3_step(s_stmt);
  if (result != SQLITE_ROW && result != SQLITE_DONE) {
    sqlite3_finalize(s_stmt);
    return NULL;
  }

  size = 100;
  p_rs = (p_result_set *)malloc(sizeof(p_result_set));
  p_rs->count = 0;
  p_rs->rows = (p_result_row *)malloc(sizeof(p_result_row) * size);

  while (result == SQLITE_ROW) {
    p_rr = p_rs->rows + p_rs->count;
    p_rr->count = sqlite3_column_count(s_stmt);
    p_rr->values = (p_result_value *)malloc(sizeof(p_result_value) * p_rr->count);

    for (i = 0; i < p_rr->count; i++) {
      p_rv = p_rr->values + i;

      s_type = sqlite3_column_type(s_stmt, i);
      switch(s_type) {
        case SQLITE_TEXT:
          s_text = sqlite3_column_text(s_stmt, i);
          p_rv->type = P_TEXT;
          p_rv->value.text = (unsigned char *)malloc(sizeof(unsigned char) * (strlen(s_text) + 1));
          strcpy(p_rv->value.text, s_text);
          break;
        case SQLITE_INTEGER:
          p_rv->type = P_INTEGER;
          p_rv->value.integer = sqlite3_column_int(s_stmt, i);
          break;
        default:
          fprintf(stderr, "Don't understand type: %d\n", s_type);
      }
    }

    p_rs->count++;
    if (p_rs->count == size) {
      size += 100;
      p_rs->rows = (p_result_row *)realloc(p_rs->rows, sizeof(p_result_row) * size);
    }
    result = sqlite3_step(s_stmt);
  }
  sqlite3_finalize(s_stmt);

  return p_rs;
}

void
database_free_result_set(p_rs)
  p_result_set *p_rs;
{
  p_result_row *p_rr;
  p_result_value *p_rv;
  int i, j;

  for (i = 0; i < p_rs->count; i++) {
    p_rr = p_rs->rows + i;
    for (j = 0; j < p_rr->count; j++) {
      p_rv = p_rr->values + j;
      if (p_rv->type == P_TEXT)
        free(p_rv->value.text);
    }
    free(p_rr->values);
  }
  free(p_rs->rows);
  free(p_rs);
}
