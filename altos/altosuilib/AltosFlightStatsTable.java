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

package org.altusmetrum.altosuilib_14;

import java.awt.*;
import javax.swing.*;
import java.util.*;
import org.altusmetrum.altoslib_14.*;

public class AltosFlightStatsTable extends JComponent implements AltosFontListener {
	GridBagLayout	layout;

	LinkedList<FlightStat> flight_stats = new LinkedList<FlightStat>();

	class FlightStat implements AltosFontListener {
		JLabel		label;
		JTextField[]	value;

		public void font_size_changed(int font_size) {
			label.setFont(AltosUILib.label_font);
			for (int i = 0; i < value.length; i++)
				value[i].setFont(AltosUILib.value_font);
		}

		public void set(String ... values) {
			for (int j = 0; j < values.length; j++)
				value[j].setText(values[j]);
		}

		public FlightStat(GridBagLayout layout, int y, String label_text, String ... values) {
			GridBagConstraints	c = new GridBagConstraints();
			c.insets = new Insets(AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad, AltosUILib.tab_elt_pad);
			c.weighty = 1;

			label = new JLabel(label_text);
			label.setFont(AltosUILib.label_font);
			label.setHorizontalAlignment(SwingConstants.LEFT);
			c.gridx = 0; c.gridy = y;
			c.anchor = GridBagConstraints.WEST;
			c.fill = GridBagConstraints.VERTICAL;
			c.weightx = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField[values.length];
			for (int j = 0; j < values.length; j++) {
				value[j] = new JTextField(values[j]);
				value[j].setEditable(false);
				value[j].setFont(AltosUILib.value_font);
				value[j].setHorizontalAlignment(SwingConstants.RIGHT);
				c.gridx = j+1; c.gridy = y;
				c.anchor = GridBagConstraints.EAST;
				c.fill = GridBagConstraints.BOTH;
				c.weightx = 1;
				layout.setConstraints(value[j], c);
				add(value[j]);
			}
			flight_stats.add(this);
		}

	}

	public void font_size_changed(int font_size) {
		for (FlightStat f : flight_stats)
			f.font_size_changed(font_size);
	}

	static String pos(double p, String pos, String neg) {
		String	h = pos;
		if (p < 0) {
			h = neg;
			p = -p;
		}
		int deg = (int) Math.floor(p);
		double min = (p - Math.floor(p)) * 60.0;
		return String.format("%s %4d° %9.6f'", h, deg, min);
	}

	private FlightStat	max_height_stat;
	private FlightStat	max_speed_stat;
	private FlightStat	max_accel_stat;
	private FlightStat	boost_accel_stat;
	private FlightStat	drogue_descent_stat;
	private FlightStat	main_descent_stat;

	public void set_values(AltosFlightStats stats) {
		if (max_height_stat != null && stats.max_height != AltosLib.MISSING) {
			max_height_stat.set(String.format("%6.1f m", stats.max_height),
					    String.format("%5.0f ft", AltosConvert.meters_to_feet(stats.max_height)));
		}
		if (max_speed_stat != null && stats.max_speed != AltosLib.MISSING) {
			max_speed_stat.set(String.format("%6.1f m/s", stats.max_speed),
					   String.format("%5.0f fps", AltosConvert.mps_to_fps(stats.max_speed)),
					   String.format("Mach %4.1f", AltosConvert.meters_to_mach(stats.max_speed)));
		}
		if (max_accel_stat != null && stats.max_acceleration != AltosLib.MISSING) {
			max_accel_stat.set(String.format("%6.1f m/s²", stats.max_acceleration),
					   String.format("%5.0f ft/s²", AltosConvert.meters_to_feet(stats.max_acceleration)),
					   String.format("%6.2f G", AltosConvert.meters_to_g(stats.max_acceleration)));
		}
		if (boost_accel_stat != null && stats.state_accel[AltosLib.ao_flight_boost] != AltosLib.MISSING) {
			boost_accel_stat.set(String.format("%6.1f m/s²", stats.state_accel[AltosLib.ao_flight_boost]),
					     String.format("%5.0f ft/s²", AltosConvert.meters_to_feet(stats.state_accel[AltosLib.ao_flight_boost])),
					     String.format("%6.2f G", AltosConvert.meters_to_g(stats.state_accel[AltosLib.ao_flight_boost])));
		}
		if (drogue_descent_stat != null && stats.state_speed[AltosLib.ao_flight_drogue] != AltosLib.MISSING) {
			drogue_descent_stat.set(String.format("%6.1f m/s", -stats.state_speed[AltosLib.ao_flight_drogue]),
						String.format("%5.0f ft/s", -AltosConvert.meters_to_feet(stats.state_speed[AltosLib.ao_flight_drogue])));
		}
		if (main_descent_stat != null && stats.state_speed[AltosLib.ao_flight_main] != AltosLib.MISSING) {
				main_descent_stat.set(String.format("%6.1f m/s", -stats.state_speed[AltosLib.ao_flight_main]),
						      String.format("%5.0f ft/s", -AltosConvert.meters_to_feet(stats.state_speed[AltosLib.ao_flight_main])));
		}
	}

	public void set_stats(AltosFlightStats stats) {
		int y = 0;
		if (stats.serial != AltosLib.MISSING) {
			if (stats.product != null && stats.firmware_version != null)
				new FlightStat(layout, y++, "Device",
					       stats.product,
					       String.format("version %s", stats.firmware_version),
					       String.format("serial %d", stats.serial));
			else
				new FlightStat(layout, y++, "Serial", String.format("%d", stats.serial));
		}
		if (stats.flight != AltosLib.MISSING)
			new FlightStat(layout, y++, "Flight", String.format("%d", stats.flight));
		if (stats.year != AltosLib.MISSING && stats.hour != AltosLib.MISSING)
			new FlightStat(layout, y++, "Date/Time",
				       String.format("%04d-%02d-%02d", stats.year, stats.month, stats.day),
				       String.format("%02d:%02d:%02d UTC", stats.hour, stats.minute, stats.second));
		else {
			if (stats.year != AltosLib.MISSING)
				new FlightStat(layout, y++, "Date",
					       String.format("%04d-%02d-%02d", stats.year, stats.month, stats.day));
			if (stats.hour != AltosLib.MISSING)
				new FlightStat(layout, y++, "Time",
					       String.format("%02d:%02d:%02d UTC", stats.hour, stats.minute, stats.second));
		}
		if (stats.max_height != AltosLib.MISSING) {
			max_height_stat = new FlightStat(layout, y++, "Maximum height",
							 String.format("%6.1f m", stats.max_height),
							 String.format("%5.0f ft", AltosConvert.meters_to_feet(stats.max_height)));
		}
		if (stats.max_gps_height != AltosLib.MISSING) {
			new FlightStat(layout, y++, "Maximum GPS height",
				       String.format("%6.1f m", stats.max_gps_height),
				       String.format("%5.0f ft", AltosConvert.meters_to_feet(stats.max_gps_height)));
		}
		if (stats.max_speed != AltosLib.MISSING) {
			max_speed_stat = new FlightStat(layout, y++, "Maximum speed",
							String.format("%6.1f m/s", stats.max_speed),
							String.format("%5.0f fps", AltosConvert.mps_to_fps(stats.max_speed)),
							String.format("Mach %4.1f", AltosConvert.meters_to_mach(stats.max_speed)));
		}
		if (stats.max_acceleration != AltosLib.MISSING)
			max_accel_stat = new FlightStat(layout, y++, "Maximum boost acceleration",
							String.format("%6.1f m/s²", stats.max_acceleration),
							String.format("%5.0f ft/s²", AltosConvert.meters_to_feet(stats.max_acceleration)),
							String.format("%6.2f G", AltosConvert.meters_to_g(stats.max_acceleration)));
		if (stats.state_accel[AltosLib.ao_flight_boost] != AltosLib.MISSING)
			boost_accel_stat = new FlightStat(layout, y++, "Average boost acceleration",
							  String.format("%6.1f m/s²", stats.state_accel[AltosLib.ao_flight_boost]),
							  String.format("%5.0f ft/s²", AltosConvert.meters_to_feet(stats.state_accel[AltosLib.ao_flight_boost])),
							  String.format("%6.2f G", AltosConvert.meters_to_g(stats.state_accel[AltosLib.ao_flight_boost])));
		if (stats.state_time[AltosLib.ao_flight_boost] != 0 || stats.state_time[AltosLib.ao_flight_fast] != 0 || stats.state_time[AltosLib.ao_flight_coast] != 0) {

			double	boost_time = stats.state_time[AltosLib.ao_flight_boost];
			double	fast_time = stats.state_time[AltosLib.ao_flight_fast];
			double	coast_time = stats.state_time[AltosLib.ao_flight_coast];

			if (fast_time > 0) {
				new FlightStat(layout, y++, "Ascent time",
					       String.format("%6.1f s %s", boost_time,
							     AltosLib.state_name(AltosLib.ao_flight_boost)),
					       String.format("%6.1f s %s", fast_time,
							     AltosLib.state_name(AltosLib.ao_flight_fast)),
					       String.format("%6.1f s %s", coast_time,
							     AltosLib.state_name(AltosLib.ao_flight_coast)));
			} else {
				new FlightStat(layout, y++, "Ascent time",
					       String.format("%6.1f s %s", boost_time,
							     AltosLib.state_name(AltosLib.ao_flight_boost)),
					       String.format("%6.1f s %s", coast_time,
							     AltosLib.state_name(AltosLib.ao_flight_coast)));
			}
		}
		if (stats.state_speed[AltosLib.ao_flight_drogue] != AltosLib.MISSING) {
			String	label;

			if (stats.state_speed[AltosLib.ao_flight_main] == AltosLib.MISSING)
				label = "Descent rate";
			else
				label = "Drogue descent rate";
			drogue_descent_stat = new FlightStat(layout, y++, label,
				       String.format("%6.1f m/s", -stats.state_speed[AltosLib.ao_flight_drogue]),
				       String.format("%5.0f ft/s", -AltosConvert.meters_to_feet(stats.state_speed[AltosLib.ao_flight_drogue])));
		}
		if (stats.state_speed[AltosLib.ao_flight_main] != AltosLib.MISSING)
			main_descent_stat = new FlightStat(layout, y++, "Main descent rate",
							   String.format("%6.1f m/s", -stats.state_speed[AltosLib.ao_flight_main]),
							   String.format("%5.0f ft/s", -AltosConvert.meters_to_feet(stats.state_speed[AltosLib.ao_flight_main])));
		if (stats.state_time[AltosLib.ao_flight_drogue] != 0 || stats.state_time[AltosLib.ao_flight_main] != 0) {
			double	drogue_duration = stats.state_time[AltosLib.ao_flight_drogue];
			double	main_duration = stats.state_time[AltosLib.ao_flight_main];
			double	duration = drogue_duration + main_duration;

			if (drogue_duration > 0 && main_duration > 0) {
				new FlightStat(layout, y++, "Descent time",
					       String.format("%6.1f s %s", drogue_duration,
							     AltosLib.state_name(AltosLib.ao_flight_drogue)),
					       String.format("%6.1f s %s", main_duration,
							     AltosLib.state_name(AltosLib.ao_flight_main)));
			} else if (duration > 0) {
				new FlightStat(layout, y++, "Descent time",
					       String.format("%6.1f s", duration));
			}
		}
		if (stats.landed_time > stats.boost_time)
			new FlightStat(layout, y++, "Flight time",
				       String.format("%6.1f s", stats.landed_time - stats.boost_time));
		if (stats.has_gps && stats.pad_lat != AltosLib.MISSING) {
			new FlightStat(layout, y++, "Pad location",
				       pos(stats.pad_lat,"N","S"),
				       pos(stats.pad_lon,"E","W"));
		}
		if (stats.has_gps && stats.lat != AltosLib.MISSING) {
			new FlightStat(layout, y++, "Last reported location",
				       pos(stats.lat,"N","S"),
				       pos(stats.lon,"E","W"));
		}
	}

	public void tell_closing() {
		AltosUIPreferences.unregister_font_listener(this);
	}

	public void filter_changed(AltosFlightStats stats) {
		set_values(stats);
	}

	public AltosFlightStatsTable() {
		layout = new GridBagLayout();

		setLayout(layout);

		AltosUIPreferences.register_font_listener(this);
	}

	public AltosFlightStatsTable(AltosFlightStats stats) {
		this();
		set_stats(stats);
	}
}
