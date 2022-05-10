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


public class AltosTelemetrySensor extends AltosTelemetryStandard {
	int	state() { return uint8(5); }
	int	accel() { return int16(6); }
	int	pres() { return int16(8); }
	int	temp() { return int16(10); }
	int	v_batt() { return int16(12); }
	int	sense_d() { return int16(14); }
	int	sense_m() { return int16(16); }

	int	acceleration() { return int16(18); }
	int	speed() { return int16(20); }
	int	height_16() { return int16(22); }

	int	ground_pres() { return int16(24); }
	int	ground_accel() { return int16(26); }
	int	accel_plus_g() { return int16(28); }
	int	accel_minus_g() { return int16(30); }

	public AltosTelemetrySensor(int[] bytes) throws AltosCRCException {
		super(bytes);
	}

	public void provide_data(AltosDataListener listener) {
		super.provide_data(listener);

		listener.set_state(state());

		AltosCalData	cal_data = listener.cal_data();

		if (type() == packet_type_TM_sensor) {
			cal_data.set_ground_accel(ground_accel());
			cal_data.set_accel_plus_minus(accel_plus_g(), accel_minus_g());
			listener.set_acceleration(cal_data.acceleration(accel()));
		}
		cal_data.set_ground_pressure(AltosConvert.barometer_to_pressure(ground_pres()));
		listener.set_pressure(AltosConvert.barometer_to_pressure(pres()));
		listener.set_temperature(AltosConvert.thermometer_to_temperature(temp()));
		listener.set_battery_voltage(AltosConvert.cc_battery_to_voltage(v_batt()));
		if (type() == packet_type_TM_sensor || type() == packet_type_Tm_sensor) {
			listener.set_apogee_voltage(AltosConvert.cc_igniter_to_voltage(sense_d()));
			listener.set_main_voltage(AltosConvert.cc_igniter_to_voltage(sense_m()));
		}

		listener.set_kalman(height_16(), speed()/16.0, acceleration()/16.0);
	}
}
