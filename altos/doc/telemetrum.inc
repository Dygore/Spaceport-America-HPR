== TeleMetrum

	.TeleMetrum v2 Board
	image::telemetrum-v2.0-th.jpg[width=400]

	.TeleMetrum v1 Board
	image::telemetrum-v1.1-thside.jpg[width=400]

	TeleMetrum is a 1 inch by 2¾ inch circuit board.  It was designed to
	fit inside coupler for 29mm air-frame tubing, but using it in a tube that
	small in diameter may require some creativity in mounting and wiring
	to succeed!  The presence of an accelerometer means TeleMetrum should
	be aligned along the flight axis of the airframe, and by default the ¼
	wave UHF wire antenna should be on the nose-cone end of the board.  The
	antenna wire is about 7 inches long, and wiring for a power switch and
	the e-matches for apogee and main ejection charges depart from the
	fin can end of the board, meaning an ideal “simple” avionics
	bay for TeleMetrum should have at least 10 inches of interior length.

	There are two generations of the TeleMetrum design. The
	major changes after v1 generation are:

	* uBlox GPS chip certified for altitude records

	* Higher power radio (40mW vs 10mW)

	* APRS support

	Otherwise, they're the same size, with mounting holes and
	screw terminals in the same position.

	=== TeleMetrum Screw Terminals

		TeleMetrum has six screw terminals on the end of the board
		opposite the telemetry antenna. Two are for the power
		switch, and two each for the apogee and main igniter
		circuits. Using the picture above and starting from the top,
		the terminals are as follows:

		.TeleMetrum Screw Terminals
		[options="header",grid="all",cols="2,3,10"]
		|===
		|Terminal #|Terminal Name|Description
		|1	|Switch Output	|Switch connection to flight computer
		|2	|Switch Input	|Switch connection to positive battery terminal
		|3	|Main +++	|Main pyro channel common connection to battery +++
		|4	|Main -		|Main pyro channel connection to pyro circuit
		|5	|Apogee +++	|Apogee pyro channel common connection to battery +++
		|6	|Apogee -	|Apogee pyro channel connection to pyro circuit
		|===

	=== Using a Separate Pyro Battery with TeleMetrum

		As described above, using an external pyro battery involves
		connecting the negative battery terminal to the flight
		computer ground, connecting the positive battery terminal to
		one of the igniter leads and connecting the other igniter
		lead to the per-channel pyro circuit connection.

		To connect the negative battery terminal to the TeleMetrum
		ground, insert a small piece of wire, 24 to 28 gauge
		stranded, into the GND hole just above the screw terminal
		strip and solder it in place.

		Connecting the switched positive battery terminal to the pyro
		charges must be done separate from TeleMetrum, by soldering
		them together or using some other connector. Note that you 
		must include a switch in the positive lead from the pyro 
		battery for safety!

		The other lead from each pyro charge is then inserted into
		the appropriate per-pyro channel screw terminal (terminal 4 
		for the Main charge, terminal 6 for the Apogee charge).

	=== Using an Active Switch with TeleMetrum

		As explained above, an external active switch requires three
		connections, one to the positive battery terminal, one to
		the flight computer positive input and one to ground.


		The positive battery terminal is available on screw terminal
		2, the positive flight computer input is on terminal 1. To
		hook a lead to ground, solder a piece of wire, 24 to 28
		gauge stranded, to the GND hole just above terminal 1.

