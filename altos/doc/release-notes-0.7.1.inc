= Release Notes for Version 0.7.1
include::release-head.adoc[]
:doctype: article

	Version 0.7.1 is the first release containing our new
	cross-platform Java-based user interface.

	== AltosUI Application

	* Receive and log telemetry from a connected TeleDongle
	  device. All data received is saved to log files named with
	  the current date and the connected rocket serial and flight
	  numbers. There is no mode in which telemetry data will not
	  be saved.


	* Download logged data from TeleMetrum devices, either through
	  a direct USB connection or over the air through a TeleDongle
	  device.


	* Configure a TeleMetrum device, setting the radio channel,
	  callsign, apogee delay and main deploy height. This can be
	  done through either a USB connection or over a radio link
	  via a TeleDongle device.


	* Replay a flight in real-time. This takes a saved telemetry
	  log or eeprom download and replays it through the user
	  interface so you can relive your favorite rocket flights.


	* Reprogram Altus Metrum devices. Using an Altus Metrum device
	  connected via USB, another Altus Metrum device can be
	  reprogrammed using the supplied programming cable between
	  the two devices.


	* Export Flight data to a comma-separated-values file. This
	  takes either telemetry or on-board flight data and generates
	  data suitable for use in external applications. All data is
	  exported using standard units so that no device-specific
	  knowledge is needed to handle the data.


	* Speak to you during the flight. Instead of spending the
	  flight hunched over your laptop looking at the screen, enjoy
	  the view while the computer tells you what’s going on up
	  there. During ascent, you hear the current flight state and
	  altitude information. During descent, you get azimuth,
	  elevation and range information to try and help you find
	  your rocket in the air. Once on the ground, the direction
	  and distance are reported.
