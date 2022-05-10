ifdef::radio[]
==== Voice Settings

	{application} provides voice announcements during
	flight so that you can keep your eyes on the
	sky and still get information about the
	current flight status. However, sometimes you
	don't want to hear them.

	Enable::
	Turns all voice announcements on and off

	Test Voice::
	Plays a short message allowing you to verify
	that the audio system is working and the volume settings
	are reasonable
endif::radio[]

==== Log Directory

	{application} logs all telemetry data and saves all
	flash data to this directory. This
	directory is also used as the staring point
	when selecting data files for display or
	export.

	Click on the directory name to bring up a
	directory choosing dialog, select a new
	directory and click 'Select Directory' to
	change where {application} reads and writes data
	files.

ifdef::radio[]
==== Callsign

	This value is transmitted in each command
	packet sent from TeleDongle and received from
	an altimeter.  It is not used in telemetry
	mode, as the callsign configured in the
	altimeter board is included in all telemetry
	packets.  Configure this with the {application}
	operators call sign as needed to comply with
	your local radio regulations.

	Note that to successfully command a flight
	computer over the radio (to configure the
	altimeter, monitor idle, or fire pyro
	charges), the callsign configured here must
	exactly match the callsign configured in the
	flight computer.  This matching is case
	sensitive.
endif::radio[]

==== Imperial Units

	This switches between metric units (meters)
	and imperial units (feet and miles). This
	affects the display of values use during
	flight monitoring, configuration, data
	graphing and all of the voice
	announcements. It does not change the units
	used when exporting to CSV files, those are
	always produced in metric units.

==== Serial Debug

	This causes all communication with a connected
	device to be dumped to the console from which
	{application} was started. If you've started it from
	an icon or menu entry, the output will simply
	be discarded. This mode can be useful to debug
	various serial communication issues.

==== Font size

	Selects the set of fonts used in the flight
	monitor window. Choose between the small,
	medium and large sets.

==== Look & feel

	Switches between the available Java user
	interface appearances. The default selection
	is supposed to match the native window system
	appearance for the target platform.
	
==== Menu position

	Selects the initial position for the main
	{application} window that includes all of the
	command buttons.

ifdef::gps[]
==== Map Cache Size

	Sets the number of map 'tiles' kept in memory
	while the application is running. More tiles
	consume more memory, but will make panning
	around the map faster.
endif::gps[]

ifdef::radio[]
==== Manage Frequencies

	This brings up a dialog where you can
	configure the set of frequencies shown in the
	various frequency menus. You can add as many
	as you like, or even reconfigure the default
	set. Changing this list does not affect the
	frequency settings of any devices, it only
	changes the set of frequencies shown in the
	menus.
endif::radio[]
