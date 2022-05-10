
Vertical Acceleration:: Select a value, and then choose whether
acceleration away from the ground should be above or below that
value. Acceleration is positive upwards, so accelerating towards the
ground would produce negative numbers. Acceleration during descent is
noisy and inaccurate, so be careful when using it during these phases
of the flight.

Ascent rate:: Select a value, and then choose whether ascent rate
should be above or below that value. Ascent rate is positive upwards,
so moving towards the ground would produce negative numbers. Ascent
rate during descent is a bit noisy and so be careful when using it
during these phases of the flight.

Height above pad:: Select a value, and then choose whether the height
above the launch pad should be above or below that value. Note that
because EasyTimer has only a low-range accelerometer and no barometer,
this value will not be very reliable on that device.

Orientation:: TeleMega, EasyMega and EasyTimer contain a 3-axis
gyroscope and accelerometer which is used to compute the orientation
of the rocket. A record of orientations over the last 0.64 seconds is
kept and the largest value within this period is compared with the
specified value. Note that the tilt angle is not the change in angle
from the launch pad, but rather absolute relative to gravity—the
3-axis accelerometer is used to compute the angle of the rocket on the
launch pad and initialize the system.

  [NOTE]
  ====
  Because this value is computed by integrating
  rate gyros, it gets progressively less
  accurate as the flight goes on. It should have
  an accumulated error of less than 0.2°/second
  (after 10 seconds of flight, the error should
  be less than 2°).

  The usual use of the orientation configuration
  is to ensure that the rocket is traveling
  mostly upwards when deciding whether to ignite
  air starts or additional stages. For that,
  choose a reasonable maximum angle (like 20°)
  and set the motor igniter to require an angle
  of less than that value.
  ====

Flight Time:: Time since launch. Select a value and choose whether to
activate the pyro channel before or after that amount of time.

[WARNING]
Firmware versions older than 1.8.6 have a bug which resets the time
since launch to zero each time a motor starts burning. Update firmware
to get the correct behavior.

[WARNING]
Firmware versions older than 1.9.8 cannot use times longer
than 327.67 seconds. Update firmware if you need a longer time.

Ascending:: A deprecated configuration value which was the same as
setting Ascent rate > 0. Existing configurations using this will be
cleared and must be reconfigured by the user.

Descending:: A deprecated configuration value which was the same as
setting Ascent rate < 0. Existing configurations using this will be
cleared and must be reconfigured by the user.

After Motor:: The flight software counts each time the rocket starts
accelerating and then decelerating (presumably due to a motor or
motors burning). Use this value for multi-staged or multi-airstart
launches. As of version 1.8.6 firmware, this checks to make sure at
least this many motors have burned. Before version 1.8.6, this checked
to make sure that exactly this many motors had burned.

Delay:: Once the other parameters all become true, a timer is
started for the specified amount of time. While the timer is running,
the other parameters are checked repeatedly and if any of them become
false, then the pyro channel is disabled and will not fire. If the
timer expires and all of the other parameters have remained true for
the entire time, then the pyro channel is fired.

[WARNING]
Firmware versions older than 1.9.8 cannot use delays longer
than 327.67 seconds. Update firmware if you need a longer delay.

Flight State:: The flight software tracks the flight
through a sequence of states:

 * Boost. The motor has lit and the rocket is
   accelerating upwards. Ascent rate will be greater than zero.
   Vertical acceleration will be greater than zero.

 * Fast. The motor has burned out and the
   rocket is decelerating, but it is going
   faster than 200m/s. Ascent rate will be greater than zero. Vertical
   acceleration will be less than zero.

 * Coast. The rocket is still moving upwards
   and decelerating, but the Ascent rate is less
   than 200m/s. Ascent rate will greater than zero. Vertical
   acceleration will be less than zero.

 * Drogue. The rocket has reached apogee and is heading back down, but
   is above the configured Main altitude. Ascent rate will be less
   than zero during this state. Vertical acceleration will be negative
   until the rocket reaches a terminal descent rate, at which point
   Vertical acceleration will be zero. Both Ascent rate and Vertical
   acceleration are very noisy in this state, so be careful when
   trying to use them to control pyro channels. This state selection
   is not available on EasyTimer.

 * Main. The rocket is still descending, and
   is below the Main altitude. Ascent rate will be less than zero
   during this state. Vertical acceleration may be briefly less than
   zero as the rocket slows from drogue descent to main descent, but
   it will settle down to a zero value once the rocket has reached the
   terminal velocity under the main chute. Ascent rate and Vertical
   acceleration should be much less noisy once the main chute has
   deployed. This state selection is not available on EasyTimer.

 * Landed. The rocket is no longer moving.

You can select a state to limit when the pyro channel may activate;
note that the check is based on when the rocket transitions *into* the
state, and so checking for “greater than Boost” means that the rocket
is currently in boost or some later state.

When a motor burns out, the rocket enters either Fast or Coast state
(depending on how fast it is moving). If the computer detects upwards
acceleration again, it will move back to Boost state.
