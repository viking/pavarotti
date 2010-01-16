## Process this file with automake to produce Makefile.in

TESTS = check_database check_song check_collection
check_PROGRAMS = check_database check_song check_collection

check_database_SOURCES = check_database.c $(top_builddir)/src/database.h
check_database_CFLAGS = @CHECK_CFLAGS@
check_database_LDADD = $(top_builddir)/src/database.o @CHECK_LIBS@

check_song_SOURCES = check_song.c $(top_builddir)/src/song.h
check_song_CFLAGS = @CHECK_CFLAGS@
check_song_LDADD = $(top_builddir)/src/song.o @CHECK_LIBS@

check_collection_SOURCES = check_collection.c $(top_builddir)/src/collection.h
check_collection_CFLAGS = @CHECK_CFLAGS@
check_collection_LDADD = $(top_builddir)/src/collection.o $(top_builddir)/src/database.o $(top_builddir)/src/song.o @CHECK_LIBS@
