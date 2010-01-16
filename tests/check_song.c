#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include "../src/song.h"

char fixture_path[2048];

/* test helpers */
char *
fixture_file(basename)
  char *basename;
{
  char *retval;
  retval = (char *)malloc(sizeof(char) * 2048);
  sprintf(retval, "%s/%s", fixture_path, basename);
  return retval;
}

/* fixtures */
void
setup()
{
  char wd[2048];
  getcwd(wd, 2048);
  sprintf(fixture_path, "%s/fixtures", wd);
}

void
teardown()
{
}

/* actual tests */
START_TEST(test_song_read_file) {
  p_song *song;
  char *file;

  file = fixture_file("hey.mp3");
  song = song_read_file(file);

  fail_unless(strcmp(song->filename, file) == 0, "Expected %s, got %s", file, song->filename);
  fail_unless(strcmp(song->title, "Hey") == 0);
  fail_unless(strcmp(song->artist, "Viking") == 0);
  fail_unless(strcmp(song->album, "Huge") == 0);
  fail_unless(song->track == 3);
  fail_unless(song->disc == 1);
  fail_unless(song->year == 2009);

  song_free(song);
  free(file);
}
END_TEST

START_TEST(test_song_read_file_with_no_tags) {
  p_song *song;
  char *file;

  file = fixture_file("no_tags.mp3");
  song = song_read_file(file);
  free(file);

  fail_unless(song->title == 0);
  fail_unless(song->artist == 0);
  fail_unless(song->album == 0);
  fail_unless(song->track == 0);
  fail_unless(song->disc == 0);
  fail_unless(song->year == 0);
  song_free(song);
}
END_TEST

START_TEST(test_song_read_non_existent_file) {
  fail_unless(song_read_file("foobar") == NULL);
}
END_TEST

/* test runner */
Suite *
song_suite()
{
  Suite *s = suite_create("Song");

  TCase *tc_core = tcase_create("Core");
  tcase_add_checked_fixture (tc_core, setup, teardown);
  tcase_add_test(tc_core, test_song_read_file);
  tcase_add_test(tc_core, test_song_read_file_with_no_tags);
  tcase_add_test(tc_core, test_song_read_non_existent_file);
  suite_add_tcase(s, tc_core);

  return s;
}

int
main()
{
  int number_failed;
  Suite *s = song_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
