= Release Notes for Version 1.9.6
include::release-head.adoc[]
:doctype: article

	Version 1.9.6

	== AltOS

	* Fix EasyTimer bug where it might mis-detect boost (either
          detect it early or not at all) due to small errors in
          accelerometer calibration leading to large accumulated error
          in speed.

	* Adjust self-test of new 9-axis IMU (BMX-160) so that it
	  doesn't think the part has a failure when tested sitting
	  horizontally.
