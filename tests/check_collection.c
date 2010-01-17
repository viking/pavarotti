#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <sqlite3.h>
#include "../src/collection.h"

#define NUM_SONGS 5

char *original_home;
char *unreadable_path;
char home[2048];
char fixture_path[2048];
sqlite3 *s_db;
sqlite3_stmt *s_stmt;

typedef struct {
  char *filename;
  int track, disc, year;
  char *title, *artist, *album;
} e_result;

/* test helpers */
char *
fixture_file(name)
  const char *name;
{
  char *retval;
  retval = (char *)malloc(sizeof(char) * 2048);
  sprintf(retval, "%s/%s", fixture_path, name);
  return(retval);
}

e_result *
expected_set(filename, track, disc, year, title, artist, album)
  char *filename;
  int track, disc, year;
  char *title, *artist, *album;
{
  e_result *r;

  r = (e_result *)malloc(sizeof(e_result));
  r->filename = fixture_file(filename);
  r->track = track;
  r->disc = disc;
  r->year = year;

  r->title = NULL;
  if (title != NULL) {
    r->title = (char *)malloc(sizeof(char) * (strlen(title) + 1));
    strcpy(r->title, title);
  }

  r->artist = NULL;
  if (artist != NULL) {
    r->artist = (char *)malloc(sizeof(char) * (strlen(artist) + 1));
    strcpy(r->artist, artist);
  }

  r->album = NULL;
  if (album != NULL) {
    r->album = (char *)malloc(sizeof(char) * (strlen(album) + 1));
    strcpy(r->album, album);
  }

  return r;
}

void
free_expected(r)
  e_result *r;
{
  free(r->filename);
  if (r->artist != NULL)
    free(r->artist);
  if (r->title != NULL)
    free(r->title);
  if (r->album != NULL)
    free(r->album);
  free(r);
}

void
compare_db_str(e_str, c_index, r_index)
  char *e_str;
  int c_index;
  int r_index;
{
  int type;
  const char *a_str, *colname;

  type = sqlite3_column_type(s_stmt, c_index);
  colname = sqlite3_column_name(s_stmt, c_index);
  if (e_str == NULL) {
    fail_unless(type == SQLITE_NULL, "%s (row %d): Expected NULL", colname, r_index);
  }
  else {
    fail_if(type == SQLITE_NULL, "%s (row %d): Expected not NULL", colname, r_index);

    a_str = sqlite3_column_text(s_stmt, c_index);
    fail_unless(strcmp(e_str, a_str) == 0, "%s (row %d): Expected %s, got %s", colname, r_index, e_str, a_str);
  }
}

void
compare_db_int(e_int, c_index, r_index)
  int e_int;
  int c_index;
  int r_index;
{
  int a_int;
  const char *colname;

  colname = sqlite3_column_name(s_stmt, c_index);
  a_int = sqlite3_column_int(s_stmt, c_index);
  fail_unless(e_int == a_int, "%s (row %d): Expected %d, got %d", colname, r_index, e_int, a_int);
}

void
compare_str(e_str, a_str)
  const char *e_str;
  const char *a_str;
{
  if (e_str == NULL) {
    fail_unless(a_str == NULL, "Expected NULL, got %s", a_str);
  }
  else {
    fail_if(a_str == NULL, "Expected %s, got NULL", e_str);
    fail_unless(strcmp(e_str, a_str) == 0, "Expected %s, got %s", e_str, a_str);
  }
}

/* fixtures */
void
setup()
{
  char wd[2048];
  char db_path[2048];

  getcwd(wd, 2048);
  original_home = getenv("HOME");
  setenv("HOME", wd, 1);
  sprintf(fixture_path, "%s/fixtures", wd);

  sprintf(db_path, "%s/.pavarotti/database.sqlite3", wd);
  if (sqlite3_open(db_path, &s_db) != SQLITE_OK) {
    fail("Couldn't open database!");
  }
  sqlite3_exec(s_db, "DELETE FROM songs", NULL, NULL, NULL);

  /* make one file unreadable */
  unreadable_path = fixture_file("unreadable.mp3");
  chmod(unreadable_path, 0000);
}

void
teardown()
{
  if (s_stmt != NULL)
    sqlite3_finalize(s_stmt);
  if (s_db != NULL)
    sqlite3_close(s_db);

  chmod(unreadable_path, 0644);
  free(unreadable_path);

  setenv("HOME", original_home, 1);
}

/* actual tests */
START_TEST(test_discovering) {
  int result, i;
  char *unreadable;
  e_result *expected[NUM_SONGS];

  expected[0] = expected_set(    "boo.mp3", 4, 2, 2009,   "Boo", "Viking",  "Small");
  expected[1] = expected_set("foo/bar.mp3", 1, 0,    0,   "Foo",    "Bar", "Foobar");
  expected[2] = expected_set(    "hey.mp3", 3, 1, 2009,   "Hey", "Viking",   "Huge");
  expected[3] = expected_set("no_tags.mp3", 0, 0,    0,    NULL,     NULL,     NULL);
  expected[4] = expected_set( "quotes.mp3", 0, 0,    0, "rofl'",     NULL,     NULL);

  collection_discover(fixture_path);

  sqlite3_prepare_v2(s_db, "SELECT filename, track, disc, year, title, artist, album, seconds FROM songs ORDER BY filename", -1, &s_stmt, NULL);
  result = sqlite3_step(s_stmt);
  for (i = 0; result == SQLITE_ROW; i++) {
    fail_if(i >= NUM_SONGS, "Too many rows");

    compare_db_str(expected[i]->filename, 0, i);
    compare_db_int(expected[i]->track,    1, i);
    compare_db_int(expected[i]->disc,     2, i);
    compare_db_int(expected[i]->year,     3, i);
    compare_db_str(expected[i]->title,    4, i);
    compare_db_str(expected[i]->artist,   5, i);
    compare_db_str(expected[i]->album,    6, i);
    free_expected(expected[i]);

    result = sqlite3_step(s_stmt);
  }
  fail_if(i < NUM_SONGS, "Not enough rows");
}
END_TEST

START_TEST(test_discovering_with_unreadable_directory) {
  collection_discover("roflsauce");
}
END_TEST

START_TEST(test_discovering_with_unreadable_subdir) {
  char *path;
  struct stat buf;

  path = fixture_file("foo");
  stat(path, &buf);
  chmod(path, 0000);

  collection_discover(fixture_path);

  chmod(path, buf.st_mode);
  free(path);
}
END_TEST

START_TEST(test_count) {
  int count;

  collection_discover(fixture_path);
  count = collection_count();
  fail_unless(count == NUM_SONGS);
}
END_TEST

START_TEST(test_find) {
  int i;
  p_song *songs;
  e_result *expected[NUM_SONGS];

  expected[0] = expected_set(    "boo.mp3", 4, 2, 2009,   "Boo", "Viking",  "Small");
  expected[1] = expected_set("foo/bar.mp3", 1, 0,    0,   "Foo",    "Bar", "Foobar");
  expected[2] = expected_set(    "hey.mp3", 3, 1, 2009,   "Hey", "Viking",   "Huge");
  expected[3] = expected_set("no_tags.mp3", 0, 0,    0,    NULL,     NULL,     NULL);
  expected[4] = expected_set( "quotes.mp3", 0, 0,    0, "rofl'",     NULL,     NULL);

  collection_discover(fixture_path);
  songs = collection_find("filename");
  for (i = 0; i < NUM_SONGS; i++) {
    compare_str(expected[i]->filename, songs[i].filename);
    compare_str(expected[i]->title, songs[i].title);
    compare_str(expected[i]->artist, songs[i].artist);
    compare_str(expected[i]->album, songs[i].album);
    fail_unless(songs[i].track == expected[i]->track, "Expected %d, got %d", expected[i]->track, songs[i].track);
    fail_unless(songs[i].disc == expected[i]->disc, "Expected %d, got %d", expected[i]->disc, songs[i].disc);
    fail_unless(songs[i].year == expected[i]->year, "Expected %d, got %d", expected[i]->year, songs[i].year);
  }
  fail_if(i < NUM_SONGS, "Not enough songs");
}
END_TEST

/* test runner */
Suite *
collection_suite()
{
  Suite *s = suite_create("Collection");

  TCase *tc_core = tcase_create("Core");
  tcase_add_checked_fixture (tc_core, setup, teardown);
  tcase_add_test(tc_core, test_discovering);
  tcase_add_test(tc_core, test_discovering_with_unreadable_directory);
  tcase_add_test(tc_core, test_discovering_with_unreadable_subdir);
  tcase_add_test(tc_core, test_count);
  tcase_add_test(tc_core, test_find);
  suite_add_tcase(s, tc_core);

  return s;
}

int
main()
{
  int number_failed;
  Suite *s = collection_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
