= Release Notes for Version 1.9.10
include::release-head.adoc[]
:doctype: article

	Version 1.9.10

	This release contains a couple of bug fixes for ground station software.

	== AltosUI

	* Rework the windows DLL build to make AltosUI run on more
          instances of Windows 10.

	== AltosDroid

	* Adapt to Android security changes which prevent AltosDroid
	  from storing flights in
	  /storage/emulated/0/AltusMetrum. Now, flights are stored in
	  /storage/emulated/0/media/org.altusmetrum.AltosDroid/AltusMetrum
	  instead.  Also, AltosDroid will display an error message if
	  flight data cannot be logged.
