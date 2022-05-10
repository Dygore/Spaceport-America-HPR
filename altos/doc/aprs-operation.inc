	=== APRS

		{aprsdevices} can send APRS if desired, and the
		interval between APRS packets can be configured. As
		each APRS packet takes a full second to transmit, we
		recommend an interval of at least 5 seconds to avoid
		consuming too much battery power or radio channel
		bandwidth. You can configure the time within each
		minute that APRS transmits by changing the APRS offset
		value. When the GPS signal is locked and knows the
		current time, the APRS offset selects the time with
		each minute for the first APRS transmission;
		subsequent transmissions occur each APRS interval
		seconds thereafter.  You can configure the APRS
		interval and APRS offset using {application}; that
		process is described in <<{configure_section}>>.

		AltOS supports both compressed and uncompressed APRS
		position report data formats. The compressed format
		provides for higher position precision and shorter
		packets than the uncompressed APRS format. We've found
		some older APRS receivers that do not handle the
		compressed format. The Kenwood TH-72A requires the use
		of uncompressed format to display altitude information
		correctly. The Yaesu FT1D requires the use of
		compressed format to display altitude information.

		APRS packets include an SSID (Secondary Station Identifier)
		field that allows one operator to have multiple
		transmitters. AltOS allows you to set this to a single digit
		from 0 to 9, allowing you to fly multiple transmitters at the
		same time while keeping the identify of each one separate in
		the receiver. By default, the SSID is set to the last digit of
		the device serial number.

		The APRS packet format includes a comment field that
		can have arbitrary text in it. AltOS uses this to send
		status information as shown in the following table.

		.Altus Metrum APRS Comments
		[options="header",cols="1,1,3"]
		|====
		|Field	      |Example	      |Description

		|1
		|L
		|GPS Status U for unlocked, L for locked

		|2
		|6
		|Number of Satellites in View

		|3
		|B4.0
		|Altimeter Battery Voltage

	ifdef::altusmetrum[]

		|4
		|A3.7
		|Apogee Igniter Voltage

		|5
		|M3.7
		|Main Igniter Voltage

		|6
		|1286
		|Device Serial Number

	endif::altusmetrum[]
	ifdef::telegps[]

		|4
		|1286
		|Device Serial Number

	endif::telegps[]

		|====

		ifdef::altusmetrum[]
		Here's an example of an APRS comment showing GPS lock with 6
		satellites in view, a primary battery at 4.0V, and
		apogee and main igniters both at 3.7V from device 1286.

		....
		L6 B4.0 A3.7 M3.7 1286
		....
		endif::altusmetrum[]

		ifdef::telegps[]
		Here's an example of an APRS comment showing GPS lock with 6
		satellites in view and a primary battery at 4.0V from device 1876.

		....
		L6 B4.0 1876
		....
		endif::telegps[]

		Make sure your primary battery is above 3.8V
	ifdef::altusmetrum[]
		any connected igniters are above 3.5V
	endif::altusmetrum[]
		and GPS is locked with at least 5 or 6 satellites in
		view before flying. If GPS is switching between L and
		U regularly, then it doesn't have a good lock and you
		should wait until it becomes stable.

		If the GPS receiver loses lock, the APRS data
		transmitted will contain the last position for which
		GPS lock was available. You can tell that this has
		happened by noticing that the GPS status character
		switches from 'L' to 'U'. Before GPS has locked, APRS
		will transmit zero for latitude, longitude and
		altitude.
