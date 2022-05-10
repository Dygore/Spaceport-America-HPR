AM_JAVACFLAGS=$(JAVAC_VERSION_FLAGS) -encoding UTF-8 -Xlint:deprecation -Xlint:unchecked

JAVAROOT=bin

VERSION=1

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH=bin

SRC=.

altoslibdir = $(datadir)/java
record_files = \
	AltosEepromRecord.java \
	AltosEepromTeleScience.java \
	AltosRecordCompanion.java \
	AltosRecordIterable.java \
	AltosOrderedRecord.java \
	AltosOrderedMegaRecord.java \
	AltosOrderedMiniRecord.java \
	AltosRecord.java \
	AltosRecordNone.java \
	AltosRecordTM.java \
	AltosRecordMM.java \
	AltosRecordMini.java


altoslib_JAVA = \
	AltosLib.java \
	AltosAccelCal.java \
	AltosAccelCalListener.java \
	AltosAdxl375.java \
	AltosCalData.java \
	AltosCompanion.java \
	AltosConfigData.java \
	AltosConfigDataException.java \
	AltosConfigValues.java \
	AltosConvert.java \
	AltosCRCException.java \
	AltosCSV.java \
	AltosDebug.java \
	AltosEeprom.java \
	AltosRecordSet.java \
	AltosEepromRecord.java \
	AltosEepromRecordFull.java \
	AltosEepromRecordTiny.java \
	AltosEepromRecordMega.java \
	AltosEepromRecordMetrum.java \
	AltosEepromRecordMini.java \
	AltosEepromRecordGps.java \
	AltosEepromRecordFireTwo.java \
	AltosEepromRecordMicroPeak2.java \
	AltosEepromRecordMotor.java \
	AltosEepromRecordSet.java \
	AltosEepromChunk.java \
	AltosEepromDownload.java \
	AltosEepromMonitor.java \
	AltosEepromFile.java \
	AltosEepromList.java \
	AltosEepromLog.java \
	AltosFile.java \
	AltosFilterListener.java \
	AltosFlash.java \
	AltosFlashListener.java \
	AltosDataListener.java \
	AltosDataProvider.java \
	AltosFlightSeries.java \
	AltosFlightReader.java \
	AltosFlightStats.java \
	AltosForce.java \
	AltosFrequency.java \
	AltosGPS.java \
	AltosGPSTimeValue.java \
	AltosGPSSat.java \
	AltosGauss.java \
	AltosGreatCircle.java \
	AltosHexfile.java \
	AltosHexsym.java \
	AltosIdle.java \
	AltosIdleFetch.java \
	AltosIdleMonitor.java \
	AltosIdleMonitorListener.java \
	AltosIdleReader.java \
	AltosIgnite.java \
	AltosIMU.java \
	AltosKML.java \
	AltosLine.java \
	AltosLink.java \
	AltosListenerState.java \
	AltosLog.java \
	AltosLogTrace.java \
	AltosMag.java \
	AltosMma655x.java \
	AltosMs5607.java \
	AltosNoSymbol.java \
	AltosOrient.java \
	AltosParse.java \
	AltosPressure.java \
	AltosPresTemp.java \
	AltosPreferences.java \
	AltosPreferencesBackend.java \
	AltosProgrammer.java \
	AltosPyroName.java \
	AltosReplayReader.java \
	AltosRomconfig.java \
	AltosSavedState.java \
	AltosSelfFlash.java \
	AltosUsbId.java \
	AltosSensorMM.java \
	AltosSensorEMini.java \
	AltosSensorEasyTimer1.java \
	AltosSensorTM.java \
	AltosSensorTMini2.java \
	AltosSensorTMini3.java \
	AltosSensorMega.java \
	AltosSensorMetrum.java \
	AltosSensorTGPS1.java \
	AltosSensorTGPS2.java \
	AltosSensorEasyMotor2.java \
	AltosState.java \
	AltosStateName.java \
	AltosStringInputStream.java \
	AltosTelemetry.java \
	AltosTelemetryConfiguration.java \
	AltosTelemetryCompanion.java \
	AltosTelemetryFile.java \
	AltosTelemetryIterable.java \
	AltosTelemetryLegacy.java \
	AltosTelemetryLocation.java \
	AltosTelemetryMap.java \
	AltosTelemetryMegaSensor.java \
	AltosTelemetryMegaNorm.java \
	AltosTelemetryMegaData.java \
	AltosTelemetryMini2.java \
	AltosTelemetryMini3.java \
	AltosTelemetryMetrumSensor.java \
	AltosTelemetryMetrumData.java \
	AltosTelemetryReader.java \
	AltosTelemetryRaw.java \
	AltosTelemetrySensor.java \
	AltosTelemetrySatellite.java \
	AltosTelemetryStandard.java \
	AltosTime.java \
	AltosTimeSeries.java \
	AltosTimeValue.java \
	AltosUnitsListener.java \
	AltosUnknownProduct.java \
	AltosMs5607.java \
	AltosIMU.java \
	AltosMag.java \
	AltosUnits.java \
	AltosUnitsRange.java \
	AltosDistance.java \
	AltosHeight.java \
	AltosSpeed.java \
	AltosTemperature.java \
	AltosAccel.java \
	AltosVoltage.java \
	AltosLocation.java \
	AltosLatitude.java \
	AltosLongitude.java \
	AltosRotationRate.java \
	AltosPyro.java \
	AltosWriter.java \
	AltosQuaternion.java \
	AltosRotation.java \
	AltosImage.java \
	AltosLatLon.java \
	AltosMap.java \
	AltosMapCache.java \
	AltosMapCacheListener.java \
	AltosMapInterface.java \
	AltosMapLine.java \
	AltosMapMark.java \
	AltosMapPath.java \
	AltosMapPathPoint.java \
	AltosMapRectangle.java \
	AltosMapStore.java \
	AltosMapStoreListener.java \
	AltosMapTile.java \
	AltosMapTileListener.java \
	AltosMapTransform.java \
	AltosMapZoomListener.java \
	AltosPointDouble.java \
	AltosPointInt.java \
	AltosRectangle.java \
	AltosFlightDisplay.java \
	AltosFontListener.java \
	AltosLaunchSite.java \
	AltosLaunchSiteListener.java \
	AltosLaunchSites.java \
	AltosMapLoaderListener.java \
	AltosMapLoader.java \
	AltosMapTypeListener.java \
	AltosJson.java \
	AltosVersion.java \
	AltosUnitInfo.java \
	AltosUnitInfoListener.java

JAR=altoslib_$(ALTOSLIB_VERSION).jar

all-local: $(JAR)

clean-local:
	-rm -rf $(JAVAROOT) altoslib_*.jar

install-altoslibJAVA: $(JAR)
	@$(NORMAL_INSTALL)
	test -z "$(altoslibdir)" || $(MKDIR_P) "$(DESTDIR)$(altoslibdir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(altoslibdir)/$(JAR)"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(altoslibdir)"

bin:
	mkdir -p bin

$(JAR): classaltoslib.stamp
	jar cf $@ -C bin org
if STRIP_NONDETERMINISM
	$(STRIP_NONDETERMINISM) $@
endif
