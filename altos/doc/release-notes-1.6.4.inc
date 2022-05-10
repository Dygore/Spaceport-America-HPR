= Release Notes for Version 1.6.4
include::release-head.adoc[]
:doctype: article

	Version 1.6.4 fixes a bluetooth communication problem with
	TeleBT v1.0 devices, along with some altosui and altosdroid
	minor nits. It also now ships firmware for some newer devices.

	== AltOS

	AltOS fixes:

	* Fix hardware flow control on TeleBT v1.0. Hardware RTS/CTS
	  doesn't seem to work, switch from using the hardware to
	  driving these pins with software.

	* Fix ARM USB drivers to deal with OS restarts. Needed to
	  reset all USB-related state when the USB bus is reset. These
	  fixes affect all STM32L, STM32F0 and LPC11U14 based devices.

	== AltosUI, TeleGPS and AltosDroid Applications

	AltosUI, TeleGPS and AltosDroid New Features:

	* Automatically switch from meters or feet to kilometers or
          miles for distance units.

	* Add Monitor Idle mode to TeleGPS application.

	AltosUI, TeleGPS and AltosDroid Fixes:

	* Abort map preloading when the preload map dialog is closed.

	* In AltosDroid, Don't reconnect to last device if the user
          had disconnected it the last time the application was
          active.

	== Documentation

	* Mention TeleMega v2.0 in hardware specs table.

	* Document TeleGPS RF output in telegps manual.
