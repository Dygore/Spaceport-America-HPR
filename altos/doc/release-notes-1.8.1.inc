= Release Notes for Version 1.8.1
include::release-head.adoc[]
:doctype: article

	Version 1.8.1 includes an important bug fix for Apogee Lockout
	operation in all flight computers. Anyone using this option
	must update firmware.

	This release also contains a change in how flight computers
	with accelerometers deal with speeds around and above Mach
	1. In previous versions, the flight computer would completely
	disregard the barometric sensor above 330m/s (around Mach
	1). Now, the data from the barometric sensor is reduced in
	effect without ever going away entirely. This prevents early
	drogue deployment for flights which spend considerable time
	above Mach 1.

	1.8.1 also contains a couple of minor fixes for AltosUI when
	analyzing saved data files.

	== AltOS

	AltOS Bug Fixes

	* Handle time value wrapping in Apogee Lockout
          correctly. Without this, apogee lockout would sometimes
          prevent any drogue charge from firing.

	* Change Kalman filter on flight computers with accelerometer
          to continue using the barometric sensor even at high speeds
          to avoid unintentional drogue deployment during
          deceleration.

	== AltosUI and TeleGPS Applications

	AltosUI New Features

	* Add new 'Huge' font size to make text even bigger on high
          resolution monitors.

	AltosUI Bug Fixes

	* Prevent some crashes when reading older saved flight data
          for graphing or KML export.

	* Load frequency preference at startup. The loading code was
          broken, so you'd see only the default frequencies.
