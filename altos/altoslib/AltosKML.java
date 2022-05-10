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

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.util.*;

class KMLWriter extends PrintWriter {
	public PrintWriter printf(String format, Object ... arguments) {
		return printf(Locale.ROOT, format, arguments);
	}

	public KMLWriter(File name) throws FileNotFoundException {
		super(name);
	}
}

public class AltosKML implements AltosWriter {

	File			name;
	PrintWriter		out;
	int			flight_state = -1;
	AltosGPS		prev = null;
	double			gps_start_altitude = AltosLib.MISSING;
	AltosFlightSeries	series;
	AltosFlightStats	stats;
	AltosCalData		cal_data;

	static final String[] kml_state_colors = {
		"FF000000",	// startup
		"FF000000",	// idle
		"FF000000",	// pad
		"FF0000FF",	// boost
		"FF8040FF",	// coast
		"FF4080FF",	// fast
		"FF00FFFF",	// drogue
		"FF00FF00",	// main
		"FF000000",	// landed
		"FFFFFFFF",	// invalid
		"FFFF0000",	// stateless
	};

	static String state_color(int state) {
		if (state < 0 || kml_state_colors.length <= state)
			return kml_state_colors[AltosLib.ao_flight_invalid];
		return kml_state_colors[state];
	}

	static final String[] kml_style_colors = {
		"FF0000FF",	// baro
		"FFFF0000",	// gps
	};

	static String style_color(int style) {
		if (style < 0 || kml_style_colors.length <= style)
			return kml_style_colors[0];
		return kml_style_colors[style];
	}

	static final String kml_header_start =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
		"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n" +
		"<Document>\n" +
		"  <name>AO Flight#%d S/N: %03d</name>\n" +
		"  <Snippet maxLines=\"8\">\n";

	static final String kml_header_end =
		"  </Snippet>\n" +
		"  <open>1</open>\n";

	static final String kml_folder_start =
		"  <Folder>\n" +
		"    <name>%s</name>\n";

	static final String kml_path_style_start =
		"    <Style id=\"ao-style-%s\">\n" +
		"      <LineStyle><color>%s</color><width>8</width></LineStyle>\n" +
		"      <BalloonStyle>\n" +
		"        <text>\n";

	static final String kml_path_style_end =
		"        </text>\n" +
		"      </BalloonStyle>\n" +
		"    </Style>\n";

	static final String kml_point_style_start =
		"    <Style id=\"ao-style-%s\">\n" +
		"      <LabelStyle><color>%s</color></LabelStyle>\n" +
		"      <IconStyle><color>%s</color></IconStyle>\n" +
		"      <BalloonStyle>\n" +
		"        <text>\n";

	static final String kml_point_style_end =
		"        </text>\n" +
		"      </BalloonStyle>\n" +
		"    </Style>\n";

	static final String kml_path_start =
		"    <Placemark>\n" +
		"      <name>%s</name>\n" +
		"      <styleUrl>#ao-style-%s</styleUrl>\n" +
		"      <LineString>\n" +
		"        <tessellate>1</tessellate>\n" +
		"        <altitudeMode>absolute</altitudeMode>\n" +
		"        <coordinates>\n";

	static final String kml_coord_fmt =
	"          %.7f,%.7f,%.7f <!-- alt %12.7f time %12.7f sats %d -->\n";

	static final String kml_path_end =
		"        </coordinates>\n" +
		"      </LineString>\n" +
		"    </Placemark>\n";

	static final String kml_point_start =
		"    <Placemark>\n" +
		"      <name>%s</name>\n" +
		"      <styleUrl>#ao-style-%s</styleUrl>\n" +
		"      <Point>\n" +
		"        <tessellate>1</tessellate>\n" +
		"        <altitudeMode>absolute</altitudeMode>\n" +
		"        <coordinates>\n";

	static final String kml_point_end =
		"        </coordinates>\n" +
		"      </Point>\n" +
		"    </Placemark>\n";

	static final String kml_folder_end =
		"  </Folder>\n";

	static final String kml_footer =
		"</Document>\n" +
		"</kml>\n";

	void start () {
		AltosGPS gps = cal_data.gps_pad;

		gps_start_altitude = cal_data.gps_pad_altitude;
		out.printf(kml_header_start, cal_data.flight, cal_data.serial);
		out.printf("Product: %s\n", stats.product);
		out.printf("Firmware: %s\n", stats.firmware_version);
		out.printf("Date: %04d-%02d-%02d\n",
			   gps.year, gps.month, gps.day);
		out.printf("Time: %2d:%02d:%02d\n",
			   gps.hour, gps.minute, gps.second);
		if (stats.max_height != AltosLib.MISSING)
			out.printf("Max baro height: %s\n", AltosConvert.height.show(6, stats.max_height));
		if (stats.max_gps_height != AltosLib.MISSING)
			out.printf("Max GPS Height: %s\n", AltosConvert.height.show(6, stats.max_gps_height));
		if (stats.max_speed != AltosLib.MISSING)
			out.printf("Max speed: %s\n", AltosConvert.speed.show(6, stats.max_speed));
		if (stats.max_acceleration != AltosLib.MISSING)
			out.printf("Max accel: %s\n", AltosConvert.accel.show(6, stats.max_acceleration));
		out.printf("%s", kml_header_end);
	}

	void folder_start(String folder_name) {
		out.printf(kml_folder_start, folder_name);
	}

	void folder_end() {
		out.printf(kml_folder_end);
	}

	void path_style_start(String style, String color) {
		out.printf(kml_path_style_start, style, color);
	}

	void path_style_end() {
		out.printf(kml_path_style_end);
	}

	void point_style_start(String style, String color) {
		out.printf(kml_point_style_start, style, color, color);
	}

	void point_style_end() {
		out.printf(kml_point_style_end);
	}

	void path_start(String name, String style) {
		out.printf(kml_path_start, name, style);
	}

	void path_end() {
		out.printf(kml_path_end);
	}

	void point_start(String name, String style) {
		out.printf(kml_point_start, name, style);
	}

	void point_end() {
		out.printf(kml_point_end);
	}

	boolean	started = false;

	private double baro_altitude(AltosFlightSeries series, double time) {
		double height = series.value(AltosFlightSeries.height_name, time);

		if (height == AltosLib.MISSING)
			return AltosLib.MISSING;
		if (cal_data.gps_pad_altitude == AltosLib.MISSING)
			return AltosLib.MISSING;

		return height + cal_data.gps_pad_altitude;
	}

	void coord(double time, AltosGPS gps, double altitude) {
		out.printf(kml_coord_fmt,
			   gps.lon, gps.lat,
			   altitude, (double) gps.alt,
			   time, gps.nsat);
	}

	void end() {
		out.printf("%s", kml_footer);
	}

	public void close() {
		if (out != null) {
			out.close();
			out = null;
		}
	}

	public void write(AltosGPS gps, double alt)
	{
		if (gps == null)
			return;
		if (gps.lat == AltosLib.MISSING)
			return;
		if (gps.lon == AltosLib.MISSING)
			return;
		if (alt == AltosLib.MISSING) {
			alt = cal_data.gps_pad_altitude;
			if (alt == AltosLib.MISSING)
				return;
		}
		coord(0, gps, alt);
		prev = gps;
	}

	public void write_point(AltosTimeValue tv, boolean is_gps) {
		int		state = (int) tv.value;
		String		style_prefix = is_gps ? "gps-" : "baro-";
		String		state_name = AltosLib.state_name(state);
		String		state_label = AltosLib.state_name_capital(state);
		String		style_name = style_prefix + state_name;
		String		folder_name = is_gps ? "GPS" : "Baro";
		String		full_name = state_label + " (" + folder_name + ")";
		AltosGPS	gps = series.gps_before(tv.time);
		double		altitude = is_gps ? gps.alt : baro_altitude(series, tv.time);

		point_style_start(style_name, state_color(state));
		out.printf("%s\n", full_name);
		switch (state) {
		case AltosLib.ao_flight_boost:
			out.printf("Max accel %s\n", AltosConvert.accel.show(6, stats.max_acceleration));
			out.printf("Max speed %s\n", AltosConvert.speed.show(6, stats.max_speed));
			break;
		case AltosLib.ao_flight_coast:
		case AltosLib.ao_flight_fast:
			out.printf("Entry speed %s\n", AltosConvert.speed.show(6, stats.state_enter_speed[state]));
			out.printf("Entry height %s\n", AltosConvert.height.show(6, altitude - cal_data.gps_pad_altitude));
			break;
		case AltosLib.ao_flight_drogue:
			out.printf("Max height %s\n", AltosConvert.height.show(6, is_gps ? stats.max_gps_height : stats.max_height));
			out.printf("Average descent rate %s\n", AltosConvert.speed.show(6, -stats.state_speed[state]));
			break;
		case AltosLib.ao_flight_main:
			out.printf("Entry speed %s\n", AltosConvert.speed.show(6, -stats.state_enter_speed[state]));
			out.printf("Entry height %s\n", AltosConvert.height.show(6, altitude - cal_data.gps_pad_altitude));
			out.printf("Average descent rate %s\n", AltosConvert.speed.show(6, -stats.state_speed[state]));
			break;
		case AltosLib.ao_flight_landed:
			out.printf("Landing speed %s\n", AltosConvert.speed.show(6, -stats.state_enter_speed[state]));
			break;
		}
		point_style_end();
		point_start(full_name, style_name);
		gps = series.gps_before(tv.time);
		write(gps, altitude);
		point_end();
	}

	public void write(AltosFlightSeries series) {
		this.series = series;
		series.finish();
		stats = new AltosFlightStats(series);
		cal_data = series.cal_data();
		start();
		if (series.height_series != null) {
			folder_start("Barometric Altitude");
			path_style_start("baro", style_color(0));
			out.printf("Barometric Altitude\n");
			out.printf("Max height: %s\n", AltosConvert.height.show(6, stats.max_height));
			path_style_end();
			path_start("Barometric Altitude", "baro");
			for (AltosGPSTimeValue gtv : series.gps_series)
				write(gtv.gps, baro_altitude(series, gtv.time));
			path_end();
			if (series.state_series != null) {
				for (AltosTimeValue tv : series.state_series) {
					write_point(tv, false);
				}
			}
			folder_end();
		}
		folder_start("GPS Altitude");
		path_style_start("gps", style_color(1));
		out.printf("GPS Altitude");
		out.printf("Max height: %s\n", AltosConvert.height.show(6, stats.max_gps_height));
		path_style_end();
		path_start("GPS Altitude", "gps");
		for (AltosGPSTimeValue gtv : series.gps_series)
			write(gtv.gps, gtv.gps.alt);
		path_end();
		if (series.state_series != null) {
			for (AltosTimeValue tv : series.state_series) {
				write_point(tv, true);
			}
		}
		folder_end();
		end();
	}

	public AltosKML(File in_name) throws FileNotFoundException {
		name = in_name;
		out = new KMLWriter(name);
	}

	public AltosKML(String in_string) throws FileNotFoundException {
		this(new File(in_string));
	}
}
