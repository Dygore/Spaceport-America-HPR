= Release Notes for Version 1.3.2
include::release-head.adoc[]
:doctype: article

    Version 1.3.2 is a minor release. It includes small bug fixes for
    the TeleMega flight software and AltosUI ground station

	== AltOS

		AltOS fixes:

      		* On TeleMega, limit number of logged GPS status
		  information to 12 satellites. That's all there is
		  room for in the log structure.

      		* Improve APRS behavior. Remembers last known GPS
		  position and keeps sending that if we lose GPS
		  lock. Marks locked/unlocked by sending L/U in the
		  APRS comment field along with the number of sats in
		  view and voltages.

	== AltosUI Application

		AltosUI fixes:

      		* If the TeleMega flight firmware reports that it has
		  logged information about more than 12 satellites,
		  don't believe it as the log only holds 12 satellite
		  records.

      		* Track the maximum height as computed from GPS
		  altitude data and report that in the flight summary
		  data.

      		* Use letters (A, B, C, D) for alternate pyro channel
		  names instead of numbers (0, 1, 2, 3) in the Fire
		  Igniter dialog.
