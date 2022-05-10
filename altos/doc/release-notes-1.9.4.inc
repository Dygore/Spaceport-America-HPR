= Release Notes for Version 1.9.4
include::release-head.adoc[]
:doctype: article

	Version 1.9.4

	== AltOS

	* Fixed firmware for TeleDongle v3.0 and TeleBT v4.0. Version 1.9.3 bricked those
	  devices requiring them to be opened up to get them working again.

	* Fixed firmware for EasyMini v1.0. Changes in the storage
	  code caused it to use significantly more RAM which made the
	  interrupt stack smash into other data structures and cause
	  the device to panic at startup time.

	== AltosUI

	* Add TeleMetrum v3.0 firmware to the Linux and Mac OSX
          AltosUI packages.
