#	$Id$	

noinst_LTLIBRARIES = libversit.la

libversit_la_SOURCES = vcc.c vobject.c

$(srcdir)/vcc.c: vcc.y
	bison $(srcdir)/vcc.y -o $(srcdir)/vcc.c
