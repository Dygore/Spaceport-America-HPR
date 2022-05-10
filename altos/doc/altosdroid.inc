== AltosDroid

	AltosDroid provides the same flight monitoring capabilities as
	AltosUI, but runs on Android devices. AltosDroid is designed
	to connect to a TeleBT receiver over Bluetooth™ and (on
	Android devices supporting USB On-the-go) TeleDongle and
	TeleBT devices over USB. AltosDroid monitors telemetry data,
	logging it to internal storage in the Android device, and
	presents that data in a UI similar to the 'Monitor Flight'
	window in AltosUI.

	This manual will explain how to configure AltosDroid, connect
	to TeleBT or TeleDongle, operate the flight monitoring
	interface and describe what the displayed data means.
    
	=== Installing AltosDroid

		AltosDroid is available from the Google Play store. To
		install it on your Android device, open the Google
		Play Store application and search for
		“altosdroid”. Make sure you don't have a space between
		“altos” and “droid” or you probably won't find what
		you want. That should bring you to the right page from
		which you can download and install the application.
      
    	=== Charging TeleBT Battery

		Before using TeleBT with AltosDroid, make sure the
		internal TeleBT battery is charged.  To do this,
		attach a micro USB cable from a computer or other USB
		power source to TeleBT.  A dual LED on the circuit
		board should illuminate, showing red while the battery
		is charging, green when charging is completed, and
		both red and green on at the same time if there is a
		battery fault.
      
    	=== Connecting to TeleBT over Bluetooth™

		Note that when turning TeleBT on, you may see a brief LED
		flash, but there will be no "activity" indicated until you
		pair with the device from AltosDroid.

		Press the Android 'Menu' button or soft-key to see the
		configuration options available. Select the 'Connect a
		device' option and then the 'Scan for devices' entry
		at the bottom to look for your TeleBT device. Select
		your device, and when it asks for the code, enter
		'1234'.

		Subsequent connections will not require you to enter
		that code, and your 'paired' device will appear in the
		list without scanning.
      
    	=== Connecting to TeleDongle or TeleBT over USB

		Get a special USB On-the-go adapter cable. These
		cables have a USB micro-B male connector on one end
		and a standard A female connector on the other
		end. Plug in your TeleDongle or TeleBT device to the
		adapter cable and the adapter cable into your phone
		and AltosDroid should automatically start up. If it
		doesn't, the most likely reason is that your Android
		device doesn't support USB On-the-go.
      
	=== AltosDroid Menu

		The main AltosDroid menu has a selection of operation
		and configuration options.

		Connect a device::

		Offers a menu of available TeleBT devices, and an
		option to scan for additional devices.

		Disconnect device::

		Disconnects the current TeleBT or TeleDongle
		device. You can reconnect TeleBT over bluetooth by
		using Connect a Device. You can reconnect a USB device
		by unplugging it and then plugging it back in.

		Select radio frequency::

		This selects which frequency to listen on by bringing
		up a menu of pre-set radio frequencies. Pick the one
		which matches your altimeter.
	
		Select Tracker::

		Switches the information displays to show data for a
		different transmitting device. The map will always
		show all of the devices in view. Trackers are shown
		and selected by serial number, so make sure you note
		the serial number of devices in each airframe.

		Delete Track::

		Deletes all information about a transmitting device.
	
		Setup::

		Offers additional configuration operations. See
		<<_setup>>.

		Idle Mode::

		Communicate with an altimeter in Idle mode. See <<_idle_mode>>

		Quit::

		Shuts down AltosDroid.

	=== Setup

		Telemetry Rate::

		Altus Metrum transmitters can be configured to operate
		at lower data rates to improve transmission range. If
		you have configured your device to do this, this menu
		item allows you to change the receiver to match.
	
		Units::

		Selects which units used to report values.
	
		Map Type::

		Displays a menu of map types and lets you select
		one. Hybrid maps include satellite images with a
		roadmap overlaid. Satellite maps dispense with the
		roadmap overlay. Roadmap shows just the roads. Terrain
		includes roads along with shadows indicating changes
		in elevation, and other geographical features.
	
		Map Source::

		Select between online and offline maps. Online maps
		will show a 'move to current position' icon in the
		upper right corner, while offline maps will have
		copyright information all over the map. Otherwise,
		they're pretty similar.
	
		Preload Maps::

		Brings up a dialog allowing you to download offline
		map tiles so that you can have maps available even if
		you have no network connectivity at the launch site.
	
		Manage Frequencies::

		This presents a dialog containing the current list of
		frequencies that will be show in the 'Select radio
		frequency' menu. You can change the label for existing
		frequencies, delete or add new frequencies.

	=== Idle Mode

		Callsign::

		Lets you configure the callsign used by AltosDroid to
		communicate with the flight computer. The callsign on
		the two devices must match or the communication will
		fail. This provides a modest amount of protection
		against accidentally controlling another persons
		flight computer.

		Monitor::

		This provides similar information to monitoring the
		telemetry stream from a flight computer in flight
		mode. The title of the application will change to
		include (idle) so you can tell this mode is active.

		Reboot::

		Remotely reboots the flight computer. This is useful
		when the flight computers turned on while the airframe
		is horizontal and you want to restart the flight
		computers in pad mode after raising the airframe to
		vertical.

		Fire Igniters::

		Remotely control igniters for ground testing recovery
		systems.

	=== AltosDroid Flight Monitoring

		AltosDroid is designed to mimic the AltosUI flight
		monitoring display, providing separate tabs for each
		stage of your rocket flight along with a tab
		containing a map of the local area with icons marking
		the current location of the altimeter and the Android
		device.
      
	=== Pad

		The 'Pad' tab shows information used to decide when
		the rocket is ready for flight. The first elements
		include red/green indicators, if any of these is red,
		you'll want to evaluate whether the rocket is ready to
		launch.

		When the pad tab is selected, the voice responses will
		include status changes to the igniters and GPS
		reception, letting you know if the rocket is still
		ready for launch.
	
		Battery::

		This indicates whether the Li-Po battery powering the
		transmitter has sufficient charge to last for the
		duration of the flight. A value of more than 3.8V is
		required for a 'GO' status.
	      
		Receiver Battery::

		This indicates whether the Li-Po battery powering the
		TeleBT has sufficient charge to last for the duration
		of the flight. A value of more than 3.8V is required
		for a 'GO' status.
	      
		Data Logging::

		This indicates whether there is space remaining
		on-board to store flight data for the upcoming
		flight. If you've downloaded data, but failed to erase
		flights, there may not be any space left. TeleMetrum
		and TeleMega can store multiple flights, depending on
		the configured maximum flight log size. TeleGPS logs
		data continuously. TeleMini v1.0 stores only a single
		flight, so it will need to be downloaded and erased
		after each flight to capture data. This only affects
		on-board flight logging; the altimeter will still
		transmit telemetry and fire ejection charges at the
		proper times.
	      
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
	      
		Apogee Igniter::

		This indicates whether the apogee igniter has
		continuity. If the igniter has a low resistance, then
		the voltage measured here will be close to the Li-Po
		battery voltage. A value greater than 3.2V is required
		for a 'GO' status.
	      
		Main Igniter::

		This indicates whether the main igniter has
		continuity. If the igniter has a low resistance, then
		the voltage measured here will be close to the Li-Po
		battery voltage. A value greater than 3.2V is required
		for a 'GO' status.
	      
		Igniter A-D::

		This indicates whether the indicated additional pyro
		channel igniter has continuity. If the igniter has a
		low resistance, then the voltage measured here will be
		close to the Li-Po battery voltage. A value greater
		than 3.2V is required for a 'GO' status.

		The Pad tab also shows the location of the Android
		device.
	
	=== Flight

		The 'Flight' tab shows information used to evaluate
		and spot a rocket while in flight. It displays speed
		and height data to monitor the health of the rocket,
		along with elevation, range and bearing to help locate
		the rocket in the sky.

		While the Flight tab is displayed, the voice
		announcements will include current speed, height,
		elevation and bearing information.

		Speed::

		Shows current vertical speed. During descent, the
		speed values are averaged over a fairly long time to
		try and make them steadier.
	      
		Height::

		Shows the current height above the launch pad.

		Max Speed::

		Shows the maximum vertical speed seen during the
		flight.

		Max Height::

		Shows the maximum height above launch pad.

		Elevation::

		This is the angle above the horizon from the android
		devices current position.

		Range::
	    
		The total distance from the android device to the
		rocket, including both ground distance and difference
		in altitude. Use this to gauge how large the rocket is
		likely to appear in the sky.

		Bearing::

		This is the azimuth from true north for the rocket
		from the android device. Use this in combination with
		the Elevation value to help locate the rocket in the
		sky, or at least to help point the antenna in the
		general direction. This is provided in both degrees
		and a compass point (like West South West). You'll
		want to know which direction is true north before
		launching your rocket.

		Ground Distance::

		This shows the distance across the ground to the
		lat/lon where the rocket is located. Use this to
		estimate what is currently under the rocket.

		Latitude/Longitude::
	    
		Displays the last known location of the rocket.

		Apogee Igniter::

		This indicates whether the apogee igniter has
		continuity. If the igniter has a low resistance, then
		the voltage measured here will be close to the Li-Po
		battery voltage. A value greater than 3.2V is required
		for a 'GO' status.

		Main Igniter::

		This indicates whether the main igniter has
		continuity. If the igniter has a low resistance, then
		the voltage measured here will be close to the Li-Po
		battery voltage. A value greater than 3.2V is required
		for a 'GO' status.

	=== Recover

		The 'Recover' tab shows information used while
		recovering the rocket on the ground after flight.

		While the Recover tab is displayed, the voice
		announcements will include distance along with either
		bearing or direction, depending on whether you are
		moving.
	
		Bearing::
	    
		This is the azimuth from true north for the rocket
		from the android device. Use this in combination with
		the Elevation value to help locate the rocket in the
		sky, or at least to help point the antenna in the
		general direction. This is provided in both degrees
		and a compass point (like West South West). You'll
		want to know which direction is true north before
		launching your rocket.
	      
		Direction::
	    
		When you are in motion, this provides the angle from
		your current direction of motion towards the rocket.

		Distance::

		Distance over the ground to the rocket.
	      
		Tar Lat/Tar Lon::

		Displays the last known location of the rocket.
	      
		My Lat/My Lon::
	    
		Displays the location of the Android device.
	      
		Max Height::

		Shows the maximum height above launch pad seen during
		the flight.
	      
		Max Speed::

		Shows the maximum vertical speed seen during the
		flight.
	      
		Max Accel::

		Shows the maximum vertical acceleration seen during
		the flight.

	=== Map

		The 'Map' tab shows a map of the area around the
		rocket being tracked along with information needed to
		recover it.

		On the map itself, icons showing the location of the
		android device along with the last known location of
		each tracker. A blue line is drawn from the android
		device location to the currently selected tracker.

		Below the map, the distance and either bearing or
		direction along with the lat/lon of the target and the
		android device are shown

		The Map tab provides the same voice announcements as
		the Recover tab.
	
	=== Downloading Flight Logs

		AltosDroid always saves every bit of telemetry data it
		receives. To download that to a computer for use with
		AltosUI, remove the SD card from your Android device,
		or connect your device to your computer's USB port and
		browse the files on that device. You will find
		'.telem' files in the TeleMetrum directory that will
		work with AltosUI directly.
