== EasyMega

	.EasyMega Board
	image::easymega-v1.0-top.jpg[width=400]

	EasyMega is a 1¼ inch by 2¼ inch circuit board. It was
	designed to easily fit in a 38mm coupler. Like TeleMetrum,
	EasyMega has an accelerometer and so it must be mounted so that
	the board is aligned with the flight axis. It can be mounted
	either antenna up or down.

	=== EasyMega Screw Terminals

		EasyMega has two sets of nine screw terminals on the end of
		the board opposite the telemetry antenna. They are as follows:

		.EasyMega Screw Terminals
		[options="header",grid="all",cols="2,3,10"]
		|====
		|Terminal #|Terminal Name|Description

		|Top 1
		|Switch Input
		|Switch connection to positive battery terminal

		|Top 2
		|Switch Output
		|Switch connection to flight computer

		|Top 3
		|GND
		|Ground connection for use with external active switch

		|Top 4
		|Main -
		|Main pyro channel connection to pyro circuit

		|Top 5
		|Main +++
		|Main pyro channel common connection to battery +++

		|Top 6
		|Apogee -
		|Apogee pyro channel connection to pyro circuit

		|Top 7
		|Apogee +++
		|Apogee pyro channel common connection to battery +++

		|Top 8
		|D -
		|D pyro channel connection to pyro circuit

		|Top 9
		|D +++
		|D pyro channel common connection to battery +++

		|Bottom 1
		|GND
		|Ground connection for negative pyro battery terminal

		|Bottom 2
		|Pyro
		|Positive pyro battery terminal

		|Bottom 3
		|Lipo
		|Power switch output. Use to connect main battery to pyro battery input

		|Bottom 4
		|A -
		|A pyro channel connection to pyro circuit

		|Bottom 5
		|A +++
		|A pyro channel common connection to battery +++

		|Bottom 6
		|B -
		|B pyro channel connection to pyro circuit

		|Bottom 7
		|B +++
		|B pyro channel common connection to battery +++

		|Bottom 8
		|C -
		|C pyro channel connection to pyro circuit

		|Bottom 9
		|C +++
		|C pyro channel common connection to battery +++
		|====

	=== Using a Separate Pyro Battery with EasyMega

		EasyMega provides explicit support for an external pyro
		battery. All that is required is to remove the jumper
		between the lipo terminal (Bottom 3) and the pyro terminal
		(Bottom 2). Then hook the negative pyro battery terminal to 
		ground (Bottom 1) and the switched positive pyro battery to 
		the pyro battery input (Bottom 2). Note that you must include
		a switch between the pyro battery and the board for safety,
		as the on-board power switch circuit only supports the primary
		battery!  You can then use the existing pyro screw
		terminals to hook up all of the pyro charges.

      	=== Using Only One Battery With EasyMega

		Because EasyMega has built-in support for a separate pyro
		battery, if you want to fly with just one battery running
		both the computer and firing the charges, you need to
		connect the flight computer battery to the pyro
		circuit. EasyMega has two screw terminals for this—hook a
		wire from the Lipo terminal (Bottom 3) to the Pyro terminal
		(Bottom 2).

      	=== Using an Active Switch with EasyMega

		As explained above, an external active switch requires three
		connections, one to the positive battery terminal, one to
		the flight computer positive input and one to ground.

		The positive battery terminal is available on Top terminal
		1, the positive flight computer input is on Top terminal
		2. Ground is on Top terminal 3.
