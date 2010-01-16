#include <stdlib.h>
#include <id3tag.h>

typedef struct {
  char *filename;
  char *title;
  char *artist;
  char *album;
  int track;
  int disc;
  int year;
} p_song;

p_song *song_read_file(const char *);
void song_free(p_song *);
