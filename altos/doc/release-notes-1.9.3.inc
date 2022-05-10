= Release Notes for Version 1.9.3
include::release-head.adoc[]
:doctype: article

	Version 1.9.3

	== AltOS

	* Add APRS offset. Allows multiple APRS transmitters to coordinate when
	  transmission occurs to allow them to share a frequency.

	* Fix max log size. Flight computer storage is erased in 64kB chunks. Adjust max
	  log size to be a multiple of this size.

	* Check flight erasing more carefully. Handle interrupting erasing in the
	  middle.

	* Add EasyTimer support.

	== AltosUI, TeleGPS, MicroPeak

	* Add configuration support for APRS offset.

	* Adjust flight log sizes to be a multiple of the flight
          computer erase block size.

	* Report barometric pad altitude in Pad tab for TeleMini.
