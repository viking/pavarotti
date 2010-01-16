#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "database.h"
#include "song.h"

void collection_discover(const char *path);
int collection_count();
p_song *collection_find(const char *order_by);
