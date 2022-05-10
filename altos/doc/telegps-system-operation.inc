[appendix]
== TeleGPS System Operation

	=== GFSK Telemetry

		TeleGPS's native telemetry system doesn't use a
		'normal packet radio' mode like APRS because it's not
		very efficient.  The GFSK modulation we use is FSK
		with the base-band pulses passed through a Gaussian
		filter before they go into the modulator to limit the
		transmitted bandwidth.  When combined with forward
		error correction and interleaving, this allows us to
		have a very robust 19.2 kilobit data link with only
		10-40 milliwatts of transmit power, a whip antenna in
		the rocket, and a hand-held Yagi on the ground.  We've
		had flights to above 21k feet AGL with great
		reception, and calculations suggest we should be good
		to well over 40k feet AGL with a 5-element yagi on the
		ground with our 10mW units and over 100k feet AGL with
		the 40mW devices.
      
	:aprsdevices: TeleGPS
	:configure_section: _configure_device
	include::aprs-operation.adoc[]

	=== Configurable Parameters

        Configuring TeleGPS is very simple; the few configurable
        parameters can all be set using the TeleGPS application over
        USB. Check out <<_configure_device>>.
