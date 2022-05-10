bin_PROGRAMS=ao-usbtrng

AM_CFLAGS=$(WARN_CFLAGS) -I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)

ao_usbtrng_DEPENDENCIES = $(top_builddir)/ao-tools/lib/libao-tools.a

ao_usbtrng_LDADD=$(top_builddir)/ao-tools/lib/libao-tools.a $(LIBUSB_LIBS)

ao_usbtrng_SOURCES=ao-usbtrng.c

man_MANS = ao-usbtrng.1
