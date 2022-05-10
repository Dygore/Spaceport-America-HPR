bin_PROGRAMS=ao-cal-freq

AM_CFLAGS=$(WARN_CFLAGS) -I$(top_srcdir)/ao-tools/lib $(LIBUSB_CFLAGS)

ao_cal_freq_DEPENDENCIES = $(top_builddir)/ao-tools/lib/libao-tools.a

ao_cal_freq_LDADD=$(top_builddir)/ao-tools/lib/libao-tools.a $(LIBUSB_LIBS) -lm

ao_cal_freq_SOURCES=ao-cal-freq.c

man_MANS = ao-cal-freq.1
