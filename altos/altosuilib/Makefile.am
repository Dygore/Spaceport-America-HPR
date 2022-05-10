AM_JAVACFLAGS=$(JAVAC_VERSION_FLAGS) -encoding UTF-8 -Xlint:deprecation -Xlint:unchecked

JAVAROOT=bin

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="$(JAVAROOT):../altoslib/*:../libaltos:/usr/share/java/*"

SRC=.

altosuilibdir = $(datadir)/java

altosuilib_JAVA = \
	AltosDevice.java \
	AltosDeviceDialog.java \
	AltosPositionListener.java \
	AltosUIAccelCal.java \
	AltosUIConfigure.java \
	AltosUIAxis.java \
	AltosUIDataMissing.java \
	AltosUIDataPoint.java \
	AltosUIDataSet.java \
	AltosUIGrapher.java \
	AltosUIDialog.java \
	AltosUIEnable.java \
	AltosUIFrame.java \
	AltosUILib.java \
	AltosUIListener.java \
	AltosUIMarker.java \
	AltosUIPreferencesBackend.java \
	AltosUIPreferences.java \
	AltosUIFlightSeries.java \
	AltosUIGraph.java \
	AltosGraph.java \
	AltosShapeListener.java \
	AltosUSBDevice.java \
	AltosVoice.java \
	AltosDisplayThread.java \
	AltosDeviceUIDialog.java \
	AltosSerial.java \
	AltosSerialInUseException.java \
	AltosConfigFreqUI.java \
	AltosScanUI.java \
	AltosEepromDelete.java \
	AltosEepromGrapher.java \
	AltosEepromManage.java \
	AltosEepromMonitorUI.java \
	AltosEepromSelect.java \
	AltosCSVUI.java \
	AltosDataChooser.java \
	AltosLights.java \
	AltosLed.java \
	AltosFlashUI.java \
	AltosRomconfigUI.java \
	AltosInfoTable.java \
	AltosFlightInfoTableModel.java \
	AltosFlightStatsTable.java \
	AltosBTDevice.java \
	AltosBTDeviceIterator.java \
	AltosBTManage.java \
	AltosBTKnown.java \
	AltosUILineStyle.java \
	AltosUIMap.java \
	AltosUIMapPreload.java \
	AltosUIFlightTab.java \
	AltosUIIndicator.java \
	AltosUIUnitsIndicator.java \
	AltosUIVoltageIndicator.java \
	AltosUIFreqList.java \
	AltosUITelemetryList.java \
	AltosUIRateList.java \
	AltosUIImage.java \
	AltosUITimeSeries.java \
	OSXAdapter.java

JAR=altosuilib_$(ALTOSUILIB_VERSION).jar

# Icons
ICONDIR=$(top_srcdir)/icon

ICONS=	$(ICONDIR)/redled.png $(ICONDIR)/redoff.png \
	$(ICONDIR)/greenled.png $(ICONDIR)/greenoff.png \
	$(ICONDIR)/grayon.png $(ICONDIR)/grayled.png

# icon base names for jar
ICONJAR= -C $(ICONDIR) redled.png -C $(ICONDIR) redoff.png \
	-C $(ICONDIR) greenled.png -C $(ICONDIR) greenoff.png \
	-C $(ICONDIR) grayon.png -C $(ICONDIR) grayled.png

all-local: $(JAR)

clean-local:
	-rm -rf $(JAVAROOT) altosuilib_*.jar

install-altosuilibJAVA: $(JAR)
	@$(NORMAL_INSTALL)
	test -z "$(altosuilibdir)" || $(MKDIR_P) "$(DESTDIR)$(altosuilibdir)"
	echo " $(INSTALL_DATA)" "$(JAR)" "'$(DESTDIR)$(altosuilibdir)/$(JAR)"; \
	$(INSTALL_DATA) "$(JAR)" "$(DESTDIR)$(altosuilibdir)"

$(JAVAROOT):
	mkdir -p $(JAVAROOT)

$(JAR): classaltosuilib.stamp $(ICONS)
	jar cf $@ $(ICONJAR) -C $(JAVAROOT) .
if STRIP_NONDETERMINISM
	$(STRIP_NONDETERMINISM) $@
endif
