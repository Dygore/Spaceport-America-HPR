= Release Notes for Version 1.9.5
include::release-head.adoc[]
:doctype: article

	Version 1.9.5

	== AltOS

	* Add Booster mode for all dual-deploy altimeters. Select
	  "Separation & Apogee" mode in AltosUI to fire the 'main'
	  channel at first motor burnout and the 'apogee' channel at
	  apogee.

	* Improve reliability of stm32l firmware under heavy
	  load. This was found using Monitor Idle on TeleMega with all
	  flash slots full of data which would occasionally lock up.

	* Fix orientation label in AltosUI for devices with
	  radios. This makes the orientation say 'Antenna up' and
	  'Antenna Down' for TeleMetrum and TeleMega again.

	* Fix log data for pyro channels which were inhibited (often
	  because of a tilt limit). They used to report 'fired' as
	  soon as they were inhibited, which was misleading. Now they
	  never report as having been fired.

	* Allow Igniter Test to fire each igniter more than once.

	== AltosUI

	* Improve performance of Monitor Idle mode by sending less data for each
	  update.

	* Improve Mac OS X install scripts so that they work on
          Catalina. This involves sending the user to get Java from
          the AdoptOpenJDK project which provides a version that works
          with Java applications.

	* Make larger dialogs scrollable for use on smaller screens.

	* Fix troubles re-flashing EasyMega boards running older
          firmware.

	* Add TeleMetrum v3.0 firmware to the Windows
          AltosUI packages.
