== TeleGPS Application

	The TeleGPS application provides a graphical user interface for
	interacting with the Altus Metrum product family. TeleGPS can
	monitor telemetry data, configure devices and many other
	tasks. The primary interface window is for displaying data
	received over the telemetry link. There are additional
	tasks available from the main window menu bar.

	=== Telemetry Monitoring

		This is the window brought up when you start the
		application. If you have a TeleDongle device connected
		to the computer, it will automatically be selected for
		telemetry monitoring

		All telemetry data received are automatically recorded
		in suitable log files. The name of the files includes
		the current date and TeleGPS serial and flight
		numbers.

		The radio frequency being monitored by the TeleDongle
		device is displayed at the top of the window. You can
		configure the frequency by clicking on the frequency
		box and selecting the desired frequency. The TeleGPS
		application remembers the last frequency selected for
		each TeleDongle and selects that automatically the
		next time you use that device.

		Below the TeleDongle frequency selector, the window
		contains a few significant pieces of information about
		the altimeter providing the telemetry data stream:

		* The configured call-sign

		* The device serial number

		* The flight number. TeleGPS remembers how many times
                  it has flown.

		* The Received Signal Strength Indicator value. This
		  lets you know how strong a signal TeleDongle is
		  receiving. The radio inside TeleDongle operates down
		  to about -100dBm; weaker signals may not be
		  receivable. The packet link uses error detection and
		  correction techniques which prevent incorrect data
		  from being reported.

		* The age of the displayed data, in seconds since the
		  last successfully received telemetry packet.  In
		  normal operation this will stay in the low single
		  digits.  If the number starts counting up, then you
		  are no longer receiving data over the radio link
		  from the flight computer.

		Finally, the largest portion of the window contains a set of
		tabs, each of which contain some information about the TeleGPS
		board. The final 'table' tab displays many of the raw telemetry
		values in one place in a spreadsheet-like format.
      
		==== Map

			The Map tab shows the TeleGPS track over time
			on top of map data making it easy to locate
			the device.
        
			.TeleGPS Map View
			image::telegps-map.png[width=400]

			The map's default scale is approximately 3m
			(10ft) per pixel. The map can be dragged using
			the left mouse button. The map will attempt to
			keep the rocket roughly centered while data is
			being received.
        
			You can adjust the style of map and the zoom
			level with buttons on the right side of the
			map window. You can draw a line on the map by
			moving the mouse over the map with a button
			other than the left one pressed, or by
			pressing the left button while also holding
			down the shift key. The length of the line in
			real-world units will be shown at the start of
			the line.
	
			Images are fetched automatically via the
			Google Maps Static API, and cached on disk for
			reuse. If map images cannot be downloaded, the
			rocket's path will be traced on a dark gray
			background instead.
        
			You can pre-load images for your favorite
			launch sites before you leave home; check out
			<<_load_maps>>.
	
		==== Location

			The Location tab shows the raw GPS data
			received from TeleGPS.
	
			.TeleGPS Location View
			image::telegps-location.png[width=400]

		==== Status

			The Status tab shows data relative to the
			location of TeleGPS when the application first
			received telemetry from it.
	
			.TeleGPS Status View
			image::telegps-status.png[width=400]

		==== Table

			The Table tab shows detailed information about
			the GPS receiver
	
			.TeleGPS Information Table
			image::telegps-table.png[width=400]

	=== TeleGPS Menus

		TeleGPS has three or four menus at the top of
		the window:

		File::

		New Window, Graph Data, Export Data, Load Maps,
		Preferences, Close and Exit
	      
		Monitor::

		Connect Device, Disconnect and Scan Channels

		Device::

		Download Data, Configure Device and Flash Device
	      
		Frequency::

		This shows the current monitoring frequency with a
		drop-down menu listing other configured
		frequencies. You can change the set of frequencies
		shown here from the Preferences dialog. This menu is
		only shown when the TeleGPS application is connected
		to a TeleDongle or TeleBT device.
	      
      
		==== New Window

			This creates another telemetry monitoring window, in case
			you have multiple TeleDongle devices connected to the
			computer.
	
	=== Graph Data

		The Graph tab shows a plot of the the GPS data
		collected. The X axis is time in seconds; there are a
		variety of Y axes available for different kinds of
		data. This window also allows you to see some
		statistics computed from the data, and an overall map
		of the entire data record.
	  
		==== Data Graph

			.TeleGPS Graph
			image::telegps-graph-graph.png[width=400]

		==== Graph Configuration

			.TeleGPS Graph Configuration
			image::telegps-graph-configure.png[width=400]

			This selects which graph elements to show, and, at the
			bottom, lets you switch between metric and imperial
			units

		==== Statistics

			.TeleGPS Statistics
			image::telegps-graph-stats.png[width=400]

			Shows overall data computed from the flight.
		
		==== Map

			.TeleGPS Map
			image::telegps-graph-map.png[width=450]

			Shows a map of the area overlaid with the GPS track. As with
			the telemetry monitoring window, you can select the style
			of map and zoom level using buttons along the side;
			you can scroll the map by dragging within the map pressing
			the left button and you can draw a line to measure
			distances using either the left button with the shift key,
			or any other button.

	=== Export Data

		This tool takes the raw data files and makes them
		available for external analysis. When you select this
		button, you are prompted to select a data file, which
		can be either a .eeprom or .telem.  The .eeprom files
		contain higher resolution and more continuous data,
		while .telem files contain receiver signal strength
		information.  Next, a second dialog appears which is
		used to select where to write the resulting file. It
		has a selector to choose between CSV and KML file
		formats.
	
		==== Comma Separated Value Format

			This is a text file containing the data in a
			form suitable for import into a spreadsheet or
			other external data analysis tool. The first
			few lines of the file contain the version and
			configuration information from TeleGPS, then
			there is a single header line which labels all
			of the fields. All of these lines start with a
			'#' character which many tools can be
			configured to skip over.

			The remaining lines of the file contain the
			data, with each field separated by a comma and
			at least one space. All of the sensor values
			are converted to standard units, with the
			barometric data reported in both pressure,
			altitude and height above pad units.

		==== Keyhole Markup Language (for Google Earth)

			This is the format used by Google Earth to provide an overlay
			within that application. With this, you can use Google Earth to
			see the whole path in 3D.

	include::load-maps.adoc[]

	=== Preferences

		.TeleGPS Preferences Window
		image::telegps-preferences.png[width=230]

		include::config-ui.adoc[]

	=== Close

		This closes the current window, leaving any other windows
		open and the application running.
	
	=== Exit

		This closes all TeleGPS windows and terminates the
		application.
	
	=== Connect Device

		Selecting this item brings up a dialog box listing all
		of the connected TeleDongle devices. When you choose
		one of these, AltosUI will display telemetry data as
		received by the selected TeleDongle device.
	
		.Device Selection Dialog
		image::device-selection.png[width=300]

	=== Disconnect

		Disconnects the currently connected TeleDongle or
		TeleBT

	=== Scan Channels

		.Radio Scanning Dialog
		image::telegps-scan.png[width=300]

		Scans the configured set of frequencies looking for
		telemetry signals. A list of all of the discovered
		signals is show; selecting one of those and clicking
		on 'Monitor' will select that frequency in the
		associated TeleGPS application window.

	=== Download Data

		TeleGPS records data to its internal flash memory.
		On-board data is recorded at the same rate as
		telemetry but is not subject to radio drop-outs. As
		such, it generally provides a more complete and
		precise record.  The 'Download Data' menu entry allows
		you to read the flash memory and write it to disk.

		Select the 'Download Data' menu entry to bring up a
		list of connected TeleGPS devices. After the device
		has been selected, a dialog showing the data stored in
		the device will be shown allowing you to select which
		entries to download and which to delete. You must
		erase flights in order for the space they consume to
		be reused by another track. This prevents accidentally
		losing data if you neglect to download data before
		starting TeleGPS again. Note that if there is no more
		space available in the device, then no data will be
		recorded.

		The file name for each data log is computed
		automatically from the recorded date, altimeter serial
		number and flight number information.
	
	=== Configure Device

		.TeleGPS Configuration Dialog
		image::telegps-configure.png[width=350]

		Select this button and then select any connected TeleGPS
		device from the list provided.

		The first few lines of the dialog provide information
		about the connected device, including the product
		name, software version and hardware serial
		number. Below that are the individual configuration
		entries.

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

	=== Flash Device

		This reprograms TeleGPS devices with new
		firmware. Please read the directions for flashing
		devices in <<_updating_device_firmware>>.
