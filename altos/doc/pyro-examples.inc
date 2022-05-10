[appendix]
== Example Pyro Channel Configurations

	Programming configurable pyro channels on Altus Metrum products that 
	include them isn't difficult, but in an attempt to aid understanding
	of the configuration interface and help "keep simple things simple",
	we offer the following examples of the simplest configurations for
	common situations, along with some hints on avoiding unexpected
	results.

	The rich set of conditions provided can be used to configure almost
	any pyro event you can imagine, for a wide variety of objectives.  
	But don't be fooled!  Typical events need only one or a few simple
	conditions to be configured for success.  A key thing to remember is 
	that *all* configured conditions must be true to allow a pyro channel 
	to fire.  Trying to include too many conditions often results in 
	conflicting rules that never allow a channel to fire.  The most 
	important advice we can offer is, therefore, to try and find the 
	simplest set of conditions that will do what you need for a given 
	project.

	=== Two-Stage Flights

		Successful completion of a two-stage flight often involves
		programming of two events.  The first is firing a separation
		charge, the second is igniting the sustainer's (primary)
		motor.

		Separation charges are best fired as soon as possible after
		the previous stage has completed providing acceleration, to
		minimize drag of the sustainer's coast phase before ignition.
		Recovery, whether the remainder of the flight is nominal or
		not, usually works best when the states are separated.  So,
		the "best" way to configure a pyro channel for a separation
		charge is to just set "after motor number".  For a 2-stage
		project, set this to "1".  This will cause the pyro channel
		to fire as soon as the firmware's flight state machine
		determines the first motor has burned out.

		Safe ignition of a sustainer (primary) motor requires that
		it happen after the previous stage burns out, while the 
		airframe remains mostly vertical, and typically after the
		sustainer has coasted away from the booster a bit.  A good
		starting point is thus "after motor number" set the same as
		the separation charge, which is "1" for a 2-stage rocket.
		Then "angle from vertical less than" set to some
		reasonably vertical amount, perhaps 20 degrees.  Then "delay
		after other conditions" set for the desired duration of coast.
		Use simulations to figure out what a reasonable value here is,
		but for typical high power rocketry sport flights that aren't 
		trying to set records, something like 2 seconds is usually a 
		good place to start.
	
	=== Triggered Clusters and Air Starts

		When an airframe has a cluster of motors, one of which is 
		"primary" and centered, surrounding by a ring of "secondary"
		motors, you may want to use the launch control system to 			fire the primary motor and use onboard electronics to light
		the rest of the cluster as soon as launch is detected.  This
		is particularly true if the primary motor is significantly
		different in geometry and may take longer to come up to 
		pressure than the secondary motors.  In this case, a simple
		configuration to light secondary motors is is "time since
		boost greater than" enabled and set to "0".  There's
		really no point in setting an angle limit since no time has
		transpired for the airframe to change orientation.

		Air starts can use the same simple configuration, but with 
		the time set to a non-zero value.  However, if air starts
		are going to light after the airframe leaves the launch rail
		or tower, add an "angle from vertical less than"
		condition just you would for a 2-stage sustainer to stay safe.

	=== Redundant Apogee

		When flying a board like TeleMega or EasyMega, it's easy to
		configure a programmable channel to fire a redundant apogee
		charge.  This is of course not *fully* redundant, since it's
		always possible that the board itself or its battery could
		the the failure source, but far more often, pyro events fail
		due to broken wires, bad connectors, or bad e-matches... so
		firing two charges from one board can add useful redundancy.

		The simplest configuration for redundant apogee is "flight
		state after" set to "drogue", and then "delay after other 
		conditions" set to a second or two.

	=== Redundant Main

		Similarly to apogee, configuring a redundant main charge can
		provide useful redundancy.  What we want is to configure an
		altitude for deployment lower than the primary main deploy
		altitude, and then ensure we only trigger on that condition
		while descending.

		The simplest configuration for redundant main is "flight
		state after" set to "drogue", which will ensure we're in to
		the descent phase, then "height less than" set to a number
		lower than you've chosen for the primary main channel 
		deployment height.

	=== Apogee Above Baro Sensor Limit

		A question we've seen increasingly often is "How does the 
		Telemega/Easymega detect apogee for flights above 100,000
		feet?"  Flights above that height are a bit outside
		our original design envelope, but can be made to work...
		This is *not* a simple flight, and the configuration for it
		is also not simple, but we think including this information
		is important for anyone contemplating such a project with our
		electronics!

		Our flight computers use a Kalman sensor-fusing filter to 
		estimate the flight state, which consists of three values:

 			1. Height above ground
 			2. Vertical speed
 			3. Vertical acceleration

		Apogee is assumed to be where vertical speed crosses zero.

		Below 30km altitude (about 100k'), we use both the barometer 
		and the accelerometer to update the flight state, along with 
		a basic Newtonian model of motion. That works well, pegging 
		apogee within a few sensor samples essentially every time.

		Above 30km, the barometric sensor doesn't provide useful data, 
		so we can't use it to update the flight state. Instead, the 
		Kalman filter falls back to a single sensor mode, using only 
		the accelerometer.

		At all altitudes, we de-sense the barometric data when we 
		estimate the speed is near or above mach as the sensor is 
		often subjected to significant transients, which would 
		otherwise push the flight state estimates too fast and could 
		trigger a false apogee event.

		That means the filter is no longer getting the benefit of two 
		sensors, and relies on just the accelerometer. The trouble 
		with accelerometers is they're measuring the derivative of 
		speed, so you have to integrate their values to compute speed. 
		Any offset error in acceleration measurement gets constantly 
		added to that speed.

		In addition, we assume the axial acceleration is actually 
		vertical acceleration; our tilt measurements have enough 
		integration error during coast that we can't usefully use 
		that to get vertical acceleration. Because we don't live in 
		an inertial frame, that means we're mis-computing the total 
		acceleration acting on the airframe as we have to add gravity 
		into the mix, and simply adding that to the axial acceleration 
		value doesn't generate the right value.

		The effect of this is to under-estimate apogee when you base 
		the computation purely on acceleration as the rocket flies a 
		parabolic path.

		For flights *near* 100k', all of this works pretty well - 
		you've got the flight state estimates adjusted using the 
		barometric sensor up to 30km, then you're flying on inertial 
		data to apogee.

		For flights well above 100k', it's not great; you're usually 
		going fast enough through 100k' that the baro sensor is still 
		de-sensed through the end of its useful range, so the flight 
		state estimates are not as close. After that, as you're flying 
		purely on accelerometer data, there's no way to re-correct the 
		state, so the apogee estimates can be off by quite a bit.

		In the worst cases we have seen, the baro sensor data was 
		wildly incorrect above mach due to poor static port design, 
		leaving the state estimate of speed across the 30km boundary 
		way off and causing the apogee detection to happen far from 
		the correct time.

		The good news is that correctly determining apogee is not 
		really all that important at high altitudes; there's so little 
		density that a drogue will have almost no drag anyways.  Data
		from customer flights shows a very parabolic path down to 
		about 50-60k feet, even with a recovery system deployed.

		So, what we recommend is to set up two apogee plans:

 			1. Use the built-in apogee detection, but add a 
			   significant delay (as much as 30 seconds). This 
			   will probably fire near enough to apogee to not 
			   have a significant impact on the maximum height 
			   achieved.

 			2. Add a back-up apogee which fires after apogee 
			   *when the height is below about 20-25km*. This 
			   way, if the flight isn't nominal, and the sustainer 
			   ends up reaching apogee in dense air, you aren't 
			   hoping the chutes come out before it gets going 
			   too fast. And, you get a second pyro channel firing 
			   at that altitude even if it reached a higher 
			   altitude before.

		You can wire these two pyro channels to the same pyro device; 
		you just need to make sure they're wired + to + and - to - 
		(the manual shows which screw terminals are which).

		The bottom line is that flights to altitudes modestly above
		the range of the baro sensor with Altus Metrum products can
		be accomplished safely, but flying "way high" (like 300k') 
		demands a deployment mechanism which doesn't solely rely on 
		altimeters (like ours) which are designed for modest altitude 
		rocketry.  Flights to those altitudes also probably need 
		active stabilization to make sure they follow the prescribed 
		trajectory and stay inside their waiver.
