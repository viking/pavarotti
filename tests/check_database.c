#include <stdlib.h>
#include <check.h>
#include "../src/database.h"

/* static vars */
char *original_home;
char home[2048];
char dot_directory[2048];
char db_path[2048];
p_database *p_db;
sqlite3 *s_db;
sqlite3_stmt *s_stmt;

/* test fixtures */
void
setup()
{
  p_db = NULL;
  original_home = getenv("HOME");
  setenv("HOME", getcwd(home, 2048), 1);
  sprintf(dot_directory, "%s/.pavarotti", home);
  sprintf(db_path, "%s/database.sqlite3", dot_directory);
}

void
teardown()
{
  setenv("HOME", original_home, 1);
  if (p_db != NULL)
    database_close(p_db);
  if (s_stmt != NULL)
    sqlite3_finalize(s_stmt);
  if (s_db != NULL)
    sqlite3_close(s_db);
}

/* test helpers */
void
remove_dot_directory()
{
  char cmd[2048];
  sprintf(cmd, "rm -fr %s", dot_directory);
  fail_if(system(cmd), "Couldn't remove dot directory");
}

void
open_sqlite_db()
{
  if (sqlite3_open(db_path, &s_db) != SQLITE_OK) {
    fail("Couldn't open database!");
  }
}

/* actual tests */
START_TEST(test_no_home_env)
{
  unsetenv("HOME");

  p_db = database_open();
  fail_if(p_db != NULL, "didn't fail when HOME is empty");
}
END_TEST

START_TEST(test_creates_dot_directory)
{
  remove_dot_directory();

  p_db = database_open();
  fail_if(p_db == NULL, NULL);
  fail_if(chdir(dot_directory) != 0, "Couldn't chdir to %s", dot_directory);
}
END_TEST

START_TEST(test_unreadable_dot_directory)
{
  remove_dot_directory();
  mkdir(dot_directory, 0000);

  p_db = database_open();
  fail_if(p_db != NULL, "didn't fail when dot directory is unreadable");
}
END_TEST

START_TEST(test_unreadable_database)
{
  FILE *fp;

  remove_dot_directory();
  mkdir(dot_directory, 0755);
  if ((fp = fopen(db_path, "w")) == NULL)
    fail("Couldn't open file");
  fclose(fp);
  if (chmod(db_path, 0000) < 0)
    fail("Couldn't chmod: %s", strerror(errno));

  p_db = database_open();
  fail_if(p_db != NULL, "didn't fail when database is unreadable");
}
END_TEST

START_TEST(test_creating_schema)
{
  const unsigned char *data;
  int i, result;

  remove_dot_directory();

  p_db = database_open();
  database_close(p_db);
  p_db = NULL;

  open_sqlite_db();

  sqlite3_prepare_v2(s_db, "SELECT name FROM SQLITE_MASTER WHERE type = 'table' ORDER BY name", -1, &s_stmt, NULL);
  result = sqlite3_step(s_stmt);
  fail_if(result == SQLITE_DONE, "Schema is empty");

  char *expected[] = { "songs", NULL };
  for (i = 0; result == SQLITE_ROW; i++) {
    fail_if(expected[i] == NULL, "Too many tables found");

    data = sqlite3_column_text(s_stmt, 0);
    fail_if(strcmp(expected[i], data) != 0, "Expected %s, got %s", expected[i], data);

    result = sqlite3_step(s_stmt);
  }
  fail_if(expected[i] != NULL, "Not enough tables found");
}
END_TEST

START_TEST(test_escaping_strings) {
  char *expected = "foo''bar";
  char *result = database_escape("foo'bar");
  fail_if(strcmp(expected, result) != 0, "Expected: %s; Got: %s", expected, result);
  free(result);
}
END_TEST

START_TEST(test_inserting)
{
  const unsigned char *data;
  int i, result;

  remove_dot_directory();

  p_db = database_open();
  result = database_exec(p_db, "INSERT INTO songs (title) VALUES('Huge');");
  fail_if(result != 0, "Query failed");
  database_close(p_db);
  p_db = NULL;

  open_sqlite_db();

  sqlite3_prepare_v2(s_db, "SELECT title FROM songs", -1, &s_stmt, NULL);
  result = sqlite3_step(s_stmt);
  fail_if(result == SQLITE_DONE, "Nothing was inserted");

  for (i = 0; result == SQLITE_ROW; i++) {
    fail_if(i > 0, "Too many results");
    data = sqlite3_column_text(s_stmt, 0);
    fail_if(strcmp("Huge", data) != 0, "Expected Huge, got %s", data);
    result = sqlite3_step(s_stmt);
  }
}
END_TEST

START_TEST(test_selecting) {
  p_result_set *p_rs;
  int i, p_type, p_int_value;
  unsigned char *p_text_value;

  remove_dot_directory();

  p_db = database_open();
  open_sqlite_db();

  for (i = 0; i < 101; i++) {
    fail_if(sqlite3_exec(s_db, "INSERT INTO songs (title, track) VALUES('Huge', 1);", NULL, NULL, NULL) != SQLITE_OK, "Couldn't insert");
  }

  p_rs = database_select(p_db, "SELECT title, track FROM songs");
  fail_if(p_rs->count != 101);

  for (i = 0; i < p_rs->count; i++) {
    fail_if(p_rs->rows[i].count != 2, "Expected %d, got %d", 2, p_rs->rows[i].count);

    p_type = p_rs->rows[i].values[0].type;
    p_text_value = p_rs->rows[i].values[0].value.text;
    fail_if(p_type != P_TEXT, "Expected %d, got %d", P_TEXT, p_type);
    fail_if(strcmp("Huge", p_text_value) != 0, "Expected %s, got %s", "Huge", p_text_value);

    p_type = p_rs->rows[i].values[1].type;
    p_int_value = p_rs->rows[i].values[1].value.integer;
    fail_if(p_type != P_INTEGER, "Expected %d, got %d", P_INTEGER, p_type);
    fail_if(p_int_value != 1, "Expected %d, got %d", 1, p_int_value);
  }

  database_free_result_set(p_rs);
}
END_TEST

START_TEST(test_selecting_with_bad_query) {
  p_db = database_open();
  fail_if(database_select(p_db, "SELECT BLAH FOO X BAR;") != NULL);
}
END_TEST

/* test runner */
Suite *
database_suite()
{
  Suite *s = suite_create("Database");

  /* Core test case */
  TCase *tc_core = tcase_create("Core");
  tcase_add_checked_fixture (tc_core, setup, teardown);
  tcase_add_test(tc_core, test_no_home_env);
  tcase_add_test(tc_core, test_creates_dot_directory);
  tcase_add_test(tc_core, test_unreadable_dot_directory);
  tcase_add_test(tc_core, test_unreadable_database);
  tcase_add_test(tc_core, test_creating_schema);
  tcase_add_test(tc_core, test_escaping_strings);
  tcase_add_test(tc_core, test_inserting);
  tcase_add_test(tc_core, test_selecting);
  tcase_add_test(tc_core, test_selecting_with_bad_query);
  suite_add_tcase(s, tc_core);

  return s;
}

int
main()
{
  int number_failed;
  Suite *s = database_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
