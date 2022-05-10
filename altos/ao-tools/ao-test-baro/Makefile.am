bin_PROGRAMS=ao-test-baro

AM_CFLAGS=$(WARN_CFLAGS) -I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)

ao_test_baro_DEPENDENCIES = $(top_builddir)/ao-tools/lib/libao-tools.a

ao_test_baro_LDADD=$(top_builddir)/ao-tools/lib/libao-tools.a $(LIBUSB_LIBS)

ao_test_baro_SOURCES=ao-test-baro.c

man_MANS = ao-test-baro.1
