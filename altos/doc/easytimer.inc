== EasyTimer

	.EasyTimer Board
	image::easytimer.jpg[width=400]

	EasyTimer is built on a 0.8 inch by 1½ inch circuit board. It's
	designed to fit in a 24mm coupler tube.

	EasyTimer is designed to control events during ascent. It has
	an accelerometer and gyroscope that can measure acceleration
	and rotation and compute speed and tilt angle. EasyTimer has
	two pyro channels which can be configured to fire at various
	points during flight. Because EasyTimer has no barometric
	sensor, it cannot be used to fire recovery charges at apogee
	or during descent. EasyTimer is configured using the AltosUI
	application which is available for Linux, Mac OS X and Windows.

	=== EasyTimer Screw Terminals
	
		EasyTimer has two sets of four screw terminals near
		one end of the board. Using the picture above, the top
		four have connections for pyro channel B and an
		external battery and the bottom four have connections
		for pyro circuit A and the power switch. Counting from
		the left, the connections are as follows:

		.EasyTimer Screw Terminals
		[options="header",grid="all",cols="2,3,10"]
		|====
		|Terminal #|Terminal Name|Description
		|Top 1
		|B -
		|Pyro channel B connection to pyro circuit
	      
		|Top 2
		|B +++
		|Pyro channel B common connection to battery +++
	      
		|Top 3
		|Battery +++
		|Positive external battery terminal
	      
		|Top 4
		|Battery -
		|Negative external battery terminal
	      
		|Bottom 1
		|A -
		|Pyro channel A connection to pyro circuit
	      
		|Bottom 2
		|A +++
		|Pyro channel A common connection to battery +++
	      
		|Bottom 3
		|Switch Output
		|Switch connection to flight computer
	      
		|Bottom 4
		|Switch Input
		|Switch connection to positive battery terminal
		|====

	=== Connecting A Battery To EasyTimer

		There are two possible battery connections on
		EasyTimer. You can use either method; both feed
		through the power switch terminals.

		One battery connection is the standard Altus Metrum
		white JST plug. This mates with single-cell Lithium
		Polymer batteries sold by Altus Metrum.

		The other is a pair of screw terminals marked 'Battery
		+' and 'Battery -'. Connect a battery from 4 to 12
		volts to these terminals, being careful to match polarity.

	=== Charging Lithium Batteries

		Because EasyTimer allows for batteries other than the
		standard Altus Metrum Lithium Polymer cells, it cannot
		incorporate a battery charger circuit. Therefore, when
		using a Litium Polymer cell, you'll need an external
		charger. These are available from Altus Metrum, or
		from Spark Fun.

      	=== Using a Separate Pyro Battery with EasyTimer
	
		As described above, using an external pyro battery involves
		connecting the negative battery terminal to the flight
		computer ground, connecting the positive battery terminal to
		one of the igniter leads and connecting the other igniter
		lead to the per-channel pyro circuit connection.

		To connect the negative pyro battery terminal to EasyTimer
		ground, connect it to the negative external battery
		connection, top terminal 4.

		Connecting the switched positive battery terminal to the pyro
		charges must be done separate from EasyTimer, by soldering
		them together or using some other connector.  Note that for
		safety, you must put a switch between the pyro battery and
		the rest of the circuit!

		The other lead from each pyro charge is then inserted into
		the appropriate per-pyro channel screw terminal (top
		terminal 1 for pyro channel A charge, bottom terminal 1 for
		pyro channel B charge).

      	=== Using an Active Switch with EasyTimer
	
		As explained above, an external active switch requires three
		connections, one to the positive battery terminal, one to
		the flight computer positive input and one to ground. Use
		the negative external battery connection, top terminal 4 for
		ground.

		The positive battery terminal is available on bottom
		terminal 4, the positive flight computer input is on the
		bottom terminal 3.
