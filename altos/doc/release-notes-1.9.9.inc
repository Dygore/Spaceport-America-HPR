= Release Notes for Version 1.9.9
include::release-head.adoc[]
:doctype: article

	Version 1.9.9

	This release contains a critical bug fix for a problem
	introduced in version 1.9.8 for TeleMega and EasyMega
	boards. This problem occurs when using the stored
	configuration from 1.9.7 or earlier.

	If you are running 1.9.8 or are upgrading from 1.9.8 on any
	version of TeleMega or EasyMega, you must reconfigure all pyro
	channels, recalibrate accelerometers, reset the APRS interval,
	adjust the beep tone and reset the pyro time.

	== AltOS

	* Fix EasyMega and TeleMega upgrade process from 1.9.7 or
	  earlier.  1.9.8 introduced larger delay values, which
	  required modifying the configuration in-place, and the 1.9.8
	  version had a flaw which broke the pyro channel config and
	  all of the config values beyond that in memory, including
	  APRS interval, IMU accel calibation, beep tone and pyro
	  time.

	* Fix TeleMega v5.0 mag sensor driver. This driver was quite
	  broken due to developing it in the presence of the magnetic
	  beeper on the board. Because of that beeper, the values this
	  sensor records are not accurate. Fortunately, they are not
	  used for controlling the flight.

	== AltosUI

	* Parse TeleMega v5.0 log files. A missing check in the code
          meant that the TeleMega v5.0 log files would cause an error
          when attempting to load them. Logs saved with AltosUI
          1.9.8 were not affected, only the presentation of the data
          was broken.
