#	$Id$	

noinst_LIBRARIES = libversit.a

libversit_a_SOURCES = vcc.c vobject.c

$(srcdir)/vcc.c: vcc.y
	bison $(srcdir)/vcc.y -o $(srcdir)/vcc.c
