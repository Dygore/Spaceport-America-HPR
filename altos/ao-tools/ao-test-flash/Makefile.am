bin_PROGRAMS=ao-test-flash

AM_CFLAGS=$(WARN_CFLAGS) -I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)

ao_test_flash_DEPENDENCIES = $(top_builddir)/ao-tools/lib/libao-tools.a

ao_test_flash_LDADD=$(top_builddir)/ao-tools/lib/libao-tools.a $(LIBUSB_LIBS)

ao_test_flash_SOURCES=ao-test-flash.c

man_MANS = ao-test-flash.1
