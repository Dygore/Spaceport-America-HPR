== AltosUI

	.AltosUI Main Window
	image::altosui.png[width=450]

	The AltosUI program provides a graphical user interface for
	interacting with the Altus Metrum product family. AltosUI can
	monitor telemetry data, configure devices and many other
	tasks. The primary interface window provides a selection of
	buttons, one for each major activity in the system.  This
	chapter is split into sections, each of which documents one of
	the tasks provided from the top-level toolbar.

	ifdef::radio[]

    	=== Monitor Flight

		Selecting this item brings up a dialog box listing all
		of the connected TeleDongle devices. When you choose
		one of these, AltosUI will create a window to display
		telemetry data as received by the selected TeleDongle
		device.

		.Device Selection Dialog
		image::device-selection.png[width=300]

		All telemetry data received are automatically recorded
		in suitable log files. The name of the files includes
		the current date and rocket serial and flight numbers.

		The radio frequency being monitored by the TeleDongle
		device is displayed at the top of the window. You can
		configure the frequency by clicking on the frequency
		box and selecting the desired frequency. AltosUI
		remembers the last frequency selected for each
		TeleDongle and selects that automatically the next
		time you use that device.

		Below the TeleDongle frequency selector, the window
		contains a few significant pieces of information about
		the altimeter providing the telemetry data stream:

		 * The configured call-sign

		 * The device serial number

		 * The flight number. Each altimeter remembers how
		   many times it has flown.

		 * The rocket flight state. Each flight passes through
		   several states including Pad, Boost, Fast, Coast,
		   Drogue, Main and Landed.

		 * The Received Signal Strength Indicator value. This
		   lets you know how strong a signal TeleDongle is
		   receiving. At the default data rate, 38400 bps, in
		   bench testing, the radio inside TeleDongle v0.2
		   operates down to about -106dBm, while the v3 radio
		   works down to about -111dBm.  Weaker signals, or an
		   environment with radio noise may cause the data to
		   not be received. The packet link uses error
		   detection and correction techniques which prevent
		   incorrect data from being reported.

		 * The age of the displayed data, in seconds since the
		   last successfully received telemetry packet.  In
		   normal operation this will stay in the low single
		   digits.  If the number starts counting up, then you
		   are no longer receiving data over the radio link
		   from the flight computer.

		Finally, the largest portion of the window contains a
		set of tabs, each of which contain some information
		about the rocket.  They're arranged in 'flight order'
		so that as the flight progresses, the selected tab
		automatically switches to display data relevant to the
		current state of the flight. You can select other tabs
		at any time. The final 'table' tab displays all of the
		raw telemetry values in one place in a
		spreadsheet-like format.

		==== Launch Pad

			.Monitor Flight Launch Pad View
			image::launch-pad.png[width=400]

			The 'Launch Pad' tab shows information used to decide when the
			rocket is ready for flight. The first elements include red/green
			indicators, if any of these is red, you'll want to evaluate
			whether the rocket is ready to launch:

			Battery Voltage::
			This indicates whether the Li-Po battery powering the
			flight computer has sufficient charge to last for
			the duration of the flight. A value of more than
			3.8V is required for a 'GO' status.

			Apogee Igniter Voltage::
			This indicates whether the apogee
			igniter has continuity. If the igniter has a low
			resistance, then the voltage measured here will be close
			to the Li-Po battery voltage. A value greater than 3.2V is
			required for a 'GO' status.

			Main Igniter Voltage::
			This indicates whether the main
			igniter has continuity. If the igniter has a low
			resistance, then the voltage measured here will be close
			to the Li-Po battery voltage. A value greater than 3.2V is
			required for a 'GO' status.

			On-board Data Logging::
			This indicates whether there is space remaining
			on-board to store flight data for the upcoming
			flight. If you've downloaded data, but failed to erase
			flights, there may not be any space left. Most of our
			flight computers can store multiple flights, depending
			on the configured maximum flight log size. TeleMini
			v1.0 stores only a single flight, so it will need to
			be downloaded and erased after each flight to capture
			data. This only affects on-board flight logging; the
			altimeter will still transmit telemetry and fire
			ejection charges at the proper times even if the
			flight data storage is full.

			GPS Locked::
			For a TeleMetrum or TeleMega device, this indicates
			whether the GPS receiver is currently able to compute
			position information. GPS requires at least 4
			satellites to compute an accurate position.

			GPS Ready::

			For a TeleMetrum or TeleMega device, this indicates
			whether GPS has reported at least 10 consecutive
			positions without losing lock. This ensures that the
			GPS receiver has reliable reception from the
			satellites.

			The Launchpad tab also shows the computed launch pad
			position and altitude, averaging many reported
			positions to improve the accuracy of the fix.

		==== Ascent

			.Monitor Flight Ascent View
			image::ascent.png[width=400]

			This tab is shown during Boost, Fast and Coast
			phases. The information displayed here helps monitor the
			rocket as it heads towards apogee.

			The height, speed, acceleration and tilt are shown along
			with the maximum values for each of them. This allows you to
			quickly answer the most commonly asked questions you'll hear
			during flight.

			The current latitude and longitude reported by the GPS are
			also shown. Note that under high acceleration, these values
			may not get updated as the GPS receiver loses position
			fix. Once the rocket starts coasting, the receiver should
			start reporting position again.

			Finally, the current igniter voltages are reported as in the
			Launch Pad tab. This can help diagnose deployment failures
			caused by wiring which comes loose under high acceleration.

		==== Descent

			.Monitor Flight Descent View
			image::descent.png[width=400]

			Once the rocket has reached apogee and (we hope)
			activated the apogee charge, attention switches to
			tracking the rocket on the way back to the ground, and
			for dual-deploy flights, waiting for the main charge
			to fire.

			To monitor whether the apogee charge operated
			correctly, the current descent rate is reported along
			with the current height. Good descent rates vary based
			on the choice of recovery components, but generally
			range from 15-30m/s on drogue and should be below
			10m/s when under the main parachute in a dual-deploy
			flight.

			With GPS-equipped flight computers, you can locate the
			rocket in the sky using the elevation and bearing
			information to figure out where to look. Elevation is
			in degrees above the horizon. Bearing is reported in
			degrees relative to true north. Range can help figure
			out how big the rocket will appear. Ground Distance
			shows how far it is to a point directly under the
			rocket and can help figure out where the rocket is
			likely to land. Note that all of these values are
			relative to the pad location. If the elevation is near
			90°, the rocket is over the pad, not over you.

			Finally, the igniter voltages are reported in this tab
			as well, both to monitor the main charge as well as to
			see what the status of the apogee charge is.  Note
			that some commercial e-matches are designed to retain
			continuity even after being fired, and will continue
			to show as green or return from red to green after
			firing.

		==== Landed

			.Monitor Flight Landed View
			image::landed.png[width=400]

			Once the rocket is on the ground, attention switches
			to recovery. While the radio signal is often lost once
			the rocket is on the ground, the last reported GPS
			position is generally within a short distance of the
			actual landing location.

			The last reported GPS position is reported both by
			latitude and longitude as well as a bearing and
			distance from the launch pad. The distance should give
			you a good idea of whether to walk or hitch a ride.
			Take the reported latitude and longitude and enter
			them into your hand-held GPS unit and have that
			compute a track to the landing location.

			Our flight computers will continue to transmit RDF
			tones after landing, allowing you to locate the rocket
			by following the radio signal if necessary. You may
			need to get away from the clutter of the flight line,
			or even get up on a hill (or your neighbor's RV roof)
			to receive the RDF signal.

			The maximum height, speed and acceleration reported
			during the flight are displayed for your admiring
			observers.  The accuracy of these immediate values
			depends on the quality of your radio link and how many
			packets were received.  Recovering the on-board data
			after flight may yield more precise results.

			To get more detailed information about the flight, you
			can click on the 'Graph Flight' button which will
			bring up a graph window for the current flight.

		==== Table

			.Monitor Flight Table View
			image::table.png[width=400]

			The table view shows all of the data available from the
			flight computer. Probably the most useful data on
			this tab is the detailed GPS information, which includes
			horizontal dilution of precision information, and
			information about the signal being received from the satellites.

		==== Site Map

			.Monitor Flight Site Map View
			image::site-map.png[width=400]

			When the TeleMetrum has a GPS fix, the Site Map tab
			will map the rocket's position to make it easier for
			you to locate the rocket, both while it is in the air,
			and when it has landed. The rocket's state is
			indicated by color: white for pad, red for boost, pink
			for fast, yellow for coast, light blue for drogue,
			dark blue for main, and black for landed.

			The map's default scale is approximately 3m (10ft) per
			pixel. The map can be dragged using the left mouse
			button. The map will attempt to keep the rocket
			roughly centered while data is being received.

			You can adjust the style of map and the zoom level
			with buttons on the right side of the map window. You
			can draw a line on the map by moving the mouse over
			the map with a button other than the left one pressed,
			or by pressing the left button while also holding down
			the shift key. The length of the line in real-world
			units will be shown at the start of the line.

			Images are fetched automatically via the Google Maps
			Static API, and cached on disk for reuse. If map
			images cannot be downloaded, the rocket's path will be
			traced on a dark gray background instead.

			You can pre-load images for your favorite launch sites
			before you leave home; check out <<_load_maps>>.

		==== Igniter

			.Monitor Flight Additional Igniter View
			image::ignitor.png[width=400]

			TeleMega includes four additional programmable pyro
			channels. The Ignitor tab shows whether each of them has
			continuity. If an ignitor has a low resistance, then the
			voltage measured here will be close to the pyro battery
			voltage. A value greater than 3.2V is required for a 'GO'
			status.

	endif::radio[]


	=== Save Flight Data

		The altimeter records flight data to its internal
		flash memory.
		ifdef::radio[]
		Data logged on board is recorded at a much
		higher rate than the telemetry system can handle, and
		is not subject to radio drop-outs. As such, it
		provides a more complete and precise record of the
		flight.
		endif::radio[]
		The 'Save Flight Data' button allows you to
		read the flash memory and write it to disk.

		Clicking on the 'Save Flight Data' button brings up a
		list of connected flight computers and TeleDongle
		devices. If you select a flight computer, the flight
		data will be downloaded from that device directly.
		ifdef::radio[]
		If you select a TeleDongle device, flight data will be
		downloaded from a flight computer over radio link via
		the specified TeleDongle. See
		<<_controlling_an_altimeter_over_the_radio_link>> for
		more information.
		endif::radio[]

		After the device has been selected, a dialog showing
		the flight data saved in the device will be shown
		allowing you to select which flights to download and
		which to delete. With version 0.9 or newer firmware,
		you must erase flights in order for the space they
		consume to be reused by another flight. This prevents
		accidentally losing flight data if you neglect to
		download data before flying again. Note that if there
		is no more space available in the device, then no data
		will be recorded during the next flight.

		The file name for each flight log is computed
		automatically from the recorded flight date, altimeter
		serial number and flight number information.

	=== Replay Flight

		Select this button and you are prompted to select a flight
		record file, either a .telem file recording telemetry data or a
		.eeprom file containing flight data saved from the altimeter
		flash memory.

		Once a flight record is selected, the flight monitor interface
		is displayed and the flight is re-enacted in real
		time.
		ifdef::radio[]
		Check
		<<_monitor_flight>> to learn how this window operates.
		endif::radio[]

	=== Graph Data

		Select this button and you are prompted to select a flight
		record file, either a .telem file recording telemetry data or a
		.eeprom file containing flight data saved from
		flash memory.

		Note that telemetry files will generally produce poor graphs
		due to the lower sampling rate and missed telemetry packets.
		Use saved flight data in .eeprom files for graphing where possible.

		Once a flight record is selected, a window with multiple tabs is
		opened.

		==== Flight Graph

			.Flight Data Graph
			image::graph.png[width=400]

			By default, the graph contains acceleration (blue),
			velocity (green) and altitude (red).

			The graph can be zoomed into a particular area by
			clicking and dragging down and to the right. Once
			zoomed, the graph can be reset by clicking and
			dragging up and to the left. Holding down control and
			clicking and dragging allows the graph to be panned.
			The right mouse button causes a pop-up menu to be
			displayed, giving you the option save or print the
			plot.

		==== Configure Graph

			.Flight Graph Configuration
			image::graph-configure.png[width=400]

			This selects which graph elements to show, and, at the
			very bottom. It also lets you configure how
			the graph is drawn:

			* Whether to use metric or imperial units

			* Whether to show a marker at each data
                          point. When displaying a small section of
                          the graph, these can be useful to know what
                          data values were recorded.

			* How wide to draw the lines in the graph

			* How to filter speed and acceleration data
                          computed from barometric data. Flight
                          computers with accelerometers never display
                          computed acceleration data, and only use
                          barometric data to compute speed during
                          descent. Flight computers without
                          accelerometers always compute both speed and
                          acceleration from barometric data. A larger
                          value smooths the data more.

		==== Flight Statistics

			.Flight Statistics
			image::graph-stats.png[width=400]

			Shows overall data computed from the flight.

		ifdef::gps[]
		==== Map

			.Flight Map
			image::graph-map.png[width=400]

			Shows a satellite image of the flight area
			overlaid with the path of the flight. The
			flight path will have different colored
			sections for each state of the flight (just
			like the Site Map in Monitor Flight mode):
			white for pad, red for boost, pink for fast,
			yellow for coast, light blue for drogue, dark
			blue for main, and black for landed.

			The red concentric circles mark the launch
			pad, the black concentric circles mark the
			landing location. Data for the point along the
			along the flight path nearest the cursor will
			be displayed at the bottom of the window. This
			data includes flight time (so you can
			correlate data in the graph window), latitude,
			longitude, height above ground and vertical
			speed. The selected point will be highlighted
			with concentric circles in the same color as
			the flight path at that point.

		endif::gps[]

	=== Export Data

		This tool takes the raw data files and makes them
		available for external analysis. When you select this
		button, you are prompted to select a flight data file,
		which can be either a .eeprom or .telem.  The .eeprom
		files contain higher resolution and more continuous
		data, while .telem files contain receiver signal
		strength information.  Next, a second dialog appears
		which is used to select where to write the resulting
		file.
		ifdef::gps[]
			It has a selector to choose between CSV and KML
			file formats.
		endif::gps[]

		==== Comma Separated Value Format

			This is a text file containing the data in a form
			suitable for import into a spreadsheet or other
			external data analysis tool. The first few lines of
			the file contain the version and configuration
			information from the altimeter, then there is a single
			header line which labels all of the fields. All of
			these lines start with a '#' character which many
			tools can be configured to skip over.

			The remaining lines of the file contain the data, with
			each field separated by a comma and at least one
			space. All of the sensor values are converted to
			standard units, with the barometric data reported in
			both pressure, altitude and height above pad units.

		ifdef::gps[]
			==== Keyhole Markup Language (for Google Earth)

				This is the format used by Google Earth to provide an
				overlay within that application. With this, you can
				use Google Earth to see the whole flight path
				in 3D.
		endif::gps[]

	=== Configure Altimeter

		.Altimeter Configuration
		image::configure-altimeter.png[width=350]

		ifdef::radio[]
		Select this button and then select either an altimeter or
		TeleDongle Device from the list provided. Selecting a TeleDongle
		device will use the radio link to configure a remote
		altimeter.
		endif::radio[]
		ifndef::radio[]
		Select this button and then select an altimeter.
		endif::radio[]

		The first few lines of the dialog provide information about the
		connected device, including the product name,
		software version and hardware serial number. Below that are the
		individual configuration entries.

		At the bottom of the dialog, there are four buttons:

		Save::
		This writes any changes to the configuration parameter
		block in flash memory. If you don't press this button,
		any changes you make will be lost.

		Reset::
		This resets the dialog to the most recently saved
		values, erasing any changes you have made.

		Reboot::

		This reboots the device. Use this to switch from idle
		to pad mode by rebooting once the rocket is oriented
		for flight, or to confirm changes you think you saved
		are really saved.

		Close::

		This closes the dialog. Any unsaved changes will be
		lost.

	        The rest of the dialog contains the parameters to be configured.

		include::config-device.adoc[]


	=== Configure AltosUI

		.Configure AltosUI Dialog
		image::configure-altosui.png[width=230]

		This button presents a dialog so that you can
		configure the AltosUI global settings.

		include::config-ui.adoc[]

	ifdef::radio[]

	=== Configure Groundstation

		.Configure Groundstation Dialog
		image::configure-groundstation.png[width=300]

		Select this button and then select a TeleDongle or
		TeleBT Device from the list provided.

		The first few lines of the dialog provide information
		about the connected device, including the product
		name, software version and hardware serial
		number. Below that are the individual configuration
		entries.

		Note that TeleDongle and TeleBT don't save any
		configuration data, the settings here are recorded on
		the local machine in the Java preferences
		database. Moving the device to another machine, or
		using a different user account on the same machine
		will cause settings made here to have no effect.

		At the bottom of the dialog, there are three
		buttons:

		Save::
		This writes any changes to the local Java
		preferences file. If you don't press this
		button, any changes you make will be lost.

		Reset::
		This resets the dialog to the most recently
		saved values, erasing any changes you have
		made.

		Close::
		This closes the dialog. Any unsaved changes
		will be lost.

		The rest of the dialog contains the parameters
		to be configured.

		==== Frequency

			This configures the frequency to use for both
			telemetry and packet command mode. Set this
			before starting any operation involving packet
			command mode so that it will use the right
			frequency. Telemetry monitoring mode also
			provides a menu to change the frequency, and
			that menu also sets the same Java preference
			value used here.

		==== RF Calibration

			The radios in every Altus Metrum device are
			calibrated at the factory to ensure that they
			transmit and receive on the specified
			frequency.  To change a TeleDongle or TeleBT's
			calibration, you must reprogram the unit
			completely, so this entry simply shows the
			current value and doesn't allow any changes.

	      	==== Telemetry Rate

			This lets you match the telemetry and packet
			link rate from the transmitter. If they don't
			match, the device won't receive any data.
	endif::radio[]

	=== Flash Image

		This reprograms Altus Metrum devices with new
		firmware.
		ifdef::telemetrum,telemini[]
			TeleMetrum v1.x, TeleDongle v0.2, TeleMini v1.0
			and TeleBT v1.0 are all reprogrammed by using another
			similar unit as a programming dongle (pair
			programming).
		endif::telemetrum,telemini[]
		ifdef::telemega,easymega,easytimer,telemetrum[]
			TeleMega, EasyMega, TeleMetrum v2 or newer,
			EasyMini, EasyTimer, TeleBT v3 or newer and TeleDongle v3
			or newer are all
		endif::telemega,easymega,easytimer,telemetrum[]
		ifndef::telemega,easymega,easytimer,telemetrum[]
			EasyMini is
		endif::telemega,easymega,easytimer,telemetrum[]
		programmed directly
		over USB (self programming). Please read
		the directions for flashing devices in
		<<_updating_device_firmware>>.

	=== Fire Igniter

		.Fire Igniter Window
		image::fire-igniter.png[width=120]

		This activates the igniter circuits in the flight
		computer to help test recovery systems
		deployment.
		ifdef::radio[]
		Because this command can operate over the
		Packet Command Link, you can prepare the rocket as for
		flight and then test the recovery system without
		needing to snake wires inside the air-frame.
		endif::radio[]

		Selecting the 'Fire Igniter' button brings up the
		usual device selection dialog. Pick the desired
		device. This brings up another window which shows the
		current continuity test status for all of the pyro
		channels.

		Next, select the desired igniter to fire. This will
		enable the 'Arm' button.

		Select the 'Arm' button. This enables the 'Fire'
		button. The word 'Arm' is replaced by a countdown
		timer indicating that you have 10 seconds to press the
		'Fire' button or the system will deactivate, at which
		point you start over again at selecting the desired
		igniter.

	ifdef::radio[]
	=== Scan Channels

		.Scan Channels Window
		image::scan-channels.png[width=300]

		This listens for telemetry packets on all of the
		configured frequencies, displaying information about
		each device it receives a packet from. You can select
		which of the baud rates and telemetry formats should
		be tried; by default, it only listens at 38400 baud
		with the standard telemetry format used in v1.0 and
		later firmware.
	endif::radio[]

	ifdef::gps[]
	include::load-maps.adoc[]
	endif::gps[]

	ifdef::radio[]
	=== Monitor Idle

		.Monitor Idle Window
		image::monitor-idle.png[width=500]

		This brings up a dialog similar to the Monitor Flight
		UI, except it works with the altimeter in “idle” mode
		by sending query commands to discover the current
		state rather than listening for telemetry
		packets. Because this uses command mode, it needs to
		have the TeleDongle and flight computer callsigns
		match exactly. If you can receive telemetry, but
		cannot manage to run Monitor Idle, then it's very
		likely that your callsigns are different in some way.

		You can change the frequency and callsign used to
		communicate with the flight computer; they must both
		match the configuration in the flight computer
		exactly.
	endif::radio[]
