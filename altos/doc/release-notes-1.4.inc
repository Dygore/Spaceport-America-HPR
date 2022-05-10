= Release Notes for Version 1.4
include::release-head.adoc[]
:doctype: article

	Version 1.4 is a major release. It includes support for our new
	TeleGPS product, new features and bug fixes in in the flight
	software for all our boards and the AltosUI ground station

	== AltOS

		AltOS new features:

		* Add support for TeleGPS boards.

		* Make the beeper tone configurable, making it
		  possible to distinguish between two Altus Metrum
		  products in the same ebay.

		* Make the firing time for extra pyro channels
		  configurable, allowing longer (or shorter) than the
		  default 50ms.  Only relevant for TeleMega at this
		  time.

		AltOS fixes:

		* Replace the 'dit dit dit' tones at startup with the
		  current battery voltage, measured in tenths of a
		  volt. This lets you check the battery voltage
		  without needing telemetry, which is especially
		  useful on EasyMini.

		* Change state beeping to "Farnsworth spacing", which
		  means they're quite a bit faster than before, and so
		  they take less time to send.

		* Fix bug preventing the selection of the 'Flight
		  State After' mode in pyro configuration.

		* Fix bug where erasing flights would reset the flight
		  number to 2 on TeleMega and TeleMetrum v2.

		* Fix u-Blox GPS driver to mark course and speed data
		  as being present.

	== AltosUI Application

		AltosUI new features:

		* Add zooming and new content types (terrain and road
		  maps) to map view. Change map storage format from
		  PNG to Jpeg, which saves a huge amount of disk
		  space. You will need to re-download all of your
		  pre-loaded map images.

		* Add a distance measuring device to the maps
		  view. Select this by using any button other than the
		  left one, or by pressing shift or control on the
		  keyboard while using the left button.

		* Add new 'Ignitor' tab to the flight monitor display
		  for TeleMega's extra ignitors.

		* Add additional ignitor firing marks and voltages to
		  the graph so you can see when the ignitors fired,
		  along with the ignitor voltages.

		* Add GPS course, ground speed and climb rate as
		  optional graph elements.

		AltosUI fixes:

		* When flashing new firmware, re-try opening the
		  device as sometimes it takes a while for the
		  underlying operating system to recognize that the
		  device has rebooted in preparation for the flashing
		  operation.

		* Hide Tilt Angle in ascent tab for devices that don't
                  have a gyro.

		* Increase the width of data lines in the graphs to
		  make them easier to read.

		* Filter out speed and acceleration spikes caused by
		  ejection charge firing when computing the maximum
		  values. This provides a more accurate reading of
		  those maximums.

		* Fix EasyMini voltage displays. Early EasyMini
		  prototypes used a 3.0V regulator, and AltosUI still
		  used that value as the basis of the
		  computation. Production EasyMini boards have always
		  shipped with a 3.3V regulator. Also, purple EasyMini
		  boards sensed the battery voltage past the blocking
		  diode, resulting in a drop of about 150mV from the
		  true battery voltage. Compensate for that when
		  displaying the value.

		* Display error message when trying to configure
		  maximum flight log size while the flight computer
		  still has flight data stored.

		* Handle TeleMetrum and TeleMini eeprom files
		  generated with pre-1.0 firmware. Those ancient
		  versions didn't report the log format, so just use
		  the product name instead.

	== TeleGPS Application

		* New application designed for use with TeleGPS boards.

		* Shares code with AltosUI, mostly just trimmed down
		  to focus on TeleGPS-related functions.

	== Documentation

		Documentation changes:

		* Re-create the drill template images; they should
		  print correctly from Firefox at least. Ship these as
		  individual PDF files so they're easy to print.

		* Add a description of the 'Apogee Lockout' setting,
		  which prevents the apogee charge from firing for a
		  configurable amount of time after boost.
