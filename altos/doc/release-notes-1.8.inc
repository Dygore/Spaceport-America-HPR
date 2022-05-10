= Release Notes for Version 1.8
include::release-head.adoc[]
:doctype: article

	Version 1.8 includes support for our new TeleBT v4.0 ground
	station, updates for data analysis in our ground station
	software and bug fixes in in the flight software for all our
	boards and ground station interfaces.

	== AltOS

	AltOS New Features

	* Add support for TeleBT v4.0 boards.

	== AltosUI and TeleGPS Applications

	AltosUI New Features

	* Add support for TeleBT v4.0 hardware

	* Rewrite graphing and export functions. This code now handles
          each data series separately so that graphs are drawn
          correctly. Smoothing now uses a filter that looks both
          forward and backwards in time to make computed speed and
          acceleration data more accurate.

	AltosUI Bug Fixes

	* Correct axis labeling of magnetic sensor in TeleMega and
          EasyMega. The Y and Z axes were flipped.


