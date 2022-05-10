JAVA_RES=16 32 48 64 128 256
MAC_RES=16 32 128 256 512
WIN_RES=16 24 32 48 64 72 96 128 180 256
AMAZON_RES=114 512
RES=$(shell echo $(JAVA_RES) $(MAC_RES) $(WIN_RES) $(AMAZON_RES)| awk '{ for (i = 1; i <= NF; i++) printf("%s\n", $$i); }' | sort -n -u)

# Application icon base names

AM_NAME		= altusmetrum-altosui
MP_NAME		= altusmetrum-micropeak
TG_NAME		= altusmetrum-telegps

# File icon base names

EEPROM_NAME	= application-vnd.altusmetrum.eeprom
TELEM_NAME	= application-vnd.altusmetrum.telemetry
MPD_NAME	= application-vnd.altusmetrum.micropeak

# Application icons

AM_ICON		= $(AM_NAME).svg
MP_ICON		= $(MP_NAME).svg
TG_ICON		= $(TG_NAME).svg

# File icons (note that MicroPeak uses the application icon for files)

EEPROM_ICON	= $(EEPROM_NAME).svg
TELEM_ICON	= $(TELEM_NAME).svg
MPD_ICON	= $(MPD_NAME).svg

LED_SVG = \
	redoff.svg \
	greenoff.svg \
	greenled.svg \
	grayon.svg \
	grayled.svg \
	redled.svg
LED_PNG = $(LED_SVG:.svg=.png)

LED_ICONS = $(LED_SVG) $(LED_PNG)

# Files needed for Mac OSX icons

MAC_AM_FILES	= $(shell for i in $(MAC_RES); do echo $(AM_NAME)-$$i.png; done)
MAC_MP_FILES	= $(shell for i in $(MAC_RES); do echo $(MP_NAME)-$$i.png; done)
MAC_TG_FILES	= $(shell for i in $(MAC_RES); do echo $(TG_NAME)-$$i.png; done)
MAC_EEPROM_FILES= $(shell for i in $(MAC_RES); do echo $(EEPROM_NAME)-$$i.png; done)
MAC_TELEM_FILES	= $(shell for i in $(MAC_RES); do echo $(TELEM_NAME)-$$i.png; done)
MAC_MPD_FILES	= $(shell for i in $(MAC_RES); do echo $(MPD_NAME)-$$i.png; done)

ICNS_FILES	= $(AM_NAME).icns $(MP_NAME).icns $(TG_NAME).icns \
		  $(EEPROM_NAME).icns $(TELEM_NAME).icns $(MPD_NAME).icns

# Files needed for Windows icons

WIN_AM_FILES	= $(shell for i in $(WIN_RES); do echo $(AM_NAME)-$$i.png; done)
WIN_MP_FILES	= $(shell for i in $(WIN_RES); do echo $(MP_NAME)-$$i.png; done)
WIN_TG_FILES	= $(shell for i in $(WIN_RES); do echo $(TG_NAME)-$$i.png; done)
WIN_EEPROM_FILES= $(shell for i in $(WIN_RES); do echo $(EEPROM_NAME)-$$i.png; done)
WIN_TELEM_FILES	= $(shell for i in $(WIN_RES); do echo $(TELEM_NAME)-$$i.png; done)
WIN_MPD_FILES	= $(shell for i in $(WIN_RES); do echo $(MPD_NAME)-$$i.png; done)

ICO_FILES	= $(AM_NAME).ico $(MP_NAME).ico $(TG_NAME).ico \
		  $(EEPROM_NAME).ico $(TELEM_NAME).ico $(MPD_NAME).ico

EXE_FILES	= $(AM_NAME).exe $(MP_NAME).exe $(TG_NAME).exe \
		  $(EEPROM_NAME).exe $(TELEM_NAME).exe $(MPD_NAME).exe

# Files needed for Java internal icons

JAVA_AM_FILES	= $(shell for i in $(JAVA_RES); do echo $(AM_NAME)-$$i.png; done)
JAVA_MP_FILES	= $(shell for i in $(JAVA_RES); do echo $(MP_NAME)-$$i.png; done)
JAVA_TG_FILES	= $(shell for i in $(JAVA_RES); do echo $(TG_NAME)-$$i.png; done)

JAVA_FILES	= $(JAVA_AM_FILES) $(JAVA_MP_FILES) $(JAVA_TG_FILES)

# Files needed for Amazon store

AMAZON_FILES	= $(shell for i in $(AMAZON_RES); do echo $(AM_NAME)-$$i.png; done)

# PNG files needed by anyone

AM_FILES	= $(shell for i in $(RES); do echo $(AM_NAME)-$$i.png; done)
MP_FILES	= $(shell for i in $(RES); do echo $(MP_NAME)-$$i.png; done)
TG_FILES	= $(shell for i in $(RES); do echo $(TG_NAME)-$$i.png; done)
EEPROM_FILES	= $(shell for i in $(RES); do echo $(EEPROM_NAME)-$$i.png; done)
TELEM_FILES	= $(shell for i in $(RES); do echo $(TELEM_NAME)-$$i.png; done)
IN_MPD_FILES	= $(shell for i in $(RES); do echo $(MPD_NAME)-$$i.png; done)

# XPM file needed by debian

AM_XPM = altusmetrum.xpm

ICON_THEME=$(datadir)/icons/hicolor

appicondir = $(ICON_THEME)/scalable/apps
appicon_DATA = $(AM_ICON) $(MP_ICON) $(TG_ICON)

MIME_ICONS = $(EEPROM_ICON) $(TELEM_ICON) $(MPD_ICON)

mimeicondir = $(ICON_THEME)/scalable/mimetypes
mimeicon_DATA = $(MIME_ICONS)

if INSTALL_SHARED_MIME_INFO
mimedir = $(datadir)/mime/packages
mime_DATA = org-altusmetrum-mimetypes.xml
endif

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = altusmetrum.xpm

EXTRA_DIST = $(appicon_DATA) $(mimeicon_DATA) $(mime_DATA) windows-stub.c

res:
	echo $(RES)

java-altosui:
	@echo $(JAVA_AM_FILES)

java-micropeak:
	@echo $(JAVA_MP_FILES)

java-telegps:
	@echo $(JAVA_TG_FILES)

fat: all $(ICO_FILES) $(ICNS_FILES) $(EXE_FILES)

all-local: $(JAVA_FILES) $(AM_XPM) $(LED_ICONS) $(AMAZON_FILES)

clean-local:
	$(RM) $(AM_NAME)-*.png $(TG_NAME)-*.png $(MP_NAME)-*.png
	$(RM) altus-metrum-*.png micropeak-*.png telegps-*.png
	$(RM) $(EEPROM_NAME)-*.png $(TELEM_NAME)-*.png $(MPD_NAME)-*.png
	$(RM) *.build *.ico *.rc *.icns *.o *.exe $(MPD_ICON)
	$(RM) altusmetrum.xpm
	$(RM) $(LED_ICONS)

if INSTALL_SHARED_MIME_INFO
install-data-hook:
	update-mime-database $(DESTDIR)$(datadir)/mime
	update-icon-caches $(DESTDIR)$(ICON_THEME)
endif

$(LED_ICONS): led.5c
	nickle led.5c $@

$(MPD_ICON): $(MP_ICON)
	$(LN_S) $(MP_ICON) $@

XPM_SRC=altusmetrum-altosui-32.png

altusmetrum.xpm: $(XPM_SRC)
	pngtopnm -alpha $(XPM_SRC) > mask-$@ && \
		pngtopnm $(XPM_SRC) | \
		ppmtoxpm -alphamask=mask-$@ > $@ && rm mask-$@

$(AM_FILES): $(AM_NAME).build
$(MP_FILES): $(MP_NAME).build
$(TG_FILES): $(TG_NAME).build

SUFFIXES=.svg .build .icns .ico .rc .o .exe

.svg.build:
	for i in $(RES); do rsvg-convert -w $$i -h $$i -o $*-$$i.png $*.svg; done && touch $@

.build.icns:
	png2icns $@ $(shell for i in $(MAC_RES); do echo $*-$$i.png; done)

.build.ico:
	icotool -c -o $@ $(shell for i in $(WIN_RES); do echo $*-$$i.png; done)

.ico.rc:
	./make-rc "$*" $(VERSION) > $@


MINGCC32=i686-w64-mingw32-gcc
MINGWINDRES=i686-w64-mingw32-windres
MINGFLAGS=-Wall -DWINDOWS -mwindows
MINGLIBS=-lshlwapi

.rc.o:
	$(MINGWINDRES) $*.rc $@

.o.exe:
	$(MINGCC32) -o $@ $(MINGFLAGS) windows-stub.o $*.o $(MINGLIBS)

$(EXE_FILES): windows-stub.o make-rc

windows-stub.o: windows-stub.c
	$(MINGCC32) -c $(MINGFLAGS) windows-stub.c

altosdroid.png: altosdroid.svg
	rsvg-convert -a -b white -h 480 altosdroid.svg | pngtopnm | pnmpad -white -width 512 -height 512 | pnmtopng > $@
