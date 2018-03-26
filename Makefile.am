include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_gb28181

mod_LTLIBRARIES = mod_gb28181.la
mod_blacklist_la_SOURCES  = mod_gb28181.c
mod_blacklist_la_CFLAGS   = $(AM_CFLAGS)
mod_blacklist_la_LIBADD   = $(switch_builddir)/libfreeswitch.la
mod_blacklist_la_LDFLAGS  = -avoid-version -module -no-undefined -shared
