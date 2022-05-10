= Release Notes for Version 1.2
include::release-head.adoc[]
:doctype: article

	Version 1.2 is a major release. It adds support for MicroPeak
	and the MicroPeak USB adapter.

	== AltOS

		AltOS New Features:

		* Add MicroPeak support. This includes support for the
		  ATtiny85 processor and adaptations to the core code
		  to allow for devices too small to run the
		  multi-tasking scheduler.

	== AltosUI and MicroPeak Application

		New Features:

		* Added MicroPeak application

		AltosUI and MicroPeak fixes:

		* Distribute Mac OS X packages in disk image ('.dmg')
		  format to greatly simplify installation.

		* Provide version numbers for the shared Java
		  libraries to ensure that upgrades work properly, and
		  to allow for multiple Altus Metrum software packages
		  to be installed in the same directory at the same
		  time.
