/*
 * Copyright © 2021 Keith Packard <keithp@keithp.com>
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

public class AltosTelemetryMegaNorm extends AltosTelemetryStandard {
	int	orient() { return int8(5); }

	int	accel() { return int16(6); }
	int	pres() { return int32(8); }
	int	temp() { return int16(12); }

	int	accel_along() { return int16(14); }
	int	accel_across() { return int16(16); }
	int	accel_through() { return int16(18); }

	int	gyro_roll() { return int16(20); }
	int	gyro_pitch() { return int16(22); }
	int	gyro_yaw() { return int16(24); }

	int	mag_along() { return int16(26); }
	int	mag_across() { return int16(28); }
	int	mag_through() { return int16(30); }

	int imu_model, mag_model;

	public AltosTelemetryMegaNorm(int[] bytes, int imu_model, int mag_model) throws AltosCRCException {
		super(bytes);
		this.imu_model = imu_model;
		this.mag_model = mag_model;
	}

	public void provide_data(AltosDataListener listener) {
		super.provide_data(listener);

		AltosCalData cal_data = listener.cal_data();

		listener.set_acceleration(cal_data.acceleration(accel()));
		listener.set_pressure(pres());
		listener.set_temperature(temp() / 100.0);

		listener.set_orient(orient());
		cal_data.set_imu_model(imu_model);
		cal_data.set_mag_model(mag_model);

		/* XXX we have no calibration data for these values */

		if (cal_data.accel_zero_along == AltosLib.MISSING)
			cal_data.set_accel_zero(0, 0, 0);
		if (cal_data.gyro_zero_roll == AltosLib.MISSING)
			cal_data.set_gyro_zero(0, 0, 0);

		int	accel_along = accel_along();
		int	accel_across = accel_across();
		int	accel_through = accel_through();

		int	gyro_roll = gyro_roll();
		int	gyro_pitch = gyro_pitch();
		int	gyro_yaw = gyro_yaw();

		int	mag_along = mag_along();
		int	mag_across = mag_across();
		int	mag_through = mag_through();

		listener.set_accel(cal_data.accel_along(accel_along),
				   cal_data.accel_across(accel_across),
				   cal_data.accel_through(accel_through));
		listener.set_gyro(cal_data.gyro_roll(gyro_roll),
				  cal_data.gyro_pitch(gyro_pitch),
				  cal_data.gyro_yaw(gyro_yaw));
		listener.set_mag(cal_data.mag_along(mag_along),
				 cal_data.mag_across(mag_across),
				 cal_data.mag_through(mag_through));
	}
}
