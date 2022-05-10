== Configuration

	There are three things you need to configure identically in each unit
	that is part of a TeleLaunch system for wireless communications to
	succeed.  Those are the operating frequency, the callsign, and a key
	for the AES encryption algorithm.  Additionally, each TeleFire unit
	also needs to be configured with a unique bank number.

	Please note that these values must match exactly, or the system will
	not operate as expected.  That means the callsign needs to have the
	same choices of upper or lower case, no trailing spaces, etc.  And
	the operating frequency and AES key must match.

	=== Connecting to a Unit

		Configuring each unit of a TeleLaunch system requires use of 
		a micro USB cable, and a computer with a suitable terminal
		emulation program.  Connect a suitable cable, open your
		favoriate terminal emulation program, and power up the unit.
		By pressing <enter> you should see a command prompt.

		=== TeleLCO

			The USB connector on TeleLCO is accessible without
			opening the lid of the Pelican box.  Look for the
			blue dust cap over the connector under the handle.

		=== TeleFireEight

			The USB connector on TeleFireEight is located on the
			circuit board.  To access it, open the box and flip
			the lid up.  you should be able to spot a vertical
			micro USB connector on the board near the DIP switch.

			[WARNING]
			Please take care when closing the TeleFireEight lid
			not to pinch any wires.

	=== Operating Frequency

		The TeleLaunch system supports operation over much of the 
		"70cm" Amateur Radio band, with the filters optimized for a
		center frequency of 435 MHz.  For each system, a single
		operating frequency should be selected and programmed into
		each unit.

		The default frequency for units leaving the factory is 
		435.750 MHz.  Since Altus Metrum flight computers operate
		by default on 10 100khz channels from 434.550 to 435.450 MHz,
		we chose this frequency to be far enough away from flight
		computers to avoid any interference, but still close enough
		to the radio subsystem design center frequency for great
		performance.

		To configure the frequency, use your terminal emulator to
		issue two commands.  First, use 'c F xxxxxx' where the xxxxxx
		is replaced with the desired operating frequency in kHz.  Then
		use the 'c w' command to save this value into non-volatile
		memory.  For example, the default 435.750 MHz would be 
		configured using

			c F 435750 +
			c w

		Note that the 'f' parameter is a frequency calibration value
		that you really, really, really don't want to change.  So, 
		please be careful to make sure you use capital 'F', not lower
		case 'f' when changing the operating frequency!

	=== Callsign

		In the US, you need an
	        link:http://www.altusmetrum.org/Radio/[amateur radio license]
	        or other authorization to legally operate the radio
	        transmitters that are part of TeleLaunch.

		The default callsign shipped from the factory is "N0CALL",
		which is not a valid callsign and meant to humorously point
		out that the callsign hasn't been configured yet.

		Individual owners of a TeleLaunch system should use their
		own callsign.  

		Club owners of a TeleLaunch system should pick the callsign
		of one club member who is willing to be designated as the 
		control operator of the system.  Under FCC Part 97 rules,
		being the control operator does not mean you have to be LCO
		all the time, it just means taking responsibility for ensuring
		the system is being operated in compliance with the rules.

		To configure the callsign, use your terminal emulator to 
		issue two commands.  First, use 'c c callsign' to set the
		callsign, then use 'c w' to write to non-volatile memory.
		For example, to set the default N0CALL, the commands would
		be

			c c N0CALL +
			c w

	=== AES Key

		For safety, TeleLaunch uses cryptographic checksums to help 
		prevent interference, intentional or un-intentional.  This
		means each system must have a shared AES encryption key
		identically configured into each unit.

		The key size required is 128 bits, which must be expressed
		as a 32-digit hexadecimal number.

		To configure the AES key, use 'c a key' followed by 'c w'
		to write the key to non-volatile memory.  For example, to
		configure your system with a key that is the answer to life,
		the universe, and everything, the commands would be

			c a 00000000000000000000000000000042 +
			c w

	=== Bank Number

		Each TeleFire unit needs to be configured with a bank number,
		and bank numbers should be unique within a given system.  For
		most systems with 8 or fewer banks, just use the DIP switch
		on the circuit board inside the TeleFire unit to select the
		desired bank.  Only one switch should be turned on.  Switch
		one means bank one, etc.

		To allow systems to have more than 8 banks, if all the DIP
		switches are "off", the unit will use the bank configured in 
		non-volatile memory.

		To configure the bank number, use 'c B bank' followed by
		'c w' to write to non-volatile memory.  For example, to
		set the bank to 42, the commands would be:

			c B 42 +
			c w

