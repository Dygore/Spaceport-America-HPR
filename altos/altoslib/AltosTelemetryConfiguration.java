/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package org.altusmetrum.altoslib_14;


public class AltosTelemetryConfiguration extends AltosTelemetryStandard {
	int	device_type() { return uint8(5); }
	int	flight() { return uint16(6); }
	int	config_major() { return uint8(8); }
	int	config_minor() { return uint8(9); }
	int	apogee_delay() { return uint16(10); }
	int	main_deploy() { return uint16(12); }
	int	v_batt() { return uint16(10); }
	int	flight_log_max() { return uint16(14); }
	String	callsign() { return string(16, 8); }
	String	version() { return string(24, 8); }

	public AltosTelemetryConfiguration(int[] bytes) throws AltosCRCException {
		super(bytes);
	}

	public void provide_data(AltosDataListener listener) {
		super.provide_data(listener);

		AltosCalData cal_data = listener.cal_data();

		listener.set_device_type(device_type());
		cal_data.set_flight(flight());
		cal_data.set_config(config_major(), config_minor(), flight_log_max());
		if (device_type() == AltosLib.product_telegps) {
			int v_batt = v_batt();
			double batt;
			if (v_batt > 4095)
				batt = AltosConvert.tele_gps_1_voltage(v_batt);
			else
				batt = AltosConvert.tele_gps_2_voltage(v_batt);
			listener.set_battery_voltage(batt);
		} else
			cal_data.set_flight_params(apogee_delay() / 100.0, main_deploy());

		cal_data.set_callsign(callsign());
		cal_data.set_firmware_version(version());
	}
}
