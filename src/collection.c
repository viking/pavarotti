#include "collection.h"

typedef struct {
  DIR *dp;
  char *path;
  void *prev;
} dirlist;

static char *
sanitize_str(str)
  char *str;
{
  char *retval, *tmp;
  if (str == NULL) {
    retval = (char *)malloc(sizeof(char) * 5);
    strcpy(retval, "NULL");
  }
  else {
    tmp = database_escape(str);
    retval = (char *)malloc(sizeof(char) * (strlen(tmp) + 3));
    sprintf(retval, "'%s'", tmp);
    free(tmp);
  }
  return retval;
}

static dirlist *
dirlist_new(path, prev)
  const char *path;
  dirlist *prev;
{
  DIR *dp;
  dirlist *dl;

  if ((dp = opendir(path)) == NULL)
    return NULL;

  dl = (dirlist *)malloc(sizeof(dirlist));
  dl->dp = dp;
  dl->path = (char *)malloc(sizeof(char) * (strlen(path) + 1));
  strcpy(dl->path, path);
  dl->prev = prev;

  return dl;
}

static void
dirlist_free(dl)
  dirlist *dl;
{
  closedir(dl->dp);
  free(dl->path);
  free(dl);
}

void
collection_discover(base_path)
  const char *base_path;
{
  int result;
  char sql[4192], path[2048], *d_filename, *d_title, *d_artist, *d_album;
  struct dirent *ep;

  dirlist *curr, *prev;
  p_song *p_s;
  p_database *p_db;

  p_db = database_open();
  curr = dirlist_new(base_path, NULL);
  if (curr == NULL)
    return;

  while(curr != NULL) {
    ep = readdir(curr->dp);
    if (ep == NULL) {
      prev = curr->prev;
      dirlist_free(curr);
      curr = prev;
      continue;
    }

    if (strcmp("..", ep->d_name) == 0 || strcmp(".", ep->d_name) == 0)
      continue;

    sprintf(path, "%s/%s", curr->path, ep->d_name);
    switch(ep->d_type) {
    case DT_DIR:
      prev = curr;
      curr = dirlist_new(path, prev);
      if (curr == NULL) {
        /* unable to open directory */
        curr = prev;
      }
      break;
    case DT_REG:
      p_s = song_read_file(path);
      if (p_s == NULL) {
        /* couldn't read song */
        continue;
      }

      d_filename = sanitize_str(p_s->filename);
      d_title    = sanitize_str(p_s->title);
      d_artist   = sanitize_str(p_s->artist);
      d_album    = sanitize_str(p_s->album);

      sprintf(sql, "INSERT INTO songs (filename, track, disc, title, artist, album) VALUES(%s, %d, %d, %s, %s, %s)", d_filename, p_s->track, p_s->disc, d_title, d_artist, d_album);
      result = database_exec(p_db, sql);

      free(d_filename);
      free(d_title);
      free(d_artist);
      free(d_album);

      song_free(p_s);
      break;
    }
  }
  database_close(p_db);
}

static int
get_count(p_db)
  p_database *p_db;
{
  p_result_set *p_rs;
  p_result_row p_rr;
  int count = -1;

  p_rs = database_select(p_db, "SELECT COUNT(*) FROM songs");
  if (p_rs != NULL) {
    if (p_rs->count == 1) {
      p_rr = p_rs->rows[0];
      if (p_rr.count == 1) {
        count = p_rr.values[0].value.integer;
      }
    }
    database_free_result_set(p_rs);
  }
  return count;
}

int
collection_count()
{
  p_database *p_db;
  int count = -1;

  p_db = database_open();
  if (p_db != NULL) {
    count = get_count(p_db);
    database_close(p_db);
  }
  return count;
}

static void
cp_result_str(dest, r_rv)
  char **dest;
  p_result_value *r_rv;
{
  *dest = NULL;
  if (r_rv->type == P_TEXT) {
    *dest = (char *)malloc(sizeof(char) * (strlen(r_rv->value.text) + 1));
    strcpy(*dest, r_rv->value.text);
  }
}


p_song *
collection_find(order_by)
  const char *order_by;
{
  int count, i;
  char *sql;
  p_database *p_db;
  p_result_set *p_rs;
  p_result_row *p_rr;
  p_song *songs, *p_s;

  p_db = database_open();
  if (p_db == NULL)
    return NULL;

  count = get_count(p_db);
  if (count < 0)
    return NULL;

  songs = (p_song *)malloc(sizeof(p_song) * count);

  const char *tmpl = "SELECT filename, track, disc, title, artist, album FROM songs ORDER BY %s";
  sql = (char *)malloc(sizeof(char) * (strlen(tmpl) + strlen(order_by) + 1));
  sprintf(sql, tmpl, order_by);
  p_rs = database_select(p_db, sql);
  if (p_rs != NULL) {
    for (i = 0; i < p_rs->count; i++) {
      p_s = songs + i;
      p_rr = p_rs->rows + i;
      cp_result_str(&p_s->filename, p_rr->values + 0);
      p_s->track = (p_rr->values[1].type == P_INTEGER) ? p_rr->values[1].value.integer : 0;
      p_s->disc  = (p_rr->values[2].type == P_INTEGER) ? p_rr->values[2].value.integer : 0;
      cp_result_str(&p_s->title, p_rr->values + 3);
      cp_result_str(&p_s->artist, p_rr->values + 4);
      cp_result_str(&p_s->album, p_rr->values + 5);
    }
  }

  return songs;
}
