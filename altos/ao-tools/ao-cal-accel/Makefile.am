bin_PROGRAMS=ao-cal-accel

AM_CFLAGS=$(WARN_CFLAGS) -I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)

ao_cal_accel_DEPENDENCIES = $(top_builddir)/ao-tools/lib/libao-tools.a

ao_cal_accel_LDADD=$(top_builddir)/ao-tools/lib/libao-tools.a $(LIBUSB_LIBS)

ao_cal_accel_SOURCES=ao-cal-accel.c

man_MANS = ao-cal-accel.1
