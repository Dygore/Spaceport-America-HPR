= Release Notes for Version 1.6
include::release-head.adoc[]
:doctype: article

	Version 1.6 includes support for our updated TeleDongle v3.0
	product and bug fixes in in the flight software for all our boards
	and ground station interfaces.

	== AltOS

	AltOS New Features

	* Add support for TeleDongle v3.0 boards.

	AltOS Fixes

	* Don't beep out the continuity twice by accident in idle mode.
	  If the battery voltage report takes longer than the initialiation
	  sequence, the igniter continuity would get reported twice.

	* Record all 32 bits of gyro calibration data in TeleMega and
	  EasyMega log files. This fixes computation of the gyro rates
	  in AltosUI.

	* Change TeleDongle LED usage. Green LED flashes when valid
	  packet is received. Red LED flashes when invalid packet is
	  received.

	* Replace LPC11U14 SPI driver with non-interrupt version. The
	  interrupt code would occasionally wedge on long transfers
	  if interrupts were blocked for too long. This affects all
	  released TeleGPS products; if you have a TeleGPS device,
	  you'll want to reflash the firmware.

	== AltosUI and TeleGPS Applications

	AltosUI and TeleGPS New Features

	* Compute tilt angle from TeleMega and EasyMega log
	  files. This duplicates the quaternion-based angle tracking
	  code from the flight firmware inside the ground station
	  software so that post-flight analysis can include evaluation
	  of the tilt angle.

	* Shows the tool button window when starting with a data file
	  specified. This means that opening a data file from the file
	  manager will now bring up the main window to let you operate
	  the whole application.

	AltosUI Fixes

	* Show the 'Connecting' dialog when using Monitor Idle. Lets
	  you cancel the Monitor Idle startup when connecting over the
	  radio link.

	* Make 'Monitor Idle' work for TeleGPS devices when connected
	  over USB. It's nice for testing without needing to broadcast
	  over the radio.

	* Use different Windows API to discover USB devices. This
	  works better on my Windows 7 box, and will be used if the
	  older API fails to provide the necessary information.

	* Look in more places in the registry to try and identify the
	  installed Java version on Windows. If you install the
	  default 32-bit version of Windows on a 64-bit OS, the Java
	  registry information is hiding \SOFTWARE\Wow6432Node for
	  some reason.

	* Fix file association on Windows by searching for the
	  javaw.exe program instead of assuming it is in
	  %SYSTEMROOT%. This makes double-clicking on Altus Metrum
	  data files in the file manager work correctly.

	* When replaying a file, put 'done' in the Age field when we
	  reach the end of the file, instead of continuing to count forever.

	* In the Scan Channels code, wait for five seconds if we see
	  any packet. This is needed because AltOS now sends the
	  callsign, serial number and flight number only once every
	  five seconds these days.

	* In the Scan Channels code, reset pending flight state
	  information each time we change channels. This avoids having
	  flight computers appear on multiple frequencies by accident.
