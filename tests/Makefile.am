## Process this file with automake to produce Makefile.in

TESTS = check_database
check_PROGRAMS = check_database
check_database_SOURCES = check_database.c $(top_builddir)/src/database.h
check_database_CFLAGS = @CHECK_CFLAGS@
check_database_LDADD = $(top_builddir)/src/database.o @CHECK_LIBS@
