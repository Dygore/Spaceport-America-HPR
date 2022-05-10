= Release Notes for Version 1.7
include::release-head.adoc[]
:doctype: article

	Version 1.7 includes support for our new TeleMini v3.0
	flight computer and bug fixes in in the flight software for all our boards
	and ground station interfaces.

	== AltOS

	AltOS New Features

	* Add support for TeleMini v3.0 boards.

	AltOS Fixes

	* Fix interrupt priorities on STM32L processors. Run timer
          interrupt at lowest priority so that device interrupts get
          serviced first.

	== AltosUI and TeleGPS Applications

	AltosUI New Features

	* Add support for TeleMini v3.0 hardware
