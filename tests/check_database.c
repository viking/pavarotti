#include <check.h>
#include "../src/database.h"
#define NUM_TABLES 1

p_database *db;
char *original_home, home[2048], dot_directory[2048];

/* test fixtures */
void
setup()
{
  db = NULL;
  original_home = getenv("HOME");
  setenv("HOME", getcwd(home, 2048), 1);
  sprintf(dot_directory, "%s/.pavarotti", home);
}

void
teardown()
{
  setenv("HOME", original_home, 1);
  if (db != NULL)
    database_close(db);
}

/* test helpers */
void
remove_dot_directory()
{
  char cmd[2048];
  int result;
  sprintf(cmd, "rm -fr %s", dot_directory);
  result = system(cmd);
}

/* actual tests */
START_TEST(test_no_home_env)
{
  unsetenv("HOME");

  db = database_open();
  fail_if(db != NULL, "didn't fail when HOME is empty");
}
END_TEST

START_TEST(test_creates_dot_directory)
{
  remove_dot_directory();

  db = database_open();
  fail_if(db == NULL, NULL);
  fail_if(chdir(dot_directory) != 0, "Couldn't chdir to %s", dot_directory);
}
END_TEST

START_TEST(test_unreadable_dot_directory)
{
  remove_dot_directory();
  mkdir(dot_directory, 0000);

  db = database_open();
  fail_if(db != NULL, "didn't fail when dot directory is unreadable");
}
END_TEST

START_TEST(test_unreadable_database)
{
  char path[2048];
  FILE *fp;

  remove_dot_directory();
  mkdir(dot_directory, 0755);
  sprintf(path, "%s/database.sqlite3", dot_directory);
  if ((fp = fopen(path, "w")) == NULL)
    fail("Couldn't open file");
  fclose(fp);
  chmod(path, 0000);

  db = database_open();
  fail_if(db != NULL, "didn't fail when database is unreadable");
}
END_TEST

START_TEST(test_creating_schema)
{
  char path[2048];
  const unsigned char *data;
  int i, result;
  sqlite3 *s_db;
  sqlite3_stmt *s_stmt;

  remove_dot_directory();

  db = database_open();
  database_close(db);
  db = NULL;

  sprintf(path, "%s/database.sqlite3", dot_directory);
  if (sqlite3_open(path, &s_db) != SQLITE_OK) {
    fail("Couldn't open database!");
  }
  sqlite3_prepare_v2(s_db, "SELECT name FROM SQLITE_MASTER WHERE type = 'table' ORDER BY name", -1, &s_stmt, NULL);
  result = sqlite3_step(s_stmt);
  if (result == SQLITE_DONE) {
    sqlite3_finalize(s_stmt);
    fail_if(result == SQLITE_DONE, "Schema is empty");
  }

  char *expected[NUM_TABLES] = { "songs" };
  for (i = 0; result == SQLITE_ROW; i++) {
    if (i >= NUM_TABLES) {
      sqlite3_finalize(s_stmt);
      fail("Too many tables found");
    }
    data = sqlite3_column_text(s_stmt, 0);
    if (strcmp(expected[i], data) != 0) {
      sqlite3_finalize(s_stmt);
      fail("Expected %s, got %s", expected[i], data);
    }
    result = sqlite3_step(s_stmt);
  }
  if (i < NUM_TABLES) {
    sqlite3_finalize(s_stmt);
    fail("Not enough tables found");
  }

  sqlite3_finalize(s_stmt);
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
  suite_add_tcase(s, tc_core);

  return s;
}

int
main()
{
  int number_failed;
  Suite *s = database_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
