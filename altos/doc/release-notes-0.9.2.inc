= Release Notes for Version 0.9.2
include::release-head.adoc[]
:doctype: article

	Version 0.9.2 is an AltosUI bug-fix release, with no firmware
	changes.

	== AltosUI

		AltosUI fixes:

		* Fix plotting problems due to missing file in the Mac
                  OS install image.

		* Always read whole eeprom blocks, mark empty records
                  invalid, display parsing errors to user.

		* Add software version to Configure AltosUI dialog
