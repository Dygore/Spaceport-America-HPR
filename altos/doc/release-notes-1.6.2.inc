= Release Notes for Version 1.6.2
include::release-head.adoc[]
:doctype: article

	Version 1.6.2 includes support for our updated TeleMega v2.0
	product and bug fixes in in the flight software for all our boards
	and ground station interfaces.

	== AltOS

	AltOS New Features:

	* Add support for TeleMega v2.0 boards.

	* Add PWM servo driver. There's no higher level code using
          this yet, but the driver allows testing of the TeleMega v2.0
          servo output connector.

	AltOS Fixes:

	* Slow down telemetry packets to allow receiver to keep
          up.

	== AltosUI and TeleGPS Applications

	AltosUI and TeleGPS Fixes:

	* Fix post-flight orientation computation when processing
	  TeleMega and EasyMega eeprom data files.

	* Capture complete eeprom data even when there are invalid
          entries in the data. This keeps reading eeprom contents and
          writing the associated .eeprom file when an error is detected.

	== Documentation

	We spent a bunch of time trying to improve our documentation

	* HTML versions now have a table of contents on the left side.

	* EasyMini now has its own shorter manual.

	* Provide links between sections in each document.

	* Lots of minor rewriting and restructuring to avoid
          duplication of information
