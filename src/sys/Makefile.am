OPT_SCRS=
EXT_LIB=
if THREADS
MAYBE_EVENTS=events
EXT_LIB+= events/libevents.la
OPT_SCRS+= sched.c thread.c
endif

SUBDIRS=$(MAYBE_EVENTS)
bermudaosdir=@libdir@/bermudaos
bermudaos_LTLIBRARIES=libsys.la
libsys_la_SOURCES=mem.c virt_timer.c epl.c $(OPT_SCRS)
libsys_la_LIBADD=$(EXT_LIB)
include ../../Makefile.flags
