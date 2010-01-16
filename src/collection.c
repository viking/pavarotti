#include "collection.h"

void
collection_discover(path)
  const char *path;
{
  p_database *p_db;
  p_song *p_s;
  DIR *dp;
  struct dirent *ep;
  int result;
  char filename[2048], sql[4192];

  p_db = database_open();
  dp = opendir(path);
  while(ep = readdir(dp)) {
    if (ep->d_type == DT_REG) {
      sprintf(filename, "%s/%s", path, ep->d_name);
      p_s = song_read_file(filename);

      sprintf(sql, "INSERT INTO songs (filename, track, disc, title, artist, album) VALUES('%s', %d, %d, '%s', '%s', '%s')", filename, p_s->track, p_s->disc, (p_s->title == NULL) ? "Unknown" : p_s->title, (p_s->artist == NULL) ? "Unknown" : p_s->artist, (p_s->album == NULL) ? "Unknown" : p_s->album);
      result = database_exec(p_db, sql);

      song_free(p_s);
    }
  }

  database_close(p_db);
}
