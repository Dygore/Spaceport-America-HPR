== Operation

	Operating an EasyMotor board is pretty easy. Turn the power on
	before launch, typically during the usual pre-flight electronics
	checklist after the rocket is installed on a launch rail.

	The board will beep out a Morse code “P” every few seconds
	indicating that it's in pad mode and ready to detect launch.
	Once launch is detected, the board logs pressure and acceleration
	data 100 times per second throughout the flight.

	After flight, AltosUI can be used to download the flight data,
	then export it to a comma separated values (CSV) file. Such a
	file can easily be loaded into a spreadsheet for analysis.
