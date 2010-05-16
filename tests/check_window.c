#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include "../src/window.h"

/* fixtures */
void
setup()
{
  gtk_init(NULL, NULL);
}

void
teardown()
{
}


/* actual tests */
START_TEST(test_window_new) {
  p_window *p_win;
  GtkWindow *g_win;

  p_win = window_new();
  g_win = p_win->window;

  fail_if(g_win == NULL);
  fail_unless(strcmp(gtk_window_get_title(g_win), "Pavarotti") == 0);
  fail_unless(gtk_widget_get_visible(g_win));

  window_free(p_win);
}
END_TEST

/* test runner */
Suite *
window_suite()
{
  Suite *s = suite_create("Window");

  TCase *tc_core = tcase_create("Core");
  tcase_add_checked_fixture (tc_core, setup, teardown);
  tcase_add_test(tc_core, test_window_new);
  suite_add_tcase(s, tc_core);

  return s;
}

int
main()
{
  int number_failed;
  Suite *s = window_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
