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

        d_filename = sanitize_str(path);
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
