#include "main.h"

int
main(argc, argv)
  int argc;
  char **argv;
{
  GtkWidget *window;
  GtkWidget *button;

  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  return 0;
}
