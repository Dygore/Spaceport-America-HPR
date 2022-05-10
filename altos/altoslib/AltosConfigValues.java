/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

public interface AltosConfigValues {
	/* set and get all of the dialog values */
	public abstract void set_product(String product);

	public abstract void set_version(String version);

	public abstract void set_serial(int serial);

	public abstract void set_altitude_32(int altitude_32);

	public abstract void set_main_deploy(int new_main_deploy);

	public abstract int main_deploy() throws AltosConfigDataException;

	public abstract void set_apogee_delay(int new_apogee_delay);

	public abstract int apogee_delay() throws AltosConfigDataException;

	public abstract void set_apogee_lockout(int new_apogee_lockout);

	public abstract int apogee_lockout() throws AltosConfigDataException;

	public abstract void set_radio_frequency(double new_radio_frequency);

	public abstract double radio_frequency() throws AltosConfigDataException;

	public abstract void set_radio_calibration(int new_radio_calibration);

	public abstract void set_radio_enable(int new_radio_enable);

	public abstract int radio_enable();

	public abstract void set_callsign(String new_callsign);

	public abstract String callsign();

	public abstract void set_telemetry_rate(int new_telemetry_rate);

	public abstract int telemetry_rate() throws AltosConfigDataException;

	public abstract void set_flight_log_max(int new_flight_log_max);

	public abstract void set_flight_log_max_enabled(boolean enable);

	public abstract int flight_log_max() throws AltosConfigDataException;

	public abstract void set_flight_log_max_limit(int flight_log_max_limit, int storage_erase_unit);

	public abstract void set_ignite_mode(int new_ignite_mode);

	public abstract int ignite_mode();

	public abstract void set_pad_orientation(int new_pad_orientation);

	public abstract int pad_orientation();

	public abstract void set_accel_cal(int accel_cal_plus, int accel_cal_minus);

	public abstract int accel_cal_plus();

	public abstract int accel_cal_minus();

	public abstract void set_dirty();

	public abstract void set_clean();

	public abstract void set_pyros(AltosPyro[] new_pyros);

	public abstract AltosPyro[] pyros() throws AltosConfigDataException;

	public abstract void set_pyro_firing_time(double new_pyro_firing_time);

	public abstract double pyro_firing_time() throws AltosConfigDataException;

	public abstract int aprs_interval() throws AltosConfigDataException;

	public abstract void set_aprs_interval(int new_aprs_interval);

	public abstract int aprs_ssid() throws AltosConfigDataException;

	public abstract void set_aprs_ssid(int new_aprs_ssid);

	public abstract int aprs_format() throws AltosConfigDataException;

	public abstract void set_aprs_format(int new_aprs_format);

	public abstract int aprs_offset() throws AltosConfigDataException;

	public abstract void set_aprs_offset(int new_aprs_offset);

	public abstract int beep() throws AltosConfigDataException;

	public abstract void set_beep(int new_beep);

	public abstract int tracker_motion() throws AltosConfigDataException;

	public abstract void set_tracker_motion(int tracker_motion);

	public abstract int tracker_interval() throws AltosConfigDataException;

	public abstract void set_tracker_interval(int tracker_motion);

	public abstract int radio_10mw() throws AltosConfigDataException;

	public abstract void set_radio_10mw(int radio_10mw);
}
