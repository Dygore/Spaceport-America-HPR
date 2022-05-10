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

public class AltosTelemetryMegaSensor extends AltosTelemetryStandard {
	int	orient() { return int8(5); }

	int	accel() { return int16(6); }
	int	pres() { return int32(8); }
	int	temp() { return int16(12); }

	int	accel_x() { return int16(14); }
	int	accel_y() { return int16(16); }
	int	accel_z() { return int16(18); }

	int	gyro_x() { return int16(20); }
	int	gyro_y() { return int16(22); }
	int	gyro_z() { return int16(24); }

	int	mag_x() { return int16(26); }
	int	mag_z() { return int16(28); }
	int	mag_y() { return int16(30); }

	int imu_type;

	private int accel_across(int imu_type) {
		switch (imu_type) {
		case AltosIMU.imu_type_telemega_v1_v2:
		case AltosIMU.imu_type_telemega_v3:
		case AltosIMU.imu_type_easymega_v1:
			return accel_x();
		case AltosIMU.imu_type_easymega_v2:
			return -accel_y();
		case  AltosIMU.imu_type_telemega_v4:
			return -accel_y();
		default:
			return AltosLib.MISSING;
		}
	}

	private int accel_along(int imu_type) {
		switch (imu_type) {
		case AltosIMU.imu_type_telemega_v1_v2:
		case AltosIMU.imu_type_telemega_v3:
		case AltosIMU.imu_type_easymega_v1:
			return accel_y();
		case AltosIMU.imu_type_easymega_v2:
		case AltosIMU.imu_type_telemega_v4:
			return accel_x();
		default:
			return AltosLib.MISSING;
		}
	}

	private int accel_through(int imu_type) {
		return accel_z();
	}

	private int gyro_roll(int imu_type) {
		switch (imu_type) {
		case AltosIMU.imu_type_telemega_v1_v2:
		case AltosIMU.imu_type_telemega_v3:
		case AltosIMU.imu_type_easymega_v1:
			return gyro_y();
		case AltosIMU.imu_type_easymega_v2:
		case AltosIMU.imu_type_telemega_v4:
			return gyro_x();
		default:
			return AltosLib.MISSING;
		}
	}

	private int gyro_pitch(int imu_type) {
		switch (imu_type) {
		case AltosIMU.imu_type_telemega_v1_v2:
		case AltosIMU.imu_type_telemega_v3:
		case AltosIMU.imu_type_easymega_v1:
			return gyro_x();
		case AltosIMU.imu_type_easymega_v2:
			return -gyro_y();
		case AltosIMU.imu_type_telemega_v4:
			return gyro_y();
		default:
			return AltosLib.MISSING;
		}
	}

	private int gyro_yaw(int imu_type) {
		return gyro_z();
	}

	private int mag_across(int imu_type) {
		switch (imu_type) {
		case AltosIMU.imu_type_telemega_v1_v2:
		case AltosIMU.imu_type_telemega_v3:
		case AltosIMU.imu_type_easymega_v1:
			return mag_x();
		case AltosIMU.imu_type_telemega_v4:
		case AltosIMU.imu_type_easymega_v2:
			return -mag_y();
		default:
			return AltosLib.MISSING;
		}
	}

	private int mag_along(int imu_type) {
		switch (imu_type) {
		case AltosIMU.imu_type_telemega_v1_v2:
		case AltosIMU.imu_type_telemega_v3:
		case AltosIMU.imu_type_easymega_v1:
			return mag_y();
		case AltosIMU.imu_type_easymega_v2:
		case AltosIMU.imu_type_telemega_v4:
			return mag_x();
		default:
			return AltosLib.MISSING;
		}
	}

	private int mag_through(int imu_type) {
		return mag_z();
	}

	public AltosTelemetryMegaSensor(int[] bytes, int imu_type) throws AltosCRCException {
		super(bytes);
		switch (imu_type) {
		case AltosIMU.imu_type_telemega_v1_v2:
		case AltosIMU.imu_type_telemega_v3:
			if (serial() < 3000)
				imu_type = AltosIMU.imu_type_telemega_v1_v2;
			else
				imu_type = AltosIMU.imu_type_telemega_v3;
			break;
		default:
			break;
		}
		this.imu_type = imu_type;
	}

	public void provide_data(AltosDataListener listener) {
		super.provide_data(listener);

		AltosCalData cal_data = listener.cal_data();

		listener.set_acceleration(cal_data.acceleration(accel()));
		listener.set_pressure(pres());
		listener.set_temperature(temp() / 100.0);

		listener.set_orient(orient());
		cal_data.set_imu_type(imu_type);

		/* XXX we have no calibration data for these values */

		if (cal_data.accel_zero_along == AltosLib.MISSING)
			cal_data.set_accel_zero(0, 0, 0);
		if (cal_data.gyro_zero_roll == AltosLib.MISSING)
			cal_data.set_gyro_zero(0, 0, 0);

		int	accel_along = accel_along(imu_type);
		int	accel_across = accel_across(imu_type);
		int	accel_through = accel_through(imu_type);

		int	gyro_roll = gyro_roll(imu_type);
		int	gyro_pitch = gyro_pitch(imu_type);
		int	gyro_yaw = gyro_yaw(imu_type);

		int	mag_along = mag_along(imu_type);
		int	mag_across = mag_across(imu_type);
		int	mag_through = mag_through(imu_type);

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
