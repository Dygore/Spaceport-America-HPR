SUBDIRS=ao-tools src doc icon altoslib libaltos altosuilib altosui micropeak ao-utils altosdroid telegps map-server

EXTRA_DIST = ChangeLog

MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || \
	(touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

if FATINSTALL
fat-install: fat
	cd altosui && $(MAKE) fat-install
	cd telegps && $(MAKE) fat-install
	cd micropeak && $(MAKE) fat-install
endif

fat:
	cd src && $(MAKE) all
	cd doc && $(MAKE) all
	cd libaltos && $(MAKE) fat
	cd altoslib && $(MAKE) all
	cd altosuilib && $(MAKE) all
	cd icon && $(MAKE) fat
	cd altosui && $(MAKE) fat
	cd micropeak && $(MAKE) fat
	cd telegps && $(MAKE) fat

fat_linux = \
	altosui/Altos-Linux-$(VERSION).sh altosui/Altos-Linux-$(VERSION).tar.bz2 \
	telegps/TeleGPS-Linux-$(VERSION).sh telegps/TeleGPS-Linux-$(VERSION).tar.bz2 \
	micropeak/MicroPeak-Linux-$(VERSION).sh micropeak/MicroPeak-Linux-$(VERSION).tar.bz2

fat_mac = \
	altosui/Altos-Mac-$(VERSION).dmg \
	telegps/TeleGPS-Mac-$(VERSION).dmg \
	micropeak/MicroPeak-Mac-$(VERSION).dmg

fat_windows = \
	altosui/Altos-Windows-$(VERSION_DASH).exe \
	telegps/TeleGPS-Windows-$(VERSION_DASH).exe \
	micropeak/MicroPeak-Windows-$(VERSION_DASH).exe

fat_android = \
	altosdroid/bin/AltosDroid-debug.apk \
	altosdroid/bin/AltosDroid-release.apk

fat_altos = \
	src/easymega-v1.0/easymega-v1.0-$(VERSION).ihx \
	src/easymega-v2.0/easymega-v2.0-$(VERSION).ihx \
	src/easymini-v1.0/easymini-v1.0-$(VERSION).ihx \
	src/easymotor-v2/easymotor-v2-$(VERSION).ihx \
	src/easytimer-v1/easytimer-v1-$(VERSION).ihx \
	src/telebt-v3.0/telebt-v3.0-$(VERSION).ihx \
	src/telebt-v4.0/telebt-v4.0-$(VERSION).ihx \
	src/teledongle-v3.0/teledongle-v3.0-$(VERSION).ihx \
	src/telegps-v1.0/telegps-v1.0-$(VERSION).ihx \
	src/telegps-v2.0/telegps-v2.0-$(VERSION).ihx \
	src/telemega-v1.0/telemega-v1.0-$(VERSION).ihx \
	src/telemega-v2.0/telemega-v2.0-$(VERSION).ihx \
	src/telemega-v3.0/telemega-v3.0-$(VERSION).ihx \
	src/telemega-v4.0/telemega-v4.0-$(VERSION).ihx \
	src/telemega-v5.0/telemega-v5.0-$(VERSION).ihx \
	src/telemetrum-v2.0/telemetrum-v2.0-$(VERSION).ihx \
	src/telemetrum-v3.0/telemetrum-v3.0-$(VERSION).ihx \
	src/telelco-v2.0/telelco-v2.0-$(VERSION).ihx \
	src/telefireeight-v1.0/telefireeight-v1.0-$(VERSION).ihx \
	src/telefireeight-v2.0/telefireeight-v2.0-$(VERSION).ihx

keithp-fat: fat
	ssh keithp.com mkdir -p public_html/altos-$(VERSION)
	scp -p $(fat_linux) $(fat_mac) $(fat_windows) $(fat_android) $(fat_altos) keithp.com:public_html/altos-$(VERSION)

set-java-versions:
	$(top_srcdir)/fix-java-versions org.altusmetrum.altoslib=$(ALTOSLIB_VERSION) org.altusmetrum.altosuilib=$(ALTOSUILIB_VERSION)
