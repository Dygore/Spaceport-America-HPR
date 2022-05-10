== Using Altus Metrum Products

	ifdef::radio[]

	=== Being Legal

	        In the US, you need an
	        link:http://www.altusmetrum.org/Radio/[amateur radio license]
	        or other authorization to legally operate the radio
	        transmitters that are part of our products.

	endif::radio[]


	=== In the Rocket

		In the rocket itself, you just need a flight computer
		and a single-cell, 3.7 volt nominal Li-Po rechargeable
		battery.
		ifdef::telemetrum,telemega,easymega,easytimer[]
		An 850mAh battery weighs less than a 9V
		alkaline battery, and will run a TeleMetrum, TeleMega,
		EasyMega or EasyTimer for hours.
		endif::telemetrum,telemega,easymega,easytimer[]
		A 110mAh battery weighs less
		than a triple A battery and is a good choice for use
		with
		ifdef::telemini[TeleMini or]
		EasyMini.

		ifdef::radio[]
		By default, we ship TeleMini, TeleMetrum and TeleMega
		flight computers with a simple wire antenna.  If your
		electronics bay or the air-frame it resides within is
		made of carbon fiber, which is opaque to RF signals,
		you may prefer to install an SMA connector so that you
		can run a coaxial cable to an antenna mounted
		elsewhere in the rocket.  However, note that the GPS
		antenna is fixed on all current products, so you
		really want to install the flight computer in a bay
		made of RF-transparent materials if at all possible.
		endif::radio[]

	=== On the Ground

		ifdef::radio[]
		To receive the data stream from the rocket, you need
		an antenna and short feed-line connected to one of our
		link:http://www.altusmetrum.org/TeleDongle/[TeleDongle]
		units.  If possible, use an SMA to BNC adapter instead
		of feedline between the antenna feedpoint and
		TeleDongle, as this will give you the best
		performance.  The TeleDongle in turn plugs directly
		into the USB port on a notebook computer.  Because
		TeleDongle looks like a simple serial port, your
		computer does not require special device
		drivers... just plug it in.
		endif::radio[]

		The GUI tool, AltosUI, is written in Java and runs
		across Linux, Mac OS and Windows. There's also a suite
		of C tools for Linux which can perform most of the
		same tasks.

		ifdef::radio[]
		Alternatively, a TeleBT attached with an SMA to BNC
		adapter at the feed point of a hand-held yagi used in
		conjunction with an Android device running AltosDroid
		makes an outstanding ground station.
		endif::radio[]

		After the flight,
		ifdef::radio[]
		you can use the radio link to
		extract the more detailed data logged in either
		TeleMetrum or TeleMini devices, or
		endif::radio[]
		you can use a
		USB cable to plug into the flight computer board directly.
		A USB cable is also how you
		charge the Li-Po battery, so you'll want one of those
		anyway. The same cable used by lots of digital
		cameras and other modern electronic stuff will work
		fine.

		ifdef::gps[]
		If your rocket lands out of sight, you may enjoy
		having a hand-held GPS receiver, so that you can put
		in a way-point for the last reported rocket position
		before touch-down.  This makes looking for your rocket
		a lot like Geo-Caching... just go to the way-point and
		look around starting from there.  AltosDroid on an
		Android device with GPS receiver works great for this,
		too!
		endif::gps[]

		ifdef::radio[]
		You may also enjoy having a ham radio “HT” that covers
		the 70cm band... you can use that with your antenna to
		direction-find the rocket on the ground the same way
		you can use a Walston or Beeline tracker.  This can be
		handy if the rocket is hiding in sage brush or a tree,
		or if the last GPS position doesn't get you close
		enough because the rocket dropped into a canyon, or
		the wind is blowing it across a dry lake bed, or
		something like that...  Keith currently uses a Yaesu
		FT1D, Bdale has a Yaesu VX-7R, which is a nicer radio
		in most ways but doesn't support APRS.

		So, to recap, on the ground the hardware you'll need includes:

		. an antenna and feed-line or adapter
		. a TeleDongle
		. a notebook computer
		. optionally, a hand-held GPS receiver
		. optionally, an HT or receiver covering 435 MHz

		The best hand-held commercial directional antennas we've found for radio
		direction finding rockets are from
		link:http://www.arrowantennas.com/[Arrow Antennas].

		The 440-3 and 440-5 are both good choices for finding
		a TeleMetrum- or TeleMini- equipped rocket when used
		with a suitable 70cm HT.  TeleDongle and an SMA to BNC
		adapter fit perfectly between the driven element and
		reflector of Arrow antennas.
		endif::radio[]

	=== Data Analysis

		Our software makes it easy to log the data from each
		flight, both the telemetry received during the flight
		itself, and the more complete data log recorded in the
		flash memory on the altimeter board.  Once this data
		is on your computer, our post-flight tools make it
		easy to quickly get to the numbers everyone wants,
		like apogee altitude, max acceleration, and max
		velocity.  You can also generate and view a standard
		set of plots showing the altitude, acceleration, and
		velocity of the rocket during flight.  And you can
		even export a flight log in a format usable with Google
		Maps and Google Earth for visualizing the flight path
		in two or three dimensions!

		Our ultimate goal is to emit a set of files for each
		flight that can be published as a web page per flight,
		or just viewed on your local disk with a web browser.

	=== Future Plans

		ifdef::telemetrum,telemega,easymega[]
		We have designed and prototyped several “companion
		boards” that can attach to the companion connector on
		TeleMetrum, TeleMega and EasyMega flight computers to
		collect more data, provide more pyro channels, and so
		forth.  We do not yet know if or when any of these
		boards will be produced in enough quantity to sell.
		If you have specific interests for data collection or
		control of events in your rockets beyond the
		capabilities of our existing productions, please let
		us know!
		endif::telemetrum,telemega,easymega[]

		Because all of our work is open, both the hardware
		designs and the software, if you have some great idea
		for an addition to the current Altus Metrum family,
		feel free to dive in and help!  Or let us know what
		you'd like to see that we aren't already working on,
		and maybe we'll get excited about it too...

		Watch our link:http://altusmetrum.org/[web site] for
		more news and information as our family of products
		evolves!
