== Using TeleGPS Hardware

	=== Hooking Up Lithium Polymer Batteries

		TeleGPS has a two pin JST PH series connector to connect up
		a single-cell Lithium Polymer cell (3.7V nominal). You can
		purchase matching batteries from the Altus Metrum store, or
		other vendors, or you can make your own. Pin 1 of the
		connector is positive, pin 2 is negative. Spark Fun sells a
		cable with the connector attached, which they call a
		link:https://www.sparkfun.com/products/9914[JST Jumper 2 Wire Assembly]


		[WARNING]
		Many RC vendors also sell lithium polymer batteries with
		this same connector. All that we have found use the opposite
		polarity, and if you use them that way, you will damage or
		destroy TeleGPS.

	=== On-board Data Recording

		TeleGPS logs GPS data at a user-configurable
		rate. Data are logged to a 2MB on-board flash memory
		part, which can be partitioned into several
		equal-sized blocks, one for each flight. 64kB of this
		storage are reserved to hold configuration data,
		leaving 1984kB for flight data.

		The on-board flash is partitioned into separate flight
		logs, each of a fixed maximum size. Increase the
		maximum size of each log and you reduce the number of
		flights that can be stored. Decrease the size and you
		can store more flights.

		To compute the amount of space needed for a single
		log, you can divide the expected time (in seconds) by
		the sample period (by default, 1 second per sample)
		and then multiply the result by 32 bytes per
		sample. For instance, a sample period of 1 second and
		a flight lasting one hour will take 32 * 3600 = 115200
		bytes. TeleGPS does try to reduce log space used by
		not recording position information when it isn't
		moving, so actual space consumed may be less than
		this.

		The default size allows for four flights of 496kB
		each, which provides over four hours of logging at 1
		sample per second.

		TeleGPS will not overwrite existing flight data, so be
		sure to download flight data and erase it from the
		onboard flash before it fills up. TeleGPS will still
		report telemetry even if memory is full, so the only
		thing you will lose is the on-board data log.

	=== LEDs

		TeleGPS v1.0 has a dual greed/red LED along the edge
		towards the USB connector end of the board. This
		indicates battery charging status while connected to
		power over USB. When the red LED is lit, the battery
		is charging. Once the battery is fully charged, the
		red LED goes out and the green LED turns on. If both
		LEDs are lit, something is probably wrong with the battery.

		TeleGPS v2.0 has the same green/red battery charging
		status LED and adds another green LED along the same
		edge, towards end of the board with the 70cm
		antenna. This green LED indicates GPS lock status — it
		blinks once every three seconds when the GPS receiver
		is locked and tracking position.

	=== Installation

		The battery connectors are a standard 2-pin JST
		connector and match batteries sold by Spark Fun. These
		batteries are single-cell Lithium Polymer batteries
		that nominally provide 3.7 volts.  Other vendors sell
		similar batteries for RC aircraft using mating
		connectors, however the polarity for those is
		generally reversed from the batteries used by Altus
		Metrum products. In particular, the Tenergy batteries
		supplied for use in Featherweight flight computers are
		not compatible with Altus Metrum flight computers or
		battery chargers.

		[WARNING]
		Check polarity and voltage before connecting any
		battery not purchased from Altus Metrum or Spark
		Fun.

		TeleGPS uses an integrate GPS patch antenna and won't
		receive GPS signals if installed inside a metal or
		carbon fiber compartment. Test GPS reception and
		telemetry transmission with the system installed and
		all other electronics powered up to verify signal
		reception and make sure there isn't any interference
		from other systems.
