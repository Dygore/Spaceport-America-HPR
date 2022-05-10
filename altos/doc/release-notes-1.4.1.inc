= Release Notes for Version 1.4.1
include::release-head.adoc[]
:doctype: article

	Version 1.4.1 is a minor release. It fixes install issues on
	Windows and provides the missing TeleMetrum V2.0 firmware. There
	aren't any changes to the firmware or host applications at
	all. All Windows users will want to upgrade to get the signed
	driver, but Mac and Linux users who do not need the TeleMetrum
	V2.0 firmware image will not need to upgrade.
  
	== AltosUI and TeleGPS Applications:

	Windows Install Fixes

      	* Provide signed Windows driver files. This should avoid any need to
	  disable driver signature checking on Windows 7 or 8.
	
      	* Fix Java version detection and download. Previously, the
	  installer would only look for Java 6 or 7 and insist on
	  downloading its own Java bits if there was something else
	  installed. Furthermore, the 64-bit Java link provided didn't
	  work for anyone other than Keith, making it impossible to
	  install AltOS on any machine with Java SE 8 installed.
	
	Other Fixes

      	* Include 1.4 firmware for TeleMetrum V2.0. None of the
	  installers shipped this file. Now it's included in the AltOS
	  packages for Linux, Mac and Windows.
	
      	* Include Google Application Key for map downloading. The 1.4
	  release didn't have this key in the released version of the
	  software, making map downloading fail for most people.
