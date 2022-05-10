= Release Notes for Version 1.8.3
include::release-head.adoc[]

	Version 1.8.3 includes support for TeleMega version 3.0 along
	with two important flight computer fixes. This version also
	changes KML export data to make Tripoli Record reporting
	better and some updates to graph presentation and data
	downloading.

	== AltOS

	=== AltOS New Features

	* Support for TeleMega version 3.0 hardware.

	=== AltOS Bug Fixes

	* Ground testing EasyMega and TeleMega additional pyro
          channels could result in a sticky 'fired' status which would
          prevent these channels from firing on future flights.

	* Corrupted flight log records could prevent future flights
          from capturing log data.

	* Fixed saving of pyro configuration that ended with
          'Descending'.

	== AltosUI and TeleGPS Applications

	=== AltosUI New Features

	* Support for TeleMega version 3.0.

	* Graph lines have improved appearance to make them easier to
	  distinguish. Markers may be placed at data points to show
	  captured recorded data values.

	* Graphing offers the ability to adjust the smoothing of
          computed speed and acceleration data.

	* The download dialog now offers to graph new flights, checks
          for existing files to avoid overwriting data and reports if
          there are checksum errors in the downloaded data.

	=== AltosUI/TeleGPS Bug Fixes

	* Restore TeleGPS tracking behavior.

	* Display flight computer call sign and serial number in
          Monitor Idle mode instead of ground station values.

	=== AltosUI and TeleGPS Changes

	* KML export now reports both barometric and GPS altitude data
          to make it more useful for Tripoli record reporting.

	* CSV export now includes TeleMega/EasyMega pyro voltages and
          tilt angle.
