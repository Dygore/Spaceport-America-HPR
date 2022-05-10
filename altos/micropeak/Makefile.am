JAVAROOT=classes
AM_JAVACFLAGS=$(JAVAC_VERSION_FLAGS) -encoding UTF-8 -Xlint:deprecation -Xlint:unchecked

man_MANS=micropeak.1

altoslibdir=$(libdir)/altos

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH=".:classes:../altoslib/*:../altosuilib/*:../libaltos:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar"

bin_SCRIPTS=micropeak

micropeakdir=$(datadir)/java

micropeak_JAVA= \
	MicroPeak.java \
	MicroData.java \
	MicroDownload.java \
	MicroExport.java \
	MicroFile.java \
	MicroFrame.java \
	MicroRaw.java \
	MicroSave.java \
	MicroSerial.java \
	MicroSerialLog.java \
	MicroFileChooser.java \
	MicroDeviceDialog.java \
	MicroUSB.java

JFREECHART_CLASS= \
    jfreechart.jar

JCOMMON_CLASS=\
    jcommon.jar

JAR=micropeak.jar

FATJAR=micropeak-fat.jar

if MULTI_ARCH
LIBALTOS_LINUX=libaltos_i686.so libaltos_amd64.so libaltos_aarch64.so libaltos_armel.so libaltos_armhf.so
else
LIBALTOS_LINUX=libaltos.so
endif

LIBALTOS= \
	$(LIBALTOS_LINUX) \
	libaltos.dylib \
	altos64.dll \
	altos.dll

ALTOSLIB_CLASS=\
	altoslib_$(ALTOSLIB_VERSION).jar

ALTOSUILIB_CLASS=\
	altosuilib_$(ALTOSUILIB_VERSION).jar

# Icons
ICONDIR=$(top_srcdir)/icon

JAVA_ICONS=\
	$(ICONDIR)/altusmetrum-micropeak-16.png \
	$(ICONDIR)/altusmetrum-micropeak-32.png \
	$(ICONDIR)/altusmetrum-micropeak-48.png \
	$(ICONDIR)/altusmetrum-micropeak-64.png \
	$(ICONDIR)/altusmetrum-micropeak-128.png\
	$(ICONDIR)/altusmetrum-micropeak-256.png

# icon base names for jar
ICONJAR= \
	-C $(ICONDIR) altusmetrum-micropeak-16.png \
	-C $(ICONDIR) altusmetrum-micropeak-32.png \
	-C $(ICONDIR) altusmetrum-micropeak-48.png \
	-C $(ICONDIR) altusmetrum-micropeak-64.png \
	-C $(ICONDIR) altusmetrum-micropeak-128.png\
	-C $(ICONDIR) altusmetrum-micropeak-256.png

WINDOWS_ICONS	=\
	../icon/altusmetrum-micropeak.ico \
	../icon/altusmetrum-micropeak.exe
	../icon/application-vnd.altusmetrum.micropeak.ico \
	../icon/application-vnd.altusmetrum.micropeak.ico

MACOSX_ICONS	=\
	../icon/altusmetrum-micropeak.icns \
	../icon/application-vnd.altusmetrum.micropeak.icns

LINUX_ICONS	=\
	$(ICONDIR)/altusmetrum-micropeak.svg \
	$(ICONDIR)/application-vnd.altusmetrum.micropeak.svg

LINUX_MIMETYPE	=\
	$(ICONDIR)/org-altusmetrum-mimetypes.xml

desktopdir = $(datadir)/applications
desktop_file = altusmetrum-micropeak.desktop
desktop_SCRIPTS = $(desktop_file)

all-local: micropeak-test micropeak-jdb $(JAR) $(MDWN)

clean-local:
	-rm -rf classes $(JAR) $(FATJAR) \
		MicroPeak-Linux-*.tar.bz2 MicroPeak-Mac-*.dmg MicroPeak-Windows-*.exe \
		altoslib_*.jar altosuilib_*.jar \
		$(JFREECHART_CLASS) $(JCOMMON_CLASS) $(LIBALTOS) Manifest.txt Manifest-fat.txt \
		micropeak micropeak-test micropeak-jdb macosx linux windows micropeak-windows.log \
		micropeak-windows.nsi *.desktop *.mdwn

EXTRA_DIST = $(desktop_file).in

$(desktop_file): $(desktop_file).in
	sed -e 's#%bindir%#@bindir@#' -e 's#%icondir%#$(datadir)/icons/hicolor/scalable/apps#' ${srcdir}/$(desktop_file).in > $@
	chmod +x $@

LINUX_DIST=MicroPeak-Linux-$(VERSION).tar.bz2
LINUX_SH=MicroPeak-Linux-$(VERSION).sh
MACOSX_DIST=MicroPeak-Mac-$(VERSION).dmg
WINDOWS_DIST=MicroPeak-Windows-$(VERSION_DASH).exe
MDWN=$(VERSION).mdwn
MDWNTMPL=mdwn.tmpl

MICROPEAK_DOC=$(top_srcdir)/doc/micropeak.pdf

DOC=$(MICROPEAK_DOC)

FAT_FILES=$(FATJAR) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(FREETTS_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS)

LINUX_FILES=$(FAT_FILES) $(LIBALTOS_LINUX) $(FIRMWARE) $(DOC) $(desktop_file).in $(LINUX_ICONS) $(LINUX_MIMETYPE)
LINUX_EXTRA=micropeak-fat $(desktop_file).in

MACOSX_DRIVER_0_URL=http://www.ftdichip.com/Drivers/VCP/MacOSX/FTDIUSBSerialDriver_v2_2_18.dmg
MACOSX_DRIVER_0=FTDI_v2_2_18.dmg

MACOSX_DRIVER_1_URL=http://www.ftdichip.com/Drivers/VCP/MacOSX/FTDIUSBSerialDriver_v2_3.dmg
MACOSX_DRIVER_1=FTDI_v2_3.dmg

MACOSX_DRIVERS=$(MACOSX_DRIVER_1) $(MACOSX_DRIVER_0)

MACOSX_INFO_PLIST=Info.plist
MACOSX_README=ReadMe-Mac.rtf
MACOSX_INSTALL=../altosui/install-macosx ../altosui/ask-pass
MACOSX_FILES=$(FAT_FILES) libaltos.dylib $(MACOSX_INFO_PLIST) $(MACOSX_DRIVERS) $(MACOSX_README) $(DOC) $(MACOSX_ICONS) $(MACOSX_INSTALL)

$(MACOSX_DRIVER_0):
	wget -O $@ $(MACOSX_DRIVER_0_URL)

$(MACOSX_DRIVER_1):
	wget -O $@ $(MACOSX_DRIVER_1_URL)

WINDOWS_DRIVER_URL=http://www.ftdichip.com/Drivers/CDM/CDM%20v2.12.00%20WHQL%20Certified.exe
WINDOWS_DRIVER=CDM_v2.12.00_WHQL_Certified.exe

$(WINDOWS_DRIVER):
	wget -O "$(WINDOWS_DRIVER)" "$(WINDOWS_DRIVER_URL)"

WINDOWS_FILES=$(FAT_FILES) altos.dll altos64.dll $(DOC) $(WINDOWS_ICONS) $(WINDOWS_DRIVER)

if FATINSTALL

FATTARGET=$(FATDIR)/MicroPeak/releases/$(VERSION)

LINUX_SH_TARGET=$(FATTARGET)/$(LINUX_SH)
MACOSX_DIST_TARGET=$(FATTARGET)/$(MACOSX_DIST)
WINDOWS_DIST_TARGET=$(FATTARGET)/$(WINDOWS_DIST)
MDWN_TARGET=$(FATDIR)/MicroPeak/releases/$(MDWN)
RELNOTES=release-notes-$(VERSION).html
RELNOTES_SRC=$(top_builddir)/doc/$(RELNOTES)
RELNOTES_TARGET=$(FATTARGET)/$(RELNOTES)

fat-install: $(LINUX_SH_TARGET) $(MACOSX_DIST_TARGET) $(WINDOWS_DIST_TARGET) $(MDWN_TARGET) $(RELNOTES_TARGET)

$(LINUX_SH_TARGET): $(LINUX_SH)
	mkdir -p $(FATTARGET)
	cp -p $< $@

$(MACOSX_DIST_TARGET): $(MACOSX_DIST)
	mkdir -p $(FATTARGET)
	cp -p $< $@

$(WINDOWS_DIST_TARGET): $(WINDOWS_DIST)
	mkdir -p $(FATTARGET)
	cp -p $< $@

$(MDWN_TARGET): $(MDWN)
	mkdir -p $(FATTARGET)
	cp -p $< $@

$(RELNOTES_TARGET): $(RELNOTES_SRC)
	mkdir -p $(FATTARGET)
	sh $(top_srcdir)/doc/install-html -d $(FATTARGET) $(RELNOTES_SRC)

endif

$(MDWN): $(MDWNTMPL)
	sed -e 's/%version%/$(VERSION)/g' -e 's/%version_dash%/$(VERSION_DASH)/g' $(MDWNTMPL) > $@

fat: $(LINUX_DIST) $(LINUX_SH) $(MACOSX_DIST) $(WINDOWS_DIST)


micropeak: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java  -Djava.library.path="$(altoslibdir)" -jar "$(micropeakdir)/micropeak.jar" "$$@"' >> $@
	chmod +x $@

micropeak-jdb: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec jdb -classpath "classes:./*:../libaltos:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar" -Djava.library.path="../libaltos/.libs" org.altusmetrum.micropeak.MicroPeak "$$@"' >> $@
	chmod +x $@

micropeak-test: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java -cp "./*:../libaltos/*:$(JCOMMON)/jcommon.jar:$(JFREECHART)/jfreechart.jar" -Djava.library.path="../libaltos/.libs" -jar micropeak.jar "$$@"' >> $@
	chmod +x $@

install-micropeakJAVA: micropeak.jar
	@$(NORMAL_INSTALL)
	test -z "$(micropeakdir)" || $(MKDIR_P) "$(DESTDIR)$(micropeakdir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(micropeakdir)/micropeak.jar'"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(micropeakdir)"

$(JAR): classmicropeak.stamp Manifest.txt $(JAVA_ICONS) $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS)
	jar cfm $@ Manifest.txt \
		$(ICONJAR) \
		-C classes org \
		-C ../libaltos libaltosJNI
if STRIP_NONDETERMINISM
	$(STRIP_NONDETERMINISM) $@
endif

$(FATJAR): classmicropeak.stamp Manifest-fat.txt $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(JFREECHART_CLASS) $(JCOMMON_CLASS) $(JAVA_ICONS)
	jar cfm $@ Manifest-fat.txt \
		$(ICONJAR) \
		-C classes org \
		-C ../libaltos libaltosJNI
if STRIP_NONDETERMINISM
	$(STRIP_NONDETERMINISM) $@
endif

classaltosui.stamp: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS)

libaltos.so: build-libaltos
	-rm -f "$@"
	$(LN_S) ../libaltos/.libs/"$@" .

libaltos_i686.so: build-libaltos
	-rm -f "$@"
	$(LN_S) ../libaltos/.libs/"$@" .

libaltos_amd64.so: build-libaltos
	-rm -f "$@"
	$(LN_S) ../libaltos/.libs/"$@" .

libaltos_aarch64.so: build-libaltos
	-rm -f "$@"
	$(LN_S) ../libaltos/.libs/"$@" .

libaltos_armel.so: build-libaltos
	-rm -f "$@"
	$(LN_S) ../libaltos/.libs/"$@" .

libaltos_armhf.so: build-libaltos
	-rm -f "$@"
	$(LN_S) ../libaltos/.libs/"$@" .

libaltos.dylib:
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

altos.dll: ../libaltos/altos.dll
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

altos64.dll: ../libaltos/altos64.dll
	-rm -f "$@"
	$(LN_S) ../libaltos/"$@" .

../libaltos/.libs/libaltos64.so: ../libaltos/.libs/libaltos32.so

../libaltos/.libs/libaltos32.so: build-libaltos

../libaltos/.libs/libaltos.so: build-libaltos

../libaltos/altos.dll: build-altos-dll

../libaltos/altos64.dll: build-altos64-dll

build-libaltos:
	+cd ../libaltos && make libaltos.la

build-altos-dll:
	+cd ../libaltos && make altos.dll

build-altos64-dll:
	+cd ../libaltos && make altos64.dll

$(ALTOSLIB_CLASS):
	-rm -f "$@"
	$(LN_S) ../altoslib/"$@" .

$(ALTOSUILIB_CLASS):
	-rm -f "$@"
	$(LN_S) ../altosuilib/"$@" .

$(JFREECHART_CLASS):
	-rm -f "$@"
	$(LN_S) "$(JFREECHART)"/"$@" .

$(JCOMMON_CLASS):
	-rm -f "$@"
	$(LN_S) "$(JCOMMON)"/"$@" .

$(LINUX_DIST): $(LINUX_FILES) $(LINUX_EXTRA)
	-rm -f $@
	-rm -rf linux
	mkdir -p linux/MicroPeak
	cp -p $(LINUX_FILES) linux/MicroPeak
	cp -p micropeak-fat linux/MicroPeak/micropeak
	chmod +x linux/MicroPeak/micropeak
	tar cjf $@ -C linux MicroPeak

$(LINUX_SH): $(LINUX_DIST) $(srcdir)/../altosui/linux-install.sh
	sed 's/AltOS/MicroPeak/g' $(srcdir)/../altosui/linux-install.sh | cat - $(LINUX_DIST) > $@
	chmod +x $@

$(MACOSX_DIST): $(MACOSX_FILES)
	-rm -f $@
	-rm -rf macosx
	mkdir macosx
	cp -a MicroPeak.app macosx/
	cp -a $(MACOSX_README) macosx/ReadMe-MicroPeak.rtf
	cp -a $(MACOSX_INSTALL) macosx
	mkdir -p macosx/Doc
	cp -a $(DOC) macosx/Doc
	cp -p Info.plist macosx/MicroPeak.app/Contents
	cp -p $(MACOSX_DRIVERS) macosx
	mkdir -p macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(MACOSX_ICONS) macosx/MicroPeak.app/Contents/Resources
	cp -p $(FATJAR) macosx/MicroPeak.app/Contents/Resources/Java/micropeak.jar
	cp -p libaltos.dylib macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(ALTOSLIB_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(ALTOSUILIB_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(JFREECHART_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	cp -p $(JCOMMON_CLASS) macosx/MicroPeak.app/Contents/Resources/Java
	genisoimage -D -V MicroPeak-$(VERSION) -no-pad -r -apple -o $@ macosx

$(WINDOWS_DIST): $(WINDOWS_FILES) micropeak-windows.nsi
	-rm -f $@
	makensis -Omicropeak-windows.log "-XOutFile $@" "-DVERSION=$(VERSION)" micropeak-windows.nsi || (cat micropeak-windows.log && exit 1)

Manifest.txt: Makefile
	echo 'Main-Class: org.altusmetrum.micropeak.MicroPeak' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) $(JCOMMON)/jcommon.jar $(JFREECHART)/jfreechart.jar" >> $@

Manifest-fat.txt:
	echo 'Main-Class: org.altusmetrum.micropeak.MicroPeak' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS) jcommon.jar jfreechart.jar" >> $@

