bin_PROGRAMS=ao-test-igniter

AM_CFLAGS=$(WARN_CFLAGS) -I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)

ao_test_igniter_DEPENDENCIES = $(top_builddir)/ao-tools/lib/libao-tools.a

ao_test_igniter_LDADD=$(top_builddir)/ao-tools/lib/libao-tools.a $(LIBUSB_LIBS)

ao_test_igniter_SOURCES=ao-test-igniter.c

man_MANS = ao-test-igniter.1
