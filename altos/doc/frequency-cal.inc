[appendix]
== Frequency Calibration

All products that have radio interfaces require calibration of the radio
frequency.  Normally, this calibration is done once during the production
process and the resulting cal value is saved into non-volatile memory.  The
procedure decribed here should only be used outside of the factory if you 
are really convinced the radio calibration is bad, and you have access to
the required tools to do the calibration.

Because this procedure is only rarely needed in the field, we have not 
written any fancy user interface for doing it .. some interaction with 
and careful typing in a command-like style interface are required!

=== Background Information

The radio system on each board uses a quartz crystal to control
a frequency synthesizer that can be programmed to a range of operating
frequencies.  While these crystals are very stable, they have an accuracy
specification that means once the base frequency they set is multiplied up
to the typical operating range of our products, any variation also gets
multiplied.  The objective of the calibration process is, indirectly, to
measure the actual operating frequency of the crystal and adjust the way
the frequency synthesizer is programmed to account for this variation.

The frequency may shift a few tens of Hz over the full operating temperature
range, and it may also shift a bit over time as the crystal ages.  But once
properly calibrated, none of those changes are likely to ever cause any 
operational problem, as the shift in operating frequency due to these factors
is tiny compared to the bandwidth of our transmitted signal.

=== Required Equipment

The calibration process requires the ability to precisely measure the actual
frequency of a steady CW carrier on or about the intended operating frequency
in the vicinity of 435 MHz.  

In production, we use an HP 5385A that is locked to a 10 MHz reference that 
is in turn locked to GPS, which provides a highly accurate calibration.  Any 
reasonably accurate frequency counter is likely to be sufficient.

You also need a computer with terminal program and USB cable to attach to
the board in question, along with a battery and power switch suitable for
powering the board up.

=== RF Calibration Procedure

Using the terminal program, connect to the board over USB.  You will find 
that you are now interacting with a command interpreter on the board.  Using
'?' will show the available commands.  Of interest for this process are the
'C' command which turns on a steady transmitted carrier on the currently
selected operating frequency, and the 'c' subcommands that allow interaction
with the saved configuration.

Use the 'c s' command to discover and note the current radio calibration 
value, and the operating frequency the board is configured for in kHz.

Set up your frequency counter with a suitable antenna near the board's 
antenna and use the 'C' command to turn on a steady carrier.  Let the
frequency stabilize, and note what it is to as many digits as are steady
on your counter's display.

To calculate the new calibration value, the equation is:

.  (intended_frequency / measured_frequency) * current_cal_value

Set the new calibration value using 'c f <value>', then use 'c w' to save
that cal value into non-volatile memory.  You can use the 'C' command again
to confirm the operating frequency is now within a few 10's of Hz of the
intended operating frequency.

