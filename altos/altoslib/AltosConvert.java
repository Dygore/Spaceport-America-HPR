/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

/*
 * Sensor data conversion functions
 */
package org.altusmetrum.altoslib_14;

import java.util.*;

public class AltosConvert {

	public static final double gravity = 9.80665;

	/*
	 * Pressure Sensor Model, version 1.1
	 *
	 * written by Holly Grimes
	 *
	 * Uses the International Standard Atmosphere as described in
	 *   "A Quick Derivation relating altitude to air pressure" (version 1.03)
	 *    from the Portland State Aerospace Society, except that the atmosphere
	 *    is divided into layers with each layer having a different lapse rate.
	 *
	 * Lapse rate data for each layer was obtained from Wikipedia on Sept. 1, 2007
	 *    at site <http://en.wikipedia.org/wiki/International_Standard_Atmosphere
	 *
	 * Height measurements use the local tangent plane.  The postive z-direction is up.
	 *
	 * All measurements are given in SI units (Kelvin, Pascal, meter, meters/second^2).
	 *   The lapse rate is given in Kelvin/meter, the gas constant for air is given
	 *   in Joules/(kilogram-Kelvin).
	 */

	private static final double GRAVITATIONAL_ACCELERATION = -gravity;
	private static final double AIR_GAS_CONSTANT		= 287.053;
	private static final double NUMBER_OF_LAYERS		= 7;
	private static final double MAXIMUM_ALTITUDE		= 84852.0;
	private static final double MINIMUM_PRESSURE		= 0.3734;
	private static final double LAYER0_BASE_TEMPERATURE	= 288.15;
	private static final double LAYER0_BASE_PRESSURE	= 101325;

	/* lapse rate and base altitude for each layer in the atmosphere */
	private static final double[] lapse_rate = {
		-0.0065, 0.0, 0.001, 0.0028, 0.0, -0.0028, -0.002
	};

	private static final int[] base_altitude = {
		0, 11000, 20000, 32000, 47000, 51000, 71000
	};

	/* outputs atmospheric pressure associated with the given altitude.
	 * altitudes are measured with respect to the mean sea level
	 */
	public static double
	altitude_to_pressure(double altitude)
	{
		double base_temperature = LAYER0_BASE_TEMPERATURE;
		double base_pressure = LAYER0_BASE_PRESSURE;

		double pressure;
		double base; /* base for function to determine pressure */
		double exponent; /* exponent for function to determine pressure */
		int layer_number; /* identifies layer in the atmosphere */
		double delta_z; /* difference between two altitudes */

		if (altitude > MAXIMUM_ALTITUDE) /* FIX ME: use sensor data to improve model */
			return 0;

		/* calculate the base temperature and pressure for the atmospheric layer
		   associated with the inputted altitude */
		for(layer_number = 0; layer_number < NUMBER_OF_LAYERS - 1 && altitude > base_altitude[layer_number + 1]; layer_number++) {
			delta_z = base_altitude[layer_number + 1] - base_altitude[layer_number];
			if (lapse_rate[layer_number] == 0.0) {
				exponent = GRAVITATIONAL_ACCELERATION * delta_z
					/ AIR_GAS_CONSTANT / base_temperature;
				base_pressure *= Math.exp(exponent);
			}
			else {
				base = (lapse_rate[layer_number] * delta_z / base_temperature) + 1.0;
				exponent = GRAVITATIONAL_ACCELERATION /
					(AIR_GAS_CONSTANT * lapse_rate[layer_number]);
				base_pressure *= Math.pow(base, exponent);
			}
			base_temperature += delta_z * lapse_rate[layer_number];
		}

		/* calculate the pressure at the inputted altitude */
		delta_z = altitude - base_altitude[layer_number];
		if (lapse_rate[layer_number] == 0.0) {
			exponent = GRAVITATIONAL_ACCELERATION * delta_z
				/ AIR_GAS_CONSTANT / base_temperature;
			pressure = base_pressure * Math.exp(exponent);
		}
		else {
			base = (lapse_rate[layer_number] * delta_z / base_temperature) + 1.0;
			exponent = GRAVITATIONAL_ACCELERATION /
				(AIR_GAS_CONSTANT * lapse_rate[layer_number]);
			pressure = base_pressure * Math.pow(base, exponent);
		}

		return pressure;
	}


/* outputs the altitude associated with the given pressure. the altitude
   returned is measured with respect to the mean sea level */
	public static double
	pressure_to_altitude(double pressure)
	{

		double next_base_temperature = LAYER0_BASE_TEMPERATURE;
		double next_base_pressure = LAYER0_BASE_PRESSURE;

		double altitude;
		double base_pressure;
		double base_temperature;
		double base; /* base for function to determine base pressure of next layer */
		double exponent; /* exponent for function to determine base pressure
				    of next layer */
		double coefficient;
		int layer_number; /* identifies layer in the atmosphere */
		int delta_z; /* difference between two altitudes */

		if (pressure < 0)  /* illegal pressure */
			return -1;
		if (pressure < MINIMUM_PRESSURE) /* FIX ME: use sensor data to improve model */
			return MAXIMUM_ALTITUDE;

		/* calculate the base temperature and pressure for the atmospheric layer
		   associated with the inputted pressure. */
		layer_number = -1;
		do {
			layer_number++;
			base_pressure = next_base_pressure;
			base_temperature = next_base_temperature;
			delta_z = base_altitude[layer_number + 1] - base_altitude[layer_number];
			if (lapse_rate[layer_number] == 0.0) {
				exponent = GRAVITATIONAL_ACCELERATION * delta_z
					/ AIR_GAS_CONSTANT / base_temperature;
				next_base_pressure *= Math.exp(exponent);
			}
			else {
				base = (lapse_rate[layer_number] * delta_z / base_temperature) + 1.0;
				exponent = GRAVITATIONAL_ACCELERATION /
					(AIR_GAS_CONSTANT * lapse_rate[layer_number]);
				next_base_pressure *= Math.pow(base, exponent);
			}
			next_base_temperature += delta_z * lapse_rate[layer_number];
		}
		while(layer_number < NUMBER_OF_LAYERS - 1 && pressure < next_base_pressure);

		/* calculate the altitude associated with the inputted pressure */
		if (lapse_rate[layer_number] == 0.0) {
			coefficient = (AIR_GAS_CONSTANT / GRAVITATIONAL_ACCELERATION)
				* base_temperature;
			altitude = base_altitude[layer_number]
				+ coefficient * Math.log(pressure / base_pressure);
		}
		else {
			base = pressure / base_pressure;
			exponent = AIR_GAS_CONSTANT * lapse_rate[layer_number]
				/ GRAVITATIONAL_ACCELERATION;
			coefficient = base_temperature / lapse_rate[layer_number];
			altitude = base_altitude[layer_number]
				+ coefficient * (Math.pow(base, exponent) - 1);
		}

		return altitude;
	}

	public static double degrees_to_radians(double degrees) {
		if (degrees == AltosLib.MISSING)
			return AltosLib.MISSING;
		return degrees * (Math.PI / 180.0);
	}

	public static double radians_to_degrees(double radians) {
		if (radians == AltosLib.MISSING)
			return AltosLib.MISSING;
		return radians * (180.0 / Math.PI);
	}

	public static double
	cc_battery_to_voltage(double battery)
	{
		return battery / 32767.0 * 5.0;
	}

	public static double
	cc_igniter_to_voltage(double ignite)
	{
		return ignite / 32767 * 15.0;
	}

	public static double
	barometer_to_pressure(double count)
	{
		return ((count / 16.0) / 2047.0 + 0.095) / 0.009 * 1000.0;
	}

	static double
	thermometer_to_temperature(double thermo)
	{
		return (thermo - 19791.268) / 32728.0 * 1.25 / 0.00247;
	}

	static double mega_adc(int raw) {
		return raw / 4095.0;
	}

	static public double mega_battery_voltage(int v_batt) {
		if (v_batt != AltosLib.MISSING)
			return 3.3 * mega_adc(v_batt) * (5.6 + 10.0) / 10.0;
		return AltosLib.MISSING;
	}

	static double mega_pyro_voltage(int raw) {
		if (raw != AltosLib.MISSING)
			return 3.3 * mega_adc(raw) * (100.0 + 27.0) / 27.0;
		return AltosLib.MISSING;
	}

	static double tele_mini_3_adc(int raw) {
		return raw / 4095.0;
	}

	static public double tele_mini_3_battery_voltage(int v_batt) {
		if (v_batt != AltosLib.MISSING)
			return 3.3 * tele_mini_3_adc(v_batt) * (5.6 + 10.0) / 10.0;
		return AltosLib.MISSING;
	}

	static double tele_mini_3_pyro_voltage(int raw) {
		if (raw != AltosLib.MISSING)
			return 3.3 * tele_mini_3_adc(raw) * (100.0 + 27.0) / 27.0;
		return AltosLib.MISSING;
	}

	static double tele_mini_2_voltage(int sensor) {
		double	supply = 3.3;

		return sensor / 32767.0 * supply * 127/27;
	}

	static double tele_gps_1_voltage(int sensor) {
		double	supply = 3.3;

		return sensor / 32767.0 * supply * (5.6 + 10.0) / 10.0;
	}

	static double tele_gps_2_voltage(int sensor) {
		double	supply = 3.3;

		return sensor / 4095.0 * supply * (5.6 + 10.0) / 10.0;
	}

	static double tele_bt_3_battery(int raw) {
		if (raw == AltosLib.MISSING)
			return AltosLib.MISSING;
		return 3.3 * mega_adc(raw) * (5.1 + 10.0) / 10.0;
	}

	static double easy_timer_voltage(int sensor) {
		return 3.3 * mega_adc(sensor) * (100.0 + 27.0) / 27.0;
	}

	static double easy_mini_2_adc(double raw) {
		return raw / 4095.0;
	}

	static double easy_mini_1_adc(double raw) {
		return raw / 32767.0;
	}

	static double easy_mini_1_voltage(int sensor, int serial) {
		double	supply = 3.3;
		double	diode_offset = 0.0;

		/* early prototypes had a 3.0V regulator */
		if (serial < 1000)
			supply = 3.0;

		/* Purple v1.0 boards had the sensor after the
		 * blocking diode, which drops about 150mV
		 */
		if (serial < 1665)
			diode_offset = 0.150;

		return easy_mini_1_adc(sensor) * supply * 127/27 + diode_offset;
	}

	static double easy_mini_2_voltage(int sensor) {
		double	supply = 3.3;

		return easy_mini_2_adc(sensor) * supply * 127/27;
	}

	static double motor_pressure(double voltage) {
		double	base = 0.5;
		double	max = 4.5;
		double	full_scale_pressure = psi_to_pa(1600);

		if (voltage < base)
			voltage = base;
		if (voltage > max)
			voltage = max;
		return (voltage - base) / (max - base) * full_scale_pressure;
	}

	static double easy_motor_2_motor_pressure(int sensor, double ground_sensor) {
		double	supply = 3.3;
		double	ground_voltage = easy_mini_2_adc(ground_sensor) * supply * 15.6 / 10.0;
		double	voltage = easy_mini_2_adc(sensor) * supply * 15.6 / 10.0;

		return motor_pressure(voltage) - motor_pressure(ground_voltage);
	}

	public static double radio_to_frequency(int freq, int setting, int cal, int channel) {
		double	f;

		if (freq > 0)
			f = freq / 1000.0;
		else {
			if (setting <= 0)
				setting = cal;
			f = 434.550 * setting / cal;
			/* Round to nearest 50KHz */
			f = Math.floor (20.0 * f + 0.5) / 20.0;
		}
		return f + channel * 0.100;
	}

	public static int radio_frequency_to_setting(double frequency, int cal) {
		double	set = frequency / 434.550 * cal;

		return (int) Math.floor (set + 0.5);
	}

	public static int radio_frequency_to_channel(double frequency) {
		int	channel = (int) Math.floor ((frequency - 434.550) / 0.100 + 0.5);

		if (channel < 0)
			channel = 0;
		if (channel > 9)
			channel = 9;
		return channel;
	}

	public static double radio_channel_to_frequency(int channel) {
		return 434.550 + channel * 0.100;
	}

	public static int telem_to_rssi(int telem) {
		return telem / 2 - 74;
	}

	public static int[] ParseHex(String line) {
		String[] tokens = line.split("\\s+");
		int[] array = new int[tokens.length];

		for (int i = 0; i < tokens.length; i++)
			try {
				array[i] = Integer.parseInt(tokens[i], 16);
			} catch (NumberFormatException ne) {
				return null;
			}
		return array;
	}

	public static double meters_to_feet(double meters) {
		return meters * (100 / (2.54 * 12));
	}

	public static double feet_to_meters(double feet) {
		return feet * 12 * 2.54 / 100.0;
	}

	public static double meters_to_miles(double meters) {
		return meters_to_feet(meters) / 5280;
	}

	public static double miles_to_meters(double miles) {
		return feet_to_meters(miles * 5280);
	}

	public static double meters_to_mph(double mps) {
		return meters_to_miles(mps) * 3600;
 	}

	public static double mph_to_meters(double mps) {
		return miles_to_meters(mps) / 3600;
 	}

	public static double mps_to_fps(double mps) {
		return meters_to_miles(mps) * 5280;
 	}

	public static double fps_to_mps(double mps) {
		return miles_to_meters(mps) / 5280;
 	}

	public static double meters_to_mach(double meters) {
		return meters / 343;		/* something close to mach at usual rocket sites */
	}

	public static double meters_to_g(double meters) {
		return meters / 9.80665;
	}

	public static double c_to_f(double c) {
		return c * 9/5 + 32;
	}

	public static double f_to_c(double c) {
		return (c - 32) * 5/9;
	}

	public static double psi_to_pa(double psi) {
		return psi * 6894.76;
	}

	public static double pa_to_psi(double pa) {
		return pa / 6894.76;
	}

	public static double n_to_lb(double n) {
		return n * 0.22480894;
	}

	public static double lb_to_n(double lb) {
		return lb / 0.22480894;
	}

	public static double acceleration_from_sensor(double sensor, double plus_g, double minus_g, double ground) {

		if (sensor == AltosLib.MISSING)
			return AltosLib.MISSING;

		if (plus_g == AltosLib.MISSING || minus_g == AltosLib.MISSING)
			return AltosLib.MISSING;

		if (ground == AltosLib.MISSING)
			ground = plus_g;

		double counts_per_g = (plus_g - minus_g) / 2.0;
		double counts_per_mss = counts_per_g / gravity;

		if (counts_per_mss == 0)
			return AltosLib.MISSING;

		return (sensor - ground) / counts_per_mss;
	}

	public static boolean imperial_units = false;

	public static AltosDistance distance = new AltosDistance();

	public static AltosHeight height = new AltosHeight();

	public static AltosPressure pressure = new AltosPressure();

	public static AltosForce force = new AltosForce();

	public static AltosSpeed speed = new AltosSpeed();

	public static AltosAccel accel = new AltosAccel();

	public static AltosTemperature temperature = new AltosTemperature();

	public static AltosOrient orient = new AltosOrient();

	public static AltosVoltage voltage = new AltosVoltage();

	public static AltosLatitude latitude = new AltosLatitude();

	public static AltosLongitude longitude = new AltosLongitude();

	public static AltosRotationRate rotation_rate = new AltosRotationRate();

	public static AltosStateName state_name = new AltosStateName();

	public static AltosPyroName pyro_name = new AltosPyroName();

	public static AltosUnits magnetic_field = new AltosGauss();

	public static String show_gs(String format, double a) {
		a = meters_to_g(a);
		format = format.concat(" g");
		return String.format(format, a);
	}

	public static String say_gs(double a) {
		return String.format("%6.0 gees", meters_to_g(a));
	}

	public static int checksum(int[] data, int start, int length) {
		int	csum = 0x5a;
		for (int i = 0; i < length; i++)
			csum += data[i + start];
		return csum & 0xff;
	}

	public static int checksum(List<Byte> data, int start, int length) {
		int	csum = 0x5a;
		for (int i = 0; i < length; i++)
			csum += data.get(i+start);
		return csum & 0xff;
	}

	public static double beep_value_to_freq(int value) {
		if (value == 0)
			return 4000;
		return 1.0/2.0 * (24.0e6/32.0) / (double) value;
	}

	public static int beep_freq_to_value(double freq) {
		if (freq == 0)
			return 94;
		return (int) Math.floor (1.0/2.0 * (24.0e6/32.0) / freq + 0.5);
	}

	public static final int BEARING_LONG = 0;
	public static final int BEARING_SHORT = 1;
	public static final int BEARING_VOICE = 2;

	public static String bearing_to_words(int length, double bearing) {
		String [][] bearing_string = {
			{
				"North", "North North East", "North East", "East North East",
				"East", "East South East", "South East", "South South East",
				"South", "South South West", "South West", "West South West",
				"West", "West North West", "North West", "North North West"
			}, {
				"N", "NNE", "NE", "ENE",
				"E", "ESE", "SE", "SSE",
				"S", "SSW", "SW", "WSW",
				"W", "WNW", "NW", "NNW"
			}, {
				"north", "nor nor east", "north east", "east nor east",
				"east", "east sow east", "south east", "sow sow east",
				"south", "sow sow west", "south west", "west sow west",
				"west", "west nor west", "north west", "nor nor west "
			}
		};
		return bearing_string[length][(int)((bearing / 90 * 8 + 1) / 2)%16];
	}
}
