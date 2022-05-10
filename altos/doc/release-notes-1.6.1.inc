= Release Notes for Version 1.6.1
include::release-head.adoc[]
:doctype: article

	Version 1.6.1 includes support for our updated TeleBT v3.0
	product and bug fixes in in the flight software for all our boards
	and ground station interfaces.

	== AltOS

	AltOS New Features:

	* Add support for TeleBT v3.0 boards.

	* Add support for uncompressed APRS data, providing support
	  for older APRS receivers. Uncompressed APRS data is less
	  precise, takes more bandwidth and doesn't have integrated
	  altitude data.

	AltOS Fixes:

	* Make TeleDongle and TeleBT more tolerant of data rate
	  variations from transmitting devices.

	== AltosUI and TeleGPS Applications

	AltosUI and TeleGPS New Features:

	* Add map to Monitor Idle display. It's nice to be able to
	  verify that maps are working, instead of needing to use
	  Monitor Flight.

	AltosUI and TeleGPS Fixes:

	* Fix frequency configuration to round values instead of
	  truncate them, avoiding a common 1kHz error in the setting.

	* Turn the Windows stub into a more useful program that can
	  launch the application with parameters so that file manager
	  icons work more reliably.

	* Force KML export to use a C locale so that numbers are
	  formatted with '.' instead of ',' for a decimal separator in
	  non-US locales.

	* Preload map tiles based on distance rather than number of
	  tiles; this means you get the same resolution covering the
	  entire area, rather than having high resolution near the
	  center and low resolution further away.

	* Allow configuration of frequency and callsign in Monitor
	  Idle mode.

	* Fix layout weirdness when resizing windows on
	  Windows. Windows shouldn't have giant blank spaces around
	  the useful content anymore.

	* Fix layout weirdness when resizing windows on
	  Windows. Windows shouldn't have giant blank spaces around
	  the useful content anymore.

	* Use a longer filter for descent speed values. This should
	  provide something more useful on the display, although it
	  will take longer to respond to changes now.

	* Make Replay Flight run in realtime again. It had been set to
	  run at 10x speed by mistake.

	== AltosDroid

	AltosDroid New Features:

	* Add offline map support using mapping code from AltosUI.

	* Support TeleDongle (and TeleBT via USB) on devices
	  supporting USB On-The-Go.

	* Display additional TeleMega pyro channel status in Pad tab.

	* Switch between metric and imperial units.

	* Monitor TeleBT battery voltage.

	* Track multiple devices at the same time, selecting between
	  them with a menu or using the map.

	* Add hybrid, satellite and terrain map types.

	AltosDroid Fixes:

	* Use standard Android display conventions so that a menu
	  button is available in the application title bar.

	* Adjust layout to work on large and small screens; shrinking
	  the go/no-go lights in smaller environments to try and make
	  everything visible.

	* Make voice announcements depend on current tab.

	* Compute adjustment to current travel direction while in
	  motion towards rocket.
