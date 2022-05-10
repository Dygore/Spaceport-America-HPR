== Using Altus Metrum Hardware

	Here are general instructions for hooking up an Altus Metrum
	flight computer. Instructions specific to each model will be
	found in the section devoted to that model below.

	=== Wiring and Electrical Interference

		To prevent electrical interference from affecting the
		operation of the flight computer, it's important to always
		twist pairs of wires connected to the board. Twist the switch
		leads, the pyro leads and the battery leads. This reduces
		interference through a mechanism called common mode rejection.

	=== Hooking Up Lithium Polymer Batteries

		All Altus Metrum flight computers have a two pin JST PH
		series connector to connect up a single-cell Lithium Polymer
		cell (3.7V nominal). You can purchase matching batteries
		from the Altus Metrum store, or other vendors, or you can
		make your own. Pin 1 of the connector is positive, pin 2 is
		negative. Spark Fun sells a cable with the connector
		attached, which they call a
		link:https://www.sparkfun.com/products/9914[JST Jumper 2 Wire Assembly]

		[WARNING]
		Many RC vendors also sell lithium polymer batteries with
		this same connector. All that we have found use the opposite
		polarity, and if you use them that way, you will damage or
		destroy the flight computer.

	=== Hooking Up Pyro Charges

		Altus Metrum flight computers always have two screws for
		each pyro charge. This means you shouldn't need to put two
		wires into a screw terminal or connect leads from pyro
		charges together externally.

		On the flight computer, one lead from each charge is hooked
		to the positive battery terminal through the power switch.
		The other lead is connected through the pyro circuit, which
		is connected to the negative battery terminal when the pyro
		circuit is fired.

	=== Hooking Up a Power Switch

		Altus Metrum flight computers need an external power switch
		to turn them on. This disconnects both the computer and the
		pyro charges from the battery, preventing the charges from
		firing when in the Off position. The switch is in-line with
		the positive battery terminal.

	=== Understanding Beeps

		Altus Metrum flight computers include a beeper to
		provide information about the state of the system.
		ifdef::telemini[]
		TeleMini doesn't have room for a beeper, so instead it
		uses an LED, which works the same, except for every
		beep is replaced with the flash of the LED.
		endif::telemini[]

		Here's a short summary of all of the modes and the
		beeping
		ifdef::telemini[(or flashing, in the case of TeleMini v1)]
		that accompanies each mode. In the description of the
		beeping pattern, “dit” means a short beep while "dah"
		means a long beep (three times as long). “Brap” means
		a long dissonant tone.

		.AltOS Modes
		[options="border",cols="1,1,2,2"]
		|====
		|Mode Name
		|Abbreviation
		|Beeps
		|Description

		|Startup
		|S
		|battery voltage in decivolts
		|Calibrating sensors, detecting orientation.

		|Idle
		|I
		|dit dit
		|Ready to accept commands over USB
		ifdef::radio[or radio link.]

		|Pad
		|P
		|dit dah dah dit
		|Waiting for launch. Not listening for commands.

		|Boost
		|B
		|dah dit dit dit
		|Accelerating upwards.

		|Fast
		|F
		|dit dit dah dit
		|Decelerating, but moving faster than 200m/s.

		|Coast
		|C
		|dah dit dah dit
		|Decelerating, moving slower than 200m/s

		|Drogue
		|D
		|dah dit dit
		|Descending after apogee. Above main height.

		|Main
		|M
		|dah dah
		|Descending. Below main height.

		|Landed
		|L
		|dit dah dit dit
		|Stable altitude for at least ten seconds.


		|Sensor error
		|X
		|dah dit dit dah
		|Error detected during sensor calibration.
		|====

		Here's a summary of all of the Pad and Idle mode
		indications. In Idle mode, you'll hear one of these
		just once after the two short dits indicating idle
		mode. In Pad mode, after the dit dah dah dit
		indicating Pad mode, you'll hear these once every five
		seconds.

		.Pad/Idle Indications
		[options="header",cols="1,1,3"]
		|====
		|Name		|Beeps		|Description

		|Neither
		|brap
		|No continuity detected on either apogee or main igniters.

		|Apogee
		|dit
		|Continuity detected only on apogee igniter.

		|Main
		|dit dit
		|Continuity detected only on main igniter.


		|Both
		|dit dit dit
		|Continuity detected on both igniters.


		|Storage Full
		|warble
		|On-board data logging storage is full. This will
		 not prevent the flight computer from safely
		 controlling the flight or transmitting telemetry
		 signals, but no record of the flight will be
		 stored in on-board flash.

		ifdef::easymega,telemega,easytimer[]
		|Additional Igniters
		|four very short beeps
		|Continuity indication for the additional pyro
		 channels on TeleMega, EasyMega and EasyTimer. One high tone for
		 no continuity, one low tone for continuity. On TeleMega and EasyMegay, these are
		 produced after the continuity indicators for the two
		 primary igniter channels.
		endif::easymega,telemega,easytimer[]

		|====

		ifdef::radio[]
		For devices with a radio transmitter, in addition to
		the digital and APRS telemetry signals, you can also
		receive audio tones with a standard amateur
		70cm FM receiver. While on the pad, you will hear
		igniter status once every five seconds.

		.Pad Radio Indications
		[options="header",cols="1,1,3"]
		|====
		|Name		|Beeps		|Description

		|Neither
		|½ second tone
		|No continuity detected on either apogee or main igniters.

		|Apogee
		|dit
		|Continuity detected only on apogee igniter.

		|Main
		|dit dit
		|Continuity detected only on main igniter.


		|Both
		|dit dit dit
		|Continuity detected on both igniters.

		|====

		During ascent, the tones will be muted to allow the
		telemetry data to consume the full radio bandwidth.

		During descent and after landing, a ½ second tone will
		be transmitted every five seconds. This can be used to
		find the rocket using RDF techniques when the signal
		is too weak to receive GPS information via telemetry
		or APRS.
		endif::radio[]

	=== Turning On the Power

		Connect a battery and power switch and turn the switch
		to "on". The flight computer will signal power on by
		reporting the battery voltage and then perform an internal self
		test and sensor calibration.

		Once the self test and calibration are complete, there
		are two modes that an Altus Metrum flight computer can
		operate in:

		Flight/Pad::
		The flight computer is waiting to detect
		launch and then fly the rocket. In this mode, the USB
		link is
		ifdef::radio[disabled, and the radio goes into transmit-only mode.]
		ifndef::radio[disabled.]
		The only way to get out of this
		mode is to power the flight computer down. See below for how to get the flight
		computer to come up in Flight/Pad mode at power on.

		Idle::
		The flight computer is ready to communicate over USB
		ifdef::radio[and in packet mode over the radio.]
		You can configure
		the flight computer, download data or display
		the current state. See below for how to get the flight
		computer to come up in Idle mode at power on.

		ifdef::telemetrum,easymega,telemega,easytimer[]
		For flight computers with accelerometers (TeleMetrum,
		EasyMega, TeleMega and EasyTimer), the mode is selected by the
		orientation of the board during the self test
		interval. If the board is pointing upwards as if ready
		to fly, it will enter Flight/Pad mode. Otherwise, it will
		enter Idle mode.
		endif::telemetrum,easymega,telemega,easytimer[]

		ifdef::easymini[]
		For EasyMini, if the USB cable is connected to a
		computer, it will enter Idle mode. Otherwise, it will
		enter Flight/Pad mode.
		endif::easymini[]

		ifdef::telemini[]
		For TeleMini v1.0, if a packet link is waiting to
		connect when the device is powered on, it will enter
		Idle mode, otherwise it will enter Flight/Pad mode.
		endif::telemini[]

		You can see in <<_understanding_beeps>>
		how to tell which mode the flight computer is in.

	=== Using an External Active Switch Circuit

		You can use an active switch circuit, such as the
		Featherweight Magnetic Switch, with any Altus Metrum
		flight computer. These require three connections, one to
		the battery, one to the positive power input on the flight
		computer and one to ground. Find instructions on how to
		hook these up for each flight computer below. Then follow
		the instructions that come with your active switch to
		connect it up.

	=== Using a Separate Pyro Battery

		As mentioned above in <<_hooking_up_pyro_charges>>, one
		lead for each of the pyro charges is connected through
		the power switch directly to the positive battery
		terminal. The other lead is connected to the pyro
		circuit, which connects it to the negative battery
		terminal when the pyro circuit is fired. The pyro
		circuit on all of the flight computers is designed to
		handle up to 16V.

		To use a separate pyro battery, connect the negative pyro
		battery terminal to the flight computer ground terminal,
		the the switched positive battery terminal to the igniter 
		and the other
		igniter lead to the negative pyro terminal on the flight
		computer. When the pyro channel fires, it will complete the
		circuit between the negative pyro terminal and the ground
		terminal, firing the igniter. Specific instructions on how
		to hook this up for each flight computer will be found
		in the section below for that flight computer.

		Note that you must include a switch in the positive lead of 
		the pyro battery for safety, as the on-board power switch 
		circuit on our product only controls current flow from the 
		the primary or system battery!


	=== Using a Different Kind of Battery

		EasyMini
		ifdef::easytimer[and EasyTimer are]
		ifndef::easytimer[is]
		designed to use either a
		lithium polymer battery or any other battery producing
		between 4 and 12 volts, such as a rectangular 9V
		battery.

		ifdef::telemega,easymega,telemetrum,telemini[]
		[WARNING]
		ifdef::telemini[TeleMini, ]
		TeleMega, EasyMega and TeleMetrum are only designed to
		operate off a single-cell Lithium Polymer battery and
		cannot be used with any other kind. Connecting a
		different kind of battery to any of these will destroy
		the board.
		endif::telemega,easymega,telemetrum,telemini[]

	=== Using Packet Link Mode

		All AltusMetrum flight computers that have a radio can
		communicate with the ground station software for
		configuration and other operations using the Packet
		Link mode. This uses radio communication instead of a
		USB cable. To set this up, the ground station software
		must be configured to the correct data rate, frequency
		and callsign.

		You can monitor Packet Link mode from TeleBT or
		TeleDongle by watching the LEDs. Each time the device
		transmits, the red LED will flash. When the link is
		busy, or when the link is not working, the device will
		transmit 10 times per second, so the LED will flash
		rapidly. When the link is working and there is no data
		to send, the link will flash once per second, and the
		LED will flash more slowly.
