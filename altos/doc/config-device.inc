ifdef::altusmetrum[]

	==== Main Deploy Altitude

		This sets the altitude (above the recorded pad
		altitude) at which the 'main' igniter will fire. The
		drop-down menu shows some common values, but you can
		edit the text directly and choose whatever you
		like. If the apogee charge fires below this altitude,
		then the main charge will fire two seconds after the
		apogee charge fires.

	==== Apogee Delay

		When flying redundant electronics, it's often
		important to ensure that multiple apogee charges don't
		fire at precisely the same time, as that can over
		pressurize the apogee deployment bay and cause a
		structural failure of the air-frame. The Apogee Delay
		parameter tells the flight computer to fire the apogee
		charge a certain number of seconds after apogee has
		been detected.

	==== Apogee Lockout

		Apogee lockout is the number of seconds after launch
		where the flight computer will not fire the apogee
		charge, even if the rocket appears to be at
		apogee. This is often called 'Mach Delay', as it is
		intended to prevent a flight computer from
		unintentionally firing apogee charges due to the
		pressure spike that occurrs across a mach
		transition. Altus Metrum flight computers include a
		Kalman filter which is not fooled by this sharp
		pressure increase, and so this setting should be left
		at the default value of zero to disable it.

		[WARNING]
		Firmware versions older than 1.8.6 have a
		bug which resets the time since launch to zero each
		time a motor starts burning. Update firmware to get
		the correct behavior.

endif::altusmetrum[]

ifdef::radio[]
	==== Frequency

		This configures which of the frequencies to use for
		both telemetry and packet command mode. Note that if
		you set this value via packet command mode, the
		TeleDongle frequency will also be automatically
		reconfigured to match so that communication will
		continue afterwards.

	==== RF Calibration

		The radios in every Altus Metrum device are calibrated
		at the factory to ensure that they transmit and
		receive on the specified frequency.  If you need to
		you can adjust the calibration by changing this value.
		Do not do this without understanding what the value
		means, read the appendix on calibration and/or the
		source code for more information.  To change a
		TeleDongle's calibration, you must reprogram the unit
		completely.

	==== Telemetry/RDF/APRS Enable

		Enables the radio for transmission during
		flight. When disabled, the radio will not
		transmit anything during flight at all.

	==== Telemetry baud rate

		This sets the modulation bit rate for data
		transmission for both telemetry and packet
		link mode. Lower bit rates will increase range
		while reducing the amount of data that can be
		sent and increasing battery consumption. All
		telemetry is done using a rate 1/2 constraint
		4 convolution code, so the actual data
		transmission rate is 1/2 of the modulation bit
		rate specified here.

	==== APRS Interval

		How often to transmit GPS information via APRS
		(in seconds). When set to zero, APRS
		transmission is disabled.
		ifdef::altusmetrum[]
		This option is
		available on TeleMetrum v2 or newer and TeleMega
		boards. TeleMetrum v1 boards cannot transmit
		APRS packets.
		endif::altusmetrum[]
		Note that a single APRS packet
		takes nearly a full second to transmit, so
		enabling this option will prevent sending any
		other telemetry during that time.

	==== APRS SSID

		Which SSID to report in APRS packets. By
		default, this is set to the last digit of the
		serial number, but can be configured to any
		value from 0 to 9.

	==== APRS Format

		Whether to send APRS data in Compressed or
		Uncompressed format. Compressed format is
		smaller and more precise. Uncompressed
		format is older, but may work better with your
		device. The Kenwood TH-D72 only displays
		altitude information with Uncompressed
		format, while the Yaesu FT1D only displays
		altitude with Compressed format. Test before
		you fly to see which to use.

	==== APRS Offset
	
		The delay from the top of the minute before sending
		the first APRS packet of the minute. Coordinating
		values for this parameter between multiple devices can
		allow a single receiver to reliably receive APRS
		packets from multiple devices. Note that this offset only
		takes effect while the GPS signal is locked so that the
		transmitting device knows the current time.
		
	==== Callsign

		This sets the call sign included in each
		telemetry packet. Set this as needed to
		conform to your local radio regulations.
endif::radio[]

ifdef::altusmetrum[]

	==== Maximum Flight Log Size

		This sets the space (in kilobytes) allocated
		for each flight log. The available space will
		be divided into chunks of this size. A smaller
		value will allow more flights to be stored, a
		larger value will record data from longer
		flights.

	==== Ignitor Firing Mode

		This configuration parameter allows the two standard ignitor
		channels (Apogee and Main) to be used in different
		configurations.

		Dual Deploy::
		This is the usual mode of operation; the
		'apogee' channel is fired at apogee and the
		'main' channel at the height above ground
		specified by the 'Main Deploy Altitude' during
		descent.

		Redundant Apogee::
		This fires both channels at apogee, the
		'apogee' channel first followed after a two
		second delay by the 'main' channel.

		Redundant Main::
		This fires both channels at the height above
		ground specified by the Main Deploy Altitude
		setting during descent. The 'apogee' channel
		is fired first, followed after a two second
		delay by the 'main' channel.

		Separation & Apogee::
		This fires the 'main' channel when the first motor
		burns out and fires the 'apogee' charge at apogee.

	ifdef::telemetrum,telemega,easymega,easytimer[]
	==== Pad Orientation

		Because they include accelerometers,
		TeleMetrum, TeleMega and EasyMega are
		sensitive to the orientation of the board. By
		default, they expect the antenna end to point
		forward. This parameter allows that default to
		be changed, permitting the board to be mounted
		with the antenna pointing aft instead.

		Antenna Up::
		In this mode, the antenna (or beeper, for devices
		without an antenna) of the flight computer must point
		forward, in line with the expected flight path.

		Antenna Down::

		In this mode, the antenna (or beeper, for devices
		without an antenna) end of the flight computer must
		point aft, in line with the expected flight path.
		endif::telemetrum,telemega,easymega,easytimer[]

	==== Beeper Frequency

		The beeper on all Altus Metrum flight
		computers works best at 4000Hz, however if you
		have more than one flight computer in a single
		airframe, having all of them sound at the same
		frequency can be confusing. This parameter
		lets you adjust the base beeper frequency
		value.

endif::altusmetrum[]

ifdef::telegps[]
	==== Logging Trigger Motion

		This sets the amount of motion that TeleGPS
		needs to see before logging the new
		position. Motions smaller than this are
		skipped, which saves storage space.

	==== Position Reporting Interval

		The interval between TeleGPS position reports,
		both over the air and in the log. Increase
		this to reduce the frequency of radio
		transmissions and the length of time available
		in the log.
endif::telegps[]

ifdef::telemega,easymega,easytimer,telemetrum[]

	==== Calibrate Accelerometer

		This opens a separate window to recalibrate the
		accelerometers. Follow the instructions, orienting the
		flight computer with the antenna end, or end opposite
		the screw terminals, in the case of EasyMega, first up
		and then down.

		When the calibration is complete, return to the
		Configure Altimeter window and save the new
		calibration values.

endif::telemega,easymega,easytimer,telemetrum[]

ifdef::telemega,easymega,easytimer[]

	==== Configure Pyro Channels

		.Additional Pyro Channel Configuration
		image::configure-pyro.png[width=400]

		This opens a separate window to configure the
		additional pyro channels available on TeleMega,
		EasyMega and EasyTimer.  One column is presented for
		each channel. Each row represents a single
		parameter, if enabled the parameter must meet
		the specified test for the pyro channel to be
		fired.

		Select conditions and set the related value;
		the pyro channel will be activated when *all*
		of the conditions are met. Each pyro channel
		has a separate set of configuration values, so
		you can use different values for the same
		condition with different channels.

		At the bottom of the window, the 'Pyro Firing
		Time' configuration sets the length of time
		(in seconds) which each of these pyro channels
		will fire for.

		Once you have selected the appropriate
		configuration for all of the necessary pyro
		channels, you can save the pyro configuration
		along with the rest of the flight computer
		configuration by pressing the 'Save' button in
		the main Configure Flight Computer window.

		include::pyro-channels.adoc[]

endif::telemega,easymega,easytimer[]
