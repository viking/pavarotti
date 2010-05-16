#include "main.h"

int
main(argc, argv)
  int argc;
  char **argv;
{
  p_window *p_win;

  gtk_init(&argc, &argv);
  p_win = window_new();
  gtk_main();

  return 0;
}
