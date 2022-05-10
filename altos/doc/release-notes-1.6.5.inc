= Release Notes for Version 1.6.5
include::release-head.adoc[]
:doctype: article

	Version 1.6.5 fixes a TeleMega and TeleMetrum v2.0 bug where
	the device would often stop logging data and transmitting
	telemetry in flight. All TeleMega v1.0, v2.0 and TeleMetrum
	v2.0 users should update their flight firmware.

	== AltOS

	AltOS fixes:

	* Fix STM32L SPI driver to prevent lock-up in the logging or
          radio code, either of which could stop data logging and
          telemetry. Found and characterized by Chuck Haskin, who also
          tested the new firmware before release.

	== AltosUI, TeleGPS and AltosDroid Applications

	AltosUI fixes:

	* Deliver firmward for TeleMega v2.0 and TeleBT v3.0 with
          Windows package.
