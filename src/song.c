#include "song.h"

p_song *
song_read_file(path)
  const char *path;
{
  struct id3_file *i_file;
  struct id3_tag *i_tag;
  struct id3_frame *i_frame;
  union id3_field *i_field;
  char *str;
  p_song *p_s;

  if ((i_file = id3_file_open(path, ID3_FILE_MODE_READONLY)) == NULL)
    return NULL;
  if ((i_tag = id3_file_tag(i_file)) == NULL)
    return NULL;

  p_s = (p_song *)malloc(sizeof(p_song));
  p_s->title = p_s->artist = p_s->album = NULL;
  p_s->track = p_s->disc = p_s->year = 0;

  /* title */
  i_frame = id3_tag_findframe(i_tag, ID3_FRAME_TITLE, 0);
  if (i_frame != NULL) {
    i_field = id3_frame_field(i_frame, 1);
    p_s->title = id3_ucs4_latin1duplicate(id3_field_getstrings(i_field, 0));
  }

  /* artist */
  i_frame = id3_tag_findframe(i_tag, ID3_FRAME_ARTIST, 0);
  if (i_frame != NULL) {
    i_field = id3_frame_field(i_frame, 1);
    p_s->artist = id3_ucs4_latin1duplicate(id3_field_getstrings(i_field, 0));
  }

  /* album */
  i_frame = id3_tag_findframe(i_tag, ID3_FRAME_ALBUM, 0);
  if (i_frame != NULL) {
    i_field = id3_frame_field(i_frame, 1);
    p_s->album = id3_ucs4_latin1duplicate(id3_field_getstrings(i_field, 0));
  }

  /* track */
  i_frame = id3_tag_findframe(i_tag, ID3_FRAME_TRACK, 0);
  if (i_frame != NULL) {
    i_field = id3_frame_field(i_frame, 1);
    str = id3_ucs4_latin1duplicate(id3_field_getstrings(i_field, 0));
    p_s->track = strtol(str, NULL, 10);
    free(str);
  }

  /* disc */
  i_frame = id3_tag_findframe(i_tag, "TPOS", 0);
  if (i_frame != NULL) {
    i_field = id3_frame_field(i_frame, 1);
    str = id3_ucs4_latin1duplicate(id3_field_getstrings(i_field, 0));
    p_s->disc = strtol(str, NULL, 10);
    free(str);
  }

  /* year */
  i_frame = id3_tag_findframe(i_tag, ID3_FRAME_YEAR, 0);
  if (i_frame != NULL) {
    i_field = id3_frame_field(i_frame, 1);
    str = id3_ucs4_latin1duplicate(id3_field_getstrings(i_field, 0));
    p_s->year = strtol(str, NULL, 10);
    free(str);
  }

  return p_s;
}

void
song_free(p_s)
  p_song *p_s;
{
  if (p_s->artist != NULL)
    free(p_s->artist);
  if (p_s->album != NULL)
    free(p_s->album);
  if (p_s->title != NULL)
    free(p_s->title);

  free(p_s);
}
