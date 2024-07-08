include $(top_srcdir)/config/Make-rules

SUBDIRS = \
	cci \
	.

H_FILES = \
	cci_configuration

nobase_include_HEADERS = $(H_FILES)

libarchdir = $(libdir)$(LIB_ARCH_SUFFIX)
libarch_LTLIBRARIES = libcci.la

pkgconfigdir = $(libarchdir)/pkgconfig
pkgconfig_DATA = cci/cci.pc

# no own sources here
libcci_la_SOURCES =

# dummy C++ source to cause C++ linking
nodist_EXTRA_libcci_la_SOURCES = cci_nonexistent.cpp

# add main library
libcci_la_LIBADD = cci/libcci.la

libcci_la_LDFLAGS = $(EXTRA_LDFLAGS) -release $(VERSION)

uninstall-hook:
	test ! -d "$(includedir)"   || rmdir --ignore-fail-on-non-empty "$(includedir)"
	test ! -d "$(pkgconfigdir)" || rmdir --ignore-fail-on-non-empty "$(pkgconfigdir)"
	test ! -d "$(libarchdir)"   || rmdir --ignore-fail-on-non-empty "$(libarchdir)"


## Taf!
