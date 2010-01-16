#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <sqlite3.h>
#include "../src/collection.h"

char *original_home;
char home[2048];
char fixture_path[2048];
sqlite3 *s_db;
sqlite3_stmt *s_stmt;

typedef struct {
  char *filename;
  int track, disc;
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
expected_row(filename, track, disc, title, artist, album)
  char *filename;
  int track, disc;
  char *title, *artist, *album;
{
  e_result *r;

  r = (e_result *)malloc(sizeof(e_result));
  r->filename = fixture_file(filename);
  r->track = track;
  r->disc = disc;

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
compare_strings(e_str, c_index, r_index)
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
compare_ints(e_int, c_index, r_index)
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
}

void
teardown()
{
  if (s_stmt != NULL)
    sqlite3_finalize(s_stmt);
  if (s_db != NULL)
    sqlite3_close(s_db);
  setenv("HOME", original_home, 1);
}

/* actual tests */
START_TEST(test_discovering) {
  int result, i, r_int, expected_num;
  char unsigned *r_str;
  e_result *expected[3];

  expected_num = 3;
  expected[0] = expected_row("boo.mp3", 4, 2, "Boo", "Viking", "Small");
  expected[1] = expected_row("hey.mp3", 3, 1, "Hey", "Viking", "Huge");
  expected[2] = expected_row("no_tags.mp3", 0, 0, "Unknown", "Unknown", "Unknown");

  collection_discover(fixture_path);

  sqlite3_prepare_v2(s_db, "SELECT filename, track, disc, title, artist, album, seconds FROM songs ORDER BY id", -1, &s_stmt, NULL);
  result = sqlite3_step(s_stmt);
  for (i = 0; result == SQLITE_ROW; i++) {
    fail_if(i >= expected_num, "Too many rows");

    compare_strings(expected[i]->filename, 0, i);
    compare_ints(expected[i]->track, 1, i);
    compare_ints(expected[i]->disc, 2, i);
    compare_strings(expected[i]->title, 3, i);
    compare_strings(expected[i]->artist, 4, i);
    compare_strings(expected[i]->album, 5, i);
    free_expected(expected[i]);

    result = sqlite3_step(s_stmt);
  }
  fail_if(i < expected_num, "Not enough rows");
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
