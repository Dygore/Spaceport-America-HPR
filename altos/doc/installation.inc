== Installation

	A typical installation involves attaching
	only a suitable battery, a single pole switch for
	power on/off, and two pairs of wires connecting e-matches for the
	apogee and main ejection charges.  All Altus Metrum products are
	designed for use with single-cell batteries with 3.7 volts
	nominal.
	EasyMini may also be used with other
	batteries as long as they supply between 4 and 12 volts.

	The battery connectors are a standard 2-pin JST connector; you
	can purchase suitable batteries from the any vendor selling
	Altus Metrum products. These batteries are
	single-cell Lithium Polymer batteries that nominally provide 3.7
	volts.  Other vendors sell similar batteries for RC aircraft
	using mating connectors, however the polarity for those is
	generally reversed from the batteries used by Altus Metrum
	products. In particular, the Tenergy batteries supplied for use
	in Featherweight flight computers are not compatible with Altus
	Metrum flight computers or battery chargers.

	[WARNING]
	Check polarity and voltage before connecting any battery not
	purchased from Altus Metrum.

	[WARNING]
	Spark Fun sells batteries that have a matching connector with
	the correct polarity. However, these batteries include an
	integrated current limiting circuit. That circuit will cause
	the battery to shut down when firing the igniter circuit. Do
	not use these batteries unless you remove the current limiting
	circuit.

	By default, we use the unregulated output of the battery
	directly to fire ejection charges.  This works marvelously
	with standard low-current e-matches like the J-Tek from MJG
	Technologies, and with Quest Q2G2 igniters.  However, if you
	want or need to use a separate pyro battery, check out
	<<_using_a_separate_pyro_battery>> for instructions on how to wire
	that up. The altimeters are designed to work with an external
	pyro battery of no more than 15 volts.

	Ejection charges are wired directly to the screw terminal block
	at the aft end of the altimeter.  You'll need a very small straight
	blade screwdriver for these screws, such as you might find in a
	jeweler's screwdriver set.
	ifndef::telemini[]
	The screw terminal block is also used for the power switch leads.
	endif::telemini[]

	ifdef::telemini[]
	Except for TeleMini v1.0, the flight computers also use the
	screw terminal block for the power switch leads. On TeleMini v1.0,
	the power switch leads are soldered directly to the board and
	can be connected directly to a switch.
	endif::telemini[]

	ifdef::radio[]
	For most air-frames, the integrated antennas are more than
	adequate.   However, if you are installing in a carbon-fiber or
	metal electronics bay which is opaque to RF signals, you may need to
	use off-board external antennas instead.  In this case, you can
	replace the stock UHF antenna wire with an edge-launched SMA connector,
	and, on TeleMetrum v1, you can unplug the integrated GPS
	antenna and select an appropriate off-board GPS antenna with
	cable terminating in a U.FL connector.
	endif::radio[]
