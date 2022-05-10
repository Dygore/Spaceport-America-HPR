[appendix]
== System Operation

	=== Firmware Modes

		The AltOS firmware build for the altimeters has two
		fundamental modes, “idle” and “flight”.  Which of these modes
		the firmware operates in is determined at start up
		time.
		ifdef::telemetrum,telemega,easymega,easytimer[]
		For
		TeleMetrum, TeleMega, EasyMega and EasyTimer, which have accelerometers, the mode is
		controlled by the orientation of the
		rocket (well, actually the board, of course...) at the time
		power is switched on.  If the rocket is “nose up”, then
		the flight computer assumes it's on a rail or rod being prepared for
		launch, so the firmware chooses flight mode.  However, if the
		rocket is more or less horizontal, the firmware instead enters
		idle mode.
		endif::telemetrum,telemega,easymega,easytimer[]
		Since
		EasyMini doesn't
		have an
		accelerometer we can use to determine orientation, “idle” mode
		is selected if the board is connected via USB to a computer,
		otherwise the board enters “flight” mode.
		ifdef::telemini[]
		TeleMini
		selects “idle” mode if it receives a command packet
		within the
		first five seconds of operation.
		endif::telemini[]

		At power on, the altimeter will beep out the battery voltage
		to the nearest tenth of a volt.  Each digit is represented by
		a sequence of short “dit” beeps, with a pause between
		digits. A zero digit is represented with one long “dah”
		beep. Then there will be a short pause while the altimeter
		completes initialization and self test, and decides which mode
		to enter next.

		In flight or “pad” mode, the altimeter engages the flight
		state machine, goes into transmit-only mode to send telemetry,
		and waits for launch to be detected.  Flight mode is indicated
		by an “di-dah-dah-dit” (“P” for pad) on the beeper or lights,
		followed by beeps or flashes indicating the state of the
		pyrotechnic igniter continuity.  One beep/flash indicates
		apogee continuity, two beeps/flashes indicate main continuity,
		three beeps/flashes indicate both apogee and main continuity,
		and one longer “brap” sound which is made by rapidly
		alternating between two tones indicates no continuity.  For a
		dual deploy flight, make sure you're getting three beeps or
		flashes before launching!  For apogee-only or motor eject
		flights, do what makes sense.

		If idle mode is entered, you will hear an audible “di-dit” or
		see two short flashes (“I” for idle), and the flight state
		machine is disengaged, thus no ejection charges will fire.
		ifdef::radio[]
		The altimeters also listen for the radio link when in idle
		mode for requests sent via TeleDongle.  Commands can be issued
		in idle mode over either USB or the radio link
		equivalently.
		ifdef::telemini[TeleMini only has the radio link.]
		endif::radio[]
		Idle mode is useful for configuring the altimeter, for
		extracting data from the on-board storage chip after
		flight, and for ground testing pyro charges.

		In “Idle” and “Pad” modes, once the mode indication
		beeps/flashes and continuity indication has been sent, if
		there is no space available to log the flight in on-board
		memory, the flight computer will emit a warbling tone (much
		slower than the “no continuity tone”)

		See <<_understanding_beeps>> for a summary of all of
		the audio signals used.

		Once landed, the flight computer will signal that by emitting
		the “Landed” sound described above, after which it will beep
		out the apogee height (in meters). Each digit is represented
		by a sequence of short “dit” beeps, with a pause between
		digits. A zero digit is represented with one long “dah”
		beep. The flight computer will continue to report landed mode
		and beep out the maximum height until turned off.

		ifdef::telemetrum,telemega,easymega[]
		One “neat trick” of particular value when TeleMetrum, TeleMega
		or EasyMega are used with
		very large air-frames, is that you can power the board up while the
		rocket is horizontal, such that it comes up in idle mode.  Then you can
		raise the air-frame to launch position, and issue a 'reset' command
		via TeleDongle over the radio link to cause the altimeter to reboot and
		come up in flight mode.  This is much safer than standing on the top
		step of a rickety step-ladder or hanging off the side of a launch
		tower with a screw-driver trying to turn on your avionics before
		installing igniters!
		endif::telemetrum,telemega,easymega[]

		ifdef::telemini[]
		TeleMini is configured solely via the radio link. Of course, that
		means you need to know the TeleMini radio configuration values
		or you won't be able to communicate with it. For situations
		when you don't have the radio configuration values,
		TeleMini v1.0
		offers an 'emergency recovery' mode. In this mode,
		TeleMini v1.0 is
		configured as follows:


		 * Sets the radio frequency to 434.550MHz
		 * Sets the radio calibration back to the factory value.
		 * Sets the callsign to N0CALL
		 * Does not go to 'pad' mode after five seconds.

		To get into 'emergency recovery' mode, first find the row of
		four small holes opposite the switch wiring. Using a short
		piece of small gauge wire, connect the outer two holes
		together, then power TeleMini up. Once the red LED is lit,
		disconnect the wire and the board should signal that it's in
		'idle' mode after the initial five second startup
		period.
		endif::telemini[]

	ifdef::gps[]
    	=== GPS

		TeleMetrum and TeleMega include a complete GPS receiver.  A
		complete explanation of how GPS works is beyond the scope of
		this manual, but the bottom line is that the GPS receiver
		needs to lock onto at least four satellites to obtain a solid
		3 dimensional position fix and know what time it is.

		The flight computers provide backup power to the GPS chip any time a
		battery is connected.  This allows the receiver to “warm start” on
		the launch rail much faster than if every power-on were a GPS
		“cold start”.  In typical operations, powering up
		on the flight line in idle mode while performing final air-frame
		preparation will be sufficient to allow the GPS receiver to cold
		start and acquire lock.  Then the board can be powered down during
		RSO review and installation on a launch rod or rail.  When the board
		is turned back on, the GPS system should lock very quickly, typically
		long before igniter installation and return to the flight line are
		complete.
	endif::gps[]

	ifdef::radio[]
    	=== Controlling An Altimeter Over The Radio Link

		One of the unique features of the Altus Metrum system is the
		ability to create a two way command link between TeleDongle
		and an altimeter using the digital radio transceivers
		built into each device. This allows you to interact with the
		altimeter from afar, as if it were directly connected to the
		computer.

		Any operation which can be performed with a flight computer can
		either be done with the device directly connected to the
		computer via the USB cable, or through the radio
		link. TeleMini doesn't provide a USB connector and so it is
		always communicated with over radio.  Select the appropriate
		TeleDongle device when the list of devices is presented and
		AltosUI will interact with an altimeter over the radio link.

		One oddity in the current interface is how AltosUI selects the
		frequency for radio communications. Instead of providing
		an interface to specifically configure the frequency, it uses
		whatever frequency was most recently selected for the target
		TeleDongle device in Monitor Flight mode. If you haven't ever
		used that mode with the TeleDongle in question, select the
		Monitor Flight button from the top level UI, and pick the
		appropriate TeleDongle device.  Once the flight monitoring
		window is open, select the desired frequency and then close it
		down again. All radio communications will now use that frequency.

		 * Save Flight Data—Recover flight data from the
	           rocket without opening it up.

		 * Configure altimeter apogee delays, main deploy
		   heights and additional pyro event conditions to
		   respond to changing launch conditions. You can also
		   'reboot' the altimeter. Use this to remotely enable
		   the flight computer by turning TeleMetrum or
		   TeleMega on in “idle” mode, then once the air-frame
		   is oriented for launch, you can reboot the
		   altimeter and have it restart in pad mode without
		   having to climb the scary ladder.

		 * Fire Igniters—Test your deployment charges without snaking
		   wires out through holes in the air-frame. Simply assemble the
		   rocket as if for flight with the apogee and main charges
		   loaded, then remotely command the altimeter to fire the
		   igniters.

		Operation over the radio link for configuring an
		altimeter, ground testing igniters, and so forth uses
		the same RF frequencies as flight telemetry.  To
		configure the desired TeleDongle frequency, select the
		monitor flight tab, then use the frequency selector
		and close the window before performing other desired
		radio operations.

		The flight computers only enable radio commanding in
		'idle' mode.  TeleMetrum and TeleMega use the
		accelerometer to detect which orientation they start
		up in, so make sure you have the flight computer lying
		horizontally when you turn it on. Otherwise, it will
		start in 'pad' mode ready for flight, and will not be
		listening for command packets from TeleDongle.

		TeleMini listens for a command packet for five seconds
		after first being turned on, if it doesn't hear
		anything, it enters 'pad' mode, ready for flight and
		will no longer listen for command packets. The easiest
		way to connect to TeleMini is to initiate the command
		and select the TeleDongle device. At this point, the
		TeleDongle will be attempting to communicate with the
		TeleMini. Now turn TeleMini on, and it should
		immediately start communicating with the TeleDongle
		and the desired operation can be performed.

		You can monitor the operation of the radio link by watching the
		lights on the devices. The red LED will flash each time a packet
		is transmitted, while the green LED will light up on TeleDongle when
		it is waiting to receive a packet from the altimeter.
	endif::radio[]

    	=== Ground Testing

		An important aspect of preparing a rocket using electronic deployment
		for flight is ground testing the recovery system.
		ifdef::radio[]
		Thanks
		to the bi-directional radio link central to the Altus Metrum system,
		this can be accomplished in a TeleMega, TeleMetrum or TeleMini equipped rocket
		with less work than you may be accustomed to with other systems.  It
		can even be fun!
		endif::radio[]

		Just prep the rocket for flight, then power up the altimeter
		in “idle”
		ifdef::telemetrum,telemega,telemini[]
			mode (placing air-frame horizontal for TeleMetrum or TeleMega, or
			selecting the Configure Altimeter tab for TeleMini).
			This will cause
			the firmware to go into “idle” mode, in which the normal flight
			state machine is disabled and charges will not fire without
			manual command.
		endif::telemetrum,telemega,telemini[]
		ifndef::telemetrum,telemega,telemini[]
			mode.
		endif::telemetrum,telemega,telemini[]
		You can now command the altimeter to fire the apogee
		or main charges from a safe distance using your
		computer and the Fire Igniter tab to complete ejection testing.

	ifdef::radio[]
    	=== Radio Link

		TeleMetrum, TeleMini and TeleMega all incorporate an
		RF transceiver, but it's not a full duplex system;
		each end can only be transmitting or receiving at any
		given moment. So we had to decide how to manage the
		link.

		By design, the altimeter firmware listens for the
		radio link when it's in “idle mode”, which allows us
		to use the radio link to configure the rocket, do
		things like ejection tests, and extract data after a
		flight without having to crack open the air-frame.
		However, when the board is in “flight mode”, the
		altimeter only transmits and doesn't listen at all.
		That's because we want to put ultimate priority on
		event detection and getting telemetry out of the
		rocket through the radio in case the rocket crashes
		and we aren't able to extract data later.

		We don't generally use a 'normal packet radio' mode
		like APRS because they're just too inefficient.  The
		GFSK modulation we use is FSK with the base-band
		pulses passed through a Gaussian filter before they go
		into the modulator to limit the transmitted bandwidth.
		When combined with forward error correction and
		interleaving, this allows us to have a very robust
		19.2 kilobit data link with only 10-40 milliwatts of
		transmit power, a whip antenna in the rocket, and a
		hand-held Yagi on the ground.  We've had flights to
		above 21k feet AGL with great reception, and
		calculations suggest we should be good to well over
		40k feet AGL with a 5-element yagi on the ground with
		our 10mW units and over 100k feet AGL with the 40mW
		devices.  We hope to fly boards to higher altitudes
		over time, and would of course appreciate customer
		feedback on performance in higher altitude flights!
	endif::radio[]

	ifdef::gps+radio[]
	:aprsdevices: TeleMetrum v2 and newer and TeleMega
	:configure_section: _configure_altimeter
	include::aprs-operation.adoc[]
	endif::gps+radio[]

    	=== Configurable Parameters

		Configuring an Altus Metrum altimeter for flight is
		very simple.  Even on our baro-only TeleMini and
		EasyMini boards, the use of a Kalman filter means
		there is no need to set a “mach delay”.  All of the
		configurable parameters can be set using AltosUI. Read
		<<_configure_altimeter>> for more information.
