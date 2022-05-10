bin_PROGRAMS=ao-test-gps

AM_CFLAGS=$(WARN_CFLAGS) -I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)

ao_test_gps_DEPENDENCIES = $(top_builddir)/ao-tools/lib/libao-tools.a

ao_test_gps_LDADD=$(top_builddir)/ao-tools/lib/libao-tools.a $(LIBUSB_LIBS)

ao_test_gps_SOURCES=ao-test-gps.c

man_MANS = ao-test-gps.1
