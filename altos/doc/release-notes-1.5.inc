= Release Notes for Version 1.5
include::release-head.adoc[]

	Version 1.5 is a major release. It includes support for our new
	EasyMega product, new features and bug fixes in in the flight
	software for all our boards and the AltosUI ground station

	== AltOS

	AltOS New Features

	* Add support for EasyMega boards.

	* Make the APRS SSID be configurable. This lets you track
	  different rockets on the same receiver without getting
	  things mixed up.

	* Report extra pyro channel continuity state on EasyMega and
	  TeleMega via the beeper. This lets you easily verify flight
	  readiness on these boards after powering up the electronics
	  on the rail.

	* Add lower telemetry data rates (2400 and 9600 bps) to
	  increase telemetry radio range. This reduces the amount of
	  data received as well as increasing battery consumption in
	  the transmitter.

	* Change TeleGPS to have only a single log, and append new
	  data to it rather than using seperate per-flight logs. This
	  avoids accidentally filling up log storage by turning
	  TeleGPS on/off several times.

	AltOS Fixes

	* Increase the maximum range for altitude values from +/-32767m
	  to +/-2147483647m, allowing the flight computers to function
	  correctly above the 32km level.

	* Continuously test pyro firing conditions during delay stage,
	  inhibiting the pyro channel if the test fails. This prevents
	  firing pyro charges where the conditions were good before
	  the delay, but become bad before the delay expires.

	* Allow negative numbers in pyro configuration values. This
	  lets you specify things like descending speed or
	  deceleration.

	== AltosUI and TeleGPS Applications

	AltosUI and TeleGPS New Features

	* Support telemetry baud rate selection. Adds menus to
	  the flight monitoring and configuration for baud rate
	  selection.

	* Support APRS SSID configuration.

	* Integrate with file managers. This provides icons for all of
	  our file types and associates our application with the files
	  so that using a file manager to open a AltOS data file
	  results in launching our application.

	AltosUI Fixes

	* Make the 'Graph' button on the landed tab work again.

	* Make tests for Java on Windows a bit smarter, and also
	  provide the user with the option to skip installing Java for
	  cases where we just can't figure out what version is installed.
