= Release Notes for Version 1.9.1
include::release-head.adoc[]
:doctype: article

	Version 1.9.1

	== AltOS

	* Add support for TeleMetrum v3.0

	* Fix accel cal value when changing EasyMega v2.0 pad orientation

	* Correct EasyMega v2.0 magnetometer data axes

	* Fix EasyMega v2.0 idle monitor display of IMU data

	* Report all sensor failures at power-up time.
	
	== AltosUI, TeleGPS, MicroPeak

	* Display error message when attempting to graph unknown format file.

	* Make it possible to disable APRS once enabled.

	* Display some data for point near cursor in map tab.

	* Support upgrading devices from pre-1.8 firmware

	* Wait for Windows to prepare new devices during firmware
	  upgrade. This should avoid the “COMxx: open failed” message.
