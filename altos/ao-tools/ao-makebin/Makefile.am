bin_PROGRAMS=ao-makebin

AM_CFLAGS=$(WARN_CFLAGS) -I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)
AO_ELFTOHEX_LIBS=$(top_builddir)/ao-tools/lib/libao-tools.a

ao_makebin_DEPENDENCIES = $(AO_ELFTOHEX_LIBS)

ao_makebin_LDADD=$(AO_ELFTOHEX_LIBS) -lelf

ao_makebin_SOURCES=ao-makebin.c

man_MANS = ao-makebin.1
