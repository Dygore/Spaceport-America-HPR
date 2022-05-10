= Release Notes for Version 1.3
include::release-head.adoc[]
:doctype: article

	Version 1.3 is a major release. It adds support for TeleMega,
	TeleMetrum v2.0, TeleMini v2.0 and EasyMini.

	== AltOS

		AltOS new features:

      		* Add STM32L processor support. This includes
		  enhancements to the scheduler to support products
		  with many threads.

      		* Add NXP LPC11U14 processor support.


      		* Support additional pyro channels. These are
		  configurable through the UI to handle air starts,
		  staging, additional recovery events and external
		  devices such as cameras.


      		* Add 3-axis gyro support for orientation
		  tracking. This integrates the gyros to compute the
		  angle from vertical during flight, allowing the
		  additional pyro events to be controlled by this
		  value.


      		* Many more device drivers, including u-Blox Max 7Q
		  GPS, Freescale MMA6555 digital single-axis
		  accelerometer, Invensense MPU6000 3-axis
		  accelerometer + 3 axis gyro, Honeywell HMC5883
		  3-axis magnetic sensor and the TI CC1120 and CC115L
		  digital FM transceivers

	== AltosUI Application

		AltosUI new features:

		* Support TeleMega, TeleMetrum v2.0, TeleMini v2.0 and
                  EasyMini telemetry and log formats.


		AltosUI fixes:

		* Use preferred units for main deployment height
		  configuration, instead of always doing configuration in
		  meters.
	== MicroPeak Application

      		* Add 'Download' button to menu bar.

		* Save the last log directory and offer that as the
		  default for new downloads
