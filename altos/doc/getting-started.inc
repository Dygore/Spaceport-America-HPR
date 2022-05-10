== Getting Started

	The first thing to do after you open the box is to hook up a
	battery and charge it if necessary.

	=== Batteries

		ifdef::telemetrum,telemega,easymega[]
		For TeleMetrum, TeleMega and EasyMega, the battery can be charged by plugging it into the
		corresponding socket of the device and then using the USB
		cable to plug the flight computer into your computer's USB socket. The
		on-board circuitry will charge the battery whenever it is plugged
		in, because the on-off switch does NOT control the
		charging circuitry.
		endif::telemetrum,telemega,easymega[]
		The Lithium Polymer
		ifdef::easytimer[EasyTimer, ]
		ifdef::telemini[TeleMini and]
		EasyMini battery can be charged by disconnecting it
		from the board and plugging it into a standalone
		battery charger such as link:http://altusmetrum.org/LipoCharger[LipoCharger], and
		connecting that via a USB cable to a laptop or other
		USB power source.

		You can also choose to use another battery with
		ifdef::easytimer[EasyTimer and]
		EasyMini, anything supplying between 4 and 12 volts should
		work fine (like a standard 9V battery), but if you are planning
		to fire pyro charges, ground testing is required to verify that
		the battery supplies enough current to fire your chosen e-matches.

		ifdef::telemetrum,telemega,easymega[]
		[NOTE]
		====
		On TeleMetrum v1 boards, when the GPS chip is initially
		searching for satellites, TeleMetrum will consume more current
		than it pulls from the USB port, so the battery must be
		attached in order to get satellite lock.  Once GPS is locked,
		the current consumption goes back down enough to enable charging
		while running. So it's a good idea to fully charge the battery
		as your first item of business so there is no issue getting and
		maintaining satellite lock.  The yellow charge indicator led
		will go out when the battery is nearly full and the charger goes
		to trickle charge. It can take several hours to fully recharge a
		deeply discharged battery.

		TeleMetrum v2 or newer, TeleMega and EasyMega use a
		higher power battery charger, allowing them to charge
		the battery while running the board at maximum
		power. When the battery is charging, or when the board
		is consuming a lot of power, the red LED will be
		lit. When the battery is fully charged, the green LED
		will be lit. When the battery is damaged or missing,
		both LEDs will be lit, which appears yellow.
		====
		endif::telemetrum,telemega,easymega[]

	ifdef::radio[]
	=== Ground Station Hardware

		There are two ground stations available, the TeleDongle USB to
		RF interface and the TeleBT Bluetooth/USB to RF interface.  If
		you plug either of these in to your Mac or Linux computer it should
		“just work”, showing up as a serial port device.  Windows systems need
		driver information that is part of the AltOS download to know that the
		existing USB modem driver will work.  We therefore recommend installing
		our software before plugging in TeleDongle if you are using a Windows
		computer.  If you are using an older version of Linux and are having
		problems, try moving to a fresher kernel (2.6.33 or
		newer).
	endif::radio[]

	=== Linux/Mac/Windows Ground Station Software

		Next you should obtain and install the AltOS software.
		The AltOS distribution includes the AltosUI ground
		station program, current firmware images for all of
		the hardware, and a number of standalone utilities
		that are rarely needed.  Pre-built binary packages are
		available for Linux, Microsoft Windows, Mac OSX. Full
		source code and build instructions are also
		available. The latest version may always be downloaded
		from http://altusmetrum.org/AltOS

	ifdef::radio[]
	=== Android Ground Station Software

		TeleBT can also connect to an Android device over
		BlueTooth or USB. The
		link:https://play.google.com/store/apps/details?id=org.altusmetrum.AltosDroid[AltosDroid
		Android application] is available from the  
		link:https://play.google.com[Google Play system].

		You don't need a data plan to use AltosDroid, but
		without network access, you'll want to download
		offline map data before wandering away from the
		network.
	endif::radio[]
