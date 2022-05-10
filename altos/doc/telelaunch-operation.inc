== Operation

	Operating a TeleLaunch system is pretty easy, and we hope fairly
	intuitive for anyone who has ever launched rockets before.  Basic 
	instructions are printed on a decal inside the lid of each TeleLCO
	unit, which early customers (before this manual was even written)
	reported were sufficient to successfully use the system.

	The remainder of this section assumes the system has already been
	properly configured with callsign, AES key, and bank numbers for each
	TeleFire box.

	=== Charging Batteries

		TeleLCO has an internal Lithium Polymer battery that is 
		charged over USB.  A weather-tight micro USB connector is
		installed with a blue cap under the handle.  This position
		was chosen so that TeleLCO can be charged while the lid is
		closed and standing handle-up.  Attach a micro USB cable
		to a computer or other source of USB power.  There are two
		charge indicators on the panel that will illuminate during
		charging, red for charge in progress and green for charge
		complete.
	
		TeleFire has an internal sealed lead acid 12V battery, and
		is provided with an external AC charger.  Connect this charger
		to the PowerPole connector on the back face of the box to
		charge the battery.  Other sources of charging current for
		a nominal 12V sealed battery may be used.  The charge 
		connector can also be used to attach an external battery if
		needed.
	
	=== Setting Up

		Each unit in the TeleLaunch system needs a suitable antenna
		attached.  A typical configuration would involve the provided
		omni-directional antenna or optional directional antenna aimed 
		at the field mounted on a pole attached to TeleLCO at the 
		launch control station, and the provided rubber whip antennas
		on each TeleFire box.  

		If deploying the system in rough terrain, on a field with 
		dense vegetation, or at great distances (some away cells), you
		may need a better antenna or at least to mount the antenna 
		higher off the ground.  All units in the TeleLaunch system
		have tri-color RF signal strength indicators.  A system should
		show green on all units during normal operation.  An occasional
		dip to amber is ok, but frequent amber or any red indicates a
		need to improve the antenna situation.

		Antenna made for use near 435 Mhz in the ham radio "70cm band" 
		should work, and there are many online sources of information
		on making inexpensive, highly effective antennas at home.

		The TeleLCO unit should be placed on a table at the Launch
		Control position.  It can be helpful for the LCO (the human(s)
		too, not just the electronics!) to be provided with some 
		shade, but the use of reflective LCDs for the digit 
		displays and ultri-bright LEDs make TeleLCO operation 
		reasonable even in direct sunlight.

		Each TeleFire box should be placed in proximity to the launch
		rails it will service.  Whip clips made from AC power cords
		(not provided) should be attached to TeleFire and routed to
		each launch rail.  The pyro output on each channel is 
		connected to the "line" and "neutral" pins on the outlet,
		while the ground pin is attached to chassis ground.

	=== Turning System On

		Turn on all TeleFire pad boxes in the system.  At power-on,
		all LEDs will turn on briefly so you can verify they're all 
		working.  The radio signal strength indicators will show red
		until TeleLCO is turned on.

		Turn on TeleLCO.  All LCD segments and LEDs will turn on
		briefly so you can verify they're all working.  An exception
		are the two battery charger LEDs, which only illuminate when
		the battery is charging.  The LCD displays will then briefly
		display the internal battery voltage, which should be at least
		3.9V for normal operation.  TeleLCO then scans to locate all 
		TeleFire boxes that are in range and configured for the same 
		frequency, callsign, and AES key.  During this process, the 
		Bank LCD diplays will appear to be counting up from 1 to 99.
		When the search is complete, the Pad display will show the
		number of TeleFire devices found.  TeleLCO beeps when ready.

	=== Launch One Rocket

		Switch the TeleFire to 'Safe' and confirm it is silent.

		Put a rocket on a launch rail / rod, and connect whip clips
		from a pad output on a TeleFire unit to the motor igniter.
		Verify igniter continuity using the LED associated with that
		output.  Green is good.

		Switch the TeleFire unit from 'Safe' to 'Arm', at which point
		it will start "chirping" to indicate that it is "armed and 
		dangerous".  Move to the TeleLCO at a safe distance away.

		Ensure the TeleLCO blue Drag Race switch is on Normal and 
		the blue Drag Race LED is extinguished.

		Select the desired TeleFire unit by pushing the TeleLCO 
		selector knob until the "Bank" LED is lit, then rotating the 
		knob until the 
		desired box is selected.  Then push the knob until the Pad 
		LED is lit and rotate the knob until the desired pad is 
		selected.  Verify the Remote Armed LED is lit, and that the
		selected pad's Igniter Continuity LED is lit.

		Perform range safety checks.

		Move the TeleLCO SAFE/ARM switch to ARM.  This will cause
		the selected TeleFire unit's strobe to start flashing and
		siren to become more insistent. 

		Perform count-down.

		Depress the LAUNCH button and hold until the rocket launches.

		Move the TeleLCO SAFE/ARM switch back to SAFE.

	=== Launch A Multi-Rocket "Drag Race"

		To enable drag race mode, move the TeleLCO blue switch to
		Drag Race.  The blue Drag Race LED should illuminate.

		Select pads to include in the drag race by using the same
		process as for a single rocket to select each Bank and Pad,
		then press Add/Remove to toggle the currently selected pad
		in the race.  The continuity LEDs blink ON/off for selected,
		and on/OFF for deselected (long ON means in the race).

		A drag race can be configured to include pads on multiple
		TeleFire boxes, but the blinking continuity indicators to
		show which pads are included can only indicate the currently
		selected bank.

		Verify remotes are ready and armed by using the Remote Armed
		and Igniter Continuity LEDs.

		Perform range safety checks.

		Move the TeleLCO SAFE/ARM switch to ARM.  This will cause
		the selected TeleFire units strobes to start flashing and
		sirens to become more insistent. 

		Perform count-down.

		Depress the LAUNCH button and hold until rockets launch.

		Move the TeleLCO SAFE/ARM switch back to SAFE.

		Move Drag Race switch back to Normal.

		Note that if there is a mis-fire in a drag race and you want
		to fix igniters and try again, the current drag configuration
		is preserved as long as you stay in Drag Race mode.  So you
		can SAFE the system, fix igniters, and try again before leaving
		Drag Race mode if desired.
