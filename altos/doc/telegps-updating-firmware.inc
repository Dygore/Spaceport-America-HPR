[appendix]
== Updating Device Firmware

	TeleGPS is programmed directly over its USB connectors.

	You may wish to begin by ensuring you have current firmware images.
	These are distributed as part of the TeleGPS software bundle that
	also includes the TeleGPS ground station program.  Newer ground
	station versions typically work fine with older firmware versions,
	so you don't need to update your devices just to try out new
	software features.  You can always download the most recent
	version from http://www.altusmetrum.org/AltOS/
    
	=== Updating TeleGPS Firmware

		. Attach a battery and power switch to the target
		  device. Power up the device.

		. Using a Micro USB cable, connect the target device to
		  your computer's USB socket.

		. Run TeleGPS, and select 'Flash Device' from the
		  Device menu.

		. Select the target device in the Device Selection
		  dialog.

		. Select the image you want to flash to the device,
		  which should have a name in the form
		  <product>-v<product-version>-<software-version>.ihx,
		  such as TeleGPS-v1.0-1.4.0.ihx.

		. Make sure the configuration parameters are reasonable
		  looking. If the serial number and/or RF configuration
		  values aren't right, you'll need to change them.

		. Hit the 'OK' button and the software should proceed
		  to flash the device with new firmware, showing a
		  progress bar.

		. Verify that the device is working by using the
		  'Configure Device item to check over the
		  configuration.
