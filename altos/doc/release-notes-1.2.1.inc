= Release Notes for Version 1.2.1
include::release-head.adoc[]
:doctype: article

	Version 1.2.1 is a minor release. It adds support for TeleBT and
	the AltosDroid application, provides several new features in
	AltosUI and fixes some bugs in the AltOS firmware.

	== AltOS

		AltOS new features:

		* Add support for TeleBT
      
		AltOS fixes:

		* In TeleMini recovery mode (when booted with the
		  outer two debug pins connected together), the radio
		  parameters are also set back to defaults
		  (434.550MHz, N0CALL, factory radio cal).
      
		* Correct Kalman filter model error covariance
		  matrix. The values used previously assumed
		  continuous measurements instead of discrete
		  measurements.
      
		* Fix some bugs in the USB driver for TeleMetrum and
		  TeleDongle that affected Windows users.
      
		* Adjusted the automatic gain control parameters that
		  affect receive performance for TeleDongle. Field
		  tests indicate that this may improve receive
		  performance somewhat.

	== AltosUI Application

		AltosUI application new features:

		* Make the initial position of the AltosUI top level
		  window configurable. Along with this change, the
		  other windows will pop up at 'sensible' places now,
		  instead of on top of one another.
      
		* Add GPS data and a map to the graph window. This
		  lets you see a complete summary of the flight
		  without needing to 'replay' the whole thing.
      
		AltosUI application fixes:

		* Handle missing GPS lock in 'Descent'
		  tab. Previously, if the GPS position of the pad was
		  unknown, an exception would be raised, breaking the
		  Descent tab contents.
      
		* Improve the graph, adding tool-tips to show values
		  near the cursor and making the displayed set of
		  values configurable, adding all of the flight data
		  as options while leaving the default settings alone
		  so that the graph starts by showing height, speed
		  and acceleration.
      
		* Add callsign to Monitor idle window and connecting
		  dialogs. This makes it clear which callsign is being
		  used so that the operator will be aware that it must
		  match the flight computer value or no communication
		  will work.
      
		* When downloading flight data, display the block
		  number so that the user has some sense of
		  progress. Unfortunately, we don't know how many
		  blocks will need to be downloaded, but at least it
		  isn't just sitting there doing nothing for a long
		  time.

	== AltosDroid

		* First version of this application
