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

public abstract class AltosTelemetryStandard extends AltosTelemetry {
	public int int8(int off) {
		return AltosLib.int8(bytes, off + 1);
	}

	public int uint8(int off) {
		return AltosLib.uint8(bytes, off + 1);
	}

	public int int16(int off) {
		return AltosLib.int16(bytes, off + 1);
	}

	public int uint16(int off) {
		return AltosLib.uint16(bytes, off + 1);
	}

	public int uint32(int off) {
		return AltosLib.uint32(bytes, off + 1);
	}

	public int int32(int off) {
		return AltosLib.int32(bytes, off + 1);
	}

	public String string(int off, int l) {
		return AltosLib.string(bytes, off + 1, l);
	}

	public int type() { return uint8(4); }

	public int serial() { return uint16(0); }

	public int tick() { return uint16(2); }

	public static AltosTelemetry parse_hex(int[] bytes) throws AltosCRCException {
		AltosTelemetry	telem;

		int type = AltosLib.uint8(bytes, 4+1);
		switch (type) {
		case packet_type_TM_sensor:
		case packet_type_Tm_sensor:
		case packet_type_Tn_sensor:
			telem = new AltosTelemetrySensor(bytes);
			break;
		case packet_type_configuration:
			telem = new AltosTelemetryConfiguration(bytes);
			break;
		case packet_type_location:
			telem = new AltosTelemetryLocation(bytes);
			break;
		case packet_type_satellite:
			telem = new AltosTelemetrySatellite(bytes);
			break;
		case packet_type_companion:
			telem = new AltosTelemetryCompanion(bytes);
			break;
		case packet_type_mega_sensor_mpu:
			telem = new AltosTelemetryMegaSensor(bytes, AltosIMU.imu_type_telemega_v3);
			break;
		case packet_type_mega_sensor_bmx160:
			telem = new AltosTelemetryMegaSensor(bytes, AltosIMU.imu_type_telemega_v4);
			break;
		case packet_type_mega_data:
			telem = new AltosTelemetryMegaData(bytes);
			break;
		case packet_type_metrum_sensor:
			telem = new AltosTelemetryMetrumSensor(bytes);
			break;
		case packet_type_metrum_data:
			telem = new AltosTelemetryMetrumData(bytes);
			break;
		case packet_type_mini2:
			telem = new AltosTelemetryMini2(bytes);
			break;
		case packet_type_mini3:
			telem = new AltosTelemetryMini3(bytes);
			break;
		case packet_type_mega_norm_mpu6000_mmc5983:
			telem = new AltosTelemetryMegaNorm(bytes, AltosLib.model_mpu6000, AltosLib.model_mmc5983);
			break;
		default:
			telem = new AltosTelemetryRaw(bytes);
			break;
		}
		return telem;
	}

	public AltosTelemetryStandard(int[] bytes) throws AltosCRCException {
		super(bytes);
	}

	public void provide_data(AltosDataListener listener) {
		super.provide_data(listener);
	}
}
