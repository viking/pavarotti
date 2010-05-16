#include "window.h"

p_window *
window_new()
{
  p_window *p_win;

  p_win = (p_window *)malloc(sizeof(p_window));
  p_win->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(p_win->window, "Pavarotti");

  return p_win;
}

void
window_free(p_win)
  p_window *p_win;
{
  gtk_object_destroy(p_win->window);
  free(p_win);
}
