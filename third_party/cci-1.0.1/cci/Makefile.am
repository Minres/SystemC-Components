include $(top_srcdir)/config/Make-rules

SUBDIRS = 
H_FILES =
NO_H_FILES =
CXX_FILES =
INCDIRS =
LIBADD =

# include file lists for sub-directories
include cfg/files.am
include core/files.am
include utils/files.am
	
localincludedir = $(includedir)/cci
nobase_localinclude_HEADERS = $(H_FILES)

noinst_LTLIBRARIES = libcci.la

libcci_la_SOURCES = $(NO_H_FILES) $(CXX_FILES)

libcci_la_LIBADD = $(LIBADD)

uninstall-hook:
	test ! -d "$(localincludedir)" || ( set -e ; cd "$(localincludedir)"; \
		for dir in $(INCDIRS) ; do test ! -d $$dir || rmdir $$dir ; done ; \
		cd - ; rmdir "$(localincludedir)" )

## Taf!
