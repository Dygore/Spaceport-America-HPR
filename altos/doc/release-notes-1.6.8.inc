= Release Notes for Version 1.6.8
include::release-head.adoc[]
:doctype: article

	Version 1.6.8 fixes a TeleMega and TeleMetrum v2.0 bug where
	the device could stop logging data and transmitting
	telemetry in flight. All TeleMega v1.0, v2.0 and TeleMetrum
	v2.0 users should update their flight firmware.

	== AltOS

	AltOS fixes:

	* Fix STM32L DMA driver to work around STM32L SoC DMA priority
          issue t lock-up in the logging or radio code, either of
          which could stop data logging and telemetry.

	* Avoid having TeleBT battery status values smash telemetry
          packet data by holding a lock during both operations.

	* Don't start reporting 'flight 0' when the log fills in
	  flight.

	AltOS changes:
	
	* Flash LEDS on all products briefly during power up so that
          they can be tested during production.

	== AltosUI, TeleGPS and AltosDroid Applications

	AltosUI fixes:

	* Re-enable go/no-go entries after they've been disabled due
          to lack of data. If telemetry information is delayed when
          the Ui starts up, sometimes important fields would get
          disabled to never re-appear.

	* Deal with ground station failure better during Configure
          Ground Station operation by cleaning up pending operations.

