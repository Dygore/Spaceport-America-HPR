= Release Notes for Version 1.3.1
include::release-head.adoc[]
:doctype: article

	Version 1.3.1 is a minor release. It improves support for
	TeleMega, TeleMetrum v2.0, TeleMini v2.0 and EasyMini.

	== AltOS

		AltOS new features:

      		* Improved APRS mode. Now uses compressed position
		  format for smaller data size, improved precision and
		  to include altitude data as well as latitude and
		  longitude. Also added battery and pyro voltage
		  reports in the APRS comment field so you can confirm
		  that the unit is ready for launch.

		AltOS fixes:
    
      		* Improve sensor boot code. If sensors fail to
		  self-test, the device will still boot up and check
		  for pad/idle modes. If in idle mode, the device will
		  warn the user with a distinct beep, if in Pad mode,
		  the unit will operate as best it can. Also, the
		  Z-axis accelerometer now uses the factory
		  calibration values instead of re-calibrating on the
		  pad each time. This avoids accidental boost detect
		  when moving the device around while in Pad mode.
      
      		* Fix antenna-down mode accelerometer
		  configuration. Antenna down mode wasn't working
		  because the accelerometer calibration values were
		  getting re-computed incorrectly in inverted mode.
      
	== AltosUI Application

		AltosUI new features:
    
      		* Display additional TeleMega sensor values in real
		  units. Make all of these values available for
		  plotting. Display TeleMega orientation value in the
		  Ascent and Table tabs.

      
      		* Support additional TeleMega pyro channels in the
		  Fire Igniter dialog. This lets you do remote testing
		  of all of the channels, rather than just Apogee and
		  Main.

		AltosUI fixes:
      
      		* Limit data rate when downloading satellite images
		  from Google to make sure we stay within their limits
		  so that all of the map tiles download successfully.
