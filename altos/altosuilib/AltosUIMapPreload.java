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
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.*;
import java.text.*;
import java.lang.Math;
import java.net.URL;
import java.net.URLConnection;
import org.altusmetrum.altoslib_14.*;

class AltosUIMapPos extends Box implements ActionListener {
	AltosUIMapPreload	preload;
	AltosUIFrame	owner;
	JLabel		label;
	JComboBox	hemi;
	JTextField	deg;
	JLabel		deg_label;
	JTextField	min;
	JLabel		min_label;

	/* ActionListener interface */
	public void actionPerformed(ActionEvent e) {
		preload.center_map();
	}

	public void set_value(double new_value) {
		double	d, m;
		int	h;

		h = 0;
		if (new_value < 0) {
			h = 1;
			new_value = -new_value;
		}
		d = Math.floor(new_value);
		deg.setText(String.format("%3.0f", d));
		m = (new_value - d) * 60.0;
		min.setText(String.format("%7.4f", m));
		hemi.setSelectedIndex(h);
	}

	public double get_value() throws ParseException {
		int	h = hemi.getSelectedIndex();
		String	d_t = deg.getText();
		String	m_t = min.getText();
		double 	d, m, v;
		try {
			d = AltosParse.parse_double_locale(d_t);
		} catch (ParseException pe) {
			JOptionPane.showMessageDialog(owner,
						      String.format("Invalid degrees \"%s\"",
								    d_t),
						      "Invalid number",
						      JOptionPane.ERROR_MESSAGE);
			throw pe;
		}
		try {
			if (m_t.equals(""))
				m = 0;
			else
				m = AltosParse.parse_double_locale(m_t);
		} catch (ParseException pe) {
			JOptionPane.showMessageDialog(owner,
						      String.format("Invalid minutes \"%s\"",
								    m_t),
						      "Invalid number",
						      JOptionPane.ERROR_MESSAGE);
			throw pe;
		}
		v = d + m/60.0;
		if (h == 1)
			v = -v;
		return v;
	}

	public AltosUIMapPos(AltosUIFrame in_owner,
			     AltosUIMapPreload preload,
			     String label_value,
			     String[] hemi_names,
			     double default_value) {
		super(BoxLayout.X_AXIS);
		owner = in_owner;
		this.preload = preload;
		label = new JLabel(label_value);
		hemi = new JComboBox<String>(hemi_names);
		hemi.setEditable(false);
		deg = new JTextField(5);
		deg.addActionListener(this);
		deg.setMinimumSize(deg.getPreferredSize());
		deg.setHorizontalAlignment(JTextField.RIGHT);
		deg_label = new JLabel("°");
		min = new JTextField(9);
		min.addActionListener(this);
		min.setMinimumSize(min.getPreferredSize());
		min_label = new JLabel("'");
		set_value(default_value);
		add(label);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(hemi);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(deg);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(deg_label);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(min);
		add(Box.createRigidArea(new Dimension(5, 0)));
		add(min_label);
	}
}

public class AltosUIMapPreload extends AltosUIFrame implements ActionListener, ItemListener, AltosLaunchSiteListener, AltosMapLoaderListener, AltosUnitsListener, AltosFontListener  {
	AltosUIFrame	owner;
	AltosUIMap	map;

	AltosUIMapPos	lat;
	AltosUIMapPos	lon;

	JProgressBar	pbar;

	JLabel		site_list_label;
	java.util.List<AltosLaunchSite> sites;
	JComboBox<AltosLaunchSite>	site_list;

	JToggleButton	load_button;
	JButton		close_button;

/*
	JCheckBox[]	maptypes = new JCheckBox[AltosMap.maptype_terrain - AltosMap.maptype_hybrid + 1];
*/

	JComboBox<Integer>	min_zoom;
	JComboBox<Integer>	max_zoom;
	JLabel			radius_label;
	JComboBox<Double>	radius;
	int scale = 1;

	Integer[]		zooms = { -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6 };

	Double[]	radius_mi = { 1.0, 2.0, 5.0, 10.0, 20.0 };
	Double		radius_def_mi = 5.0;
	Double[]	radius_km = { 2.0, 5.0, 10.0, 20.0, 30.0 };
	Double		radius_def_km = 10.0;

	AltosMapLoader	loader;

	static final String[]	lat_hemi_names = { "N", "S" };
	static final String[]	lon_hemi_names = { "E", "W" };

	double	latitude, longitude;

	long	loader_notify_time;

	/* AltosMapLoaderListener interfaces */
	public void loader_start(final int max) {
		loader_notify_time = System.currentTimeMillis();

		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					pbar.setMaximum(max);
					pbar.setValue(0);
					pbar.setString("");
				}
			});
	}

	public void loader_notify(final int cur, final int max, final String name) {
		long	now = System.currentTimeMillis();

		if (now - loader_notify_time < 100)
			return;

		loader_notify_time = now;

		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					pbar.setValue(cur);
					pbar.setString(name);
				}
			});
	}

	public void loader_done(int max) {
		loader = null;
		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					pbar.setValue(0);
					pbar.setString("");
					load_button.setSelected(false);
				}
			});
	}

	public void debug(String format, Object ... arguments) {
		if (AltosSerial.debug)
			System.out.printf(format, arguments);
	}


	private int all_types() {
/*
		int all_types = 0;
		for (int t = AltosMap.maptype_hybrid; t <= AltosMap.maptype_terrain; t++)
			if (maptypes[t].isSelected())
				all_types |= (1 << t);
		return all_types;
*/
		return 1 << AltosMap.maptype_hybrid;
	}

	void add_mark(double lat, double lon, int state, String label) {
		map.add_mark(lat, lon, state, label);
	}

	void reset_marks() {
		map.clear_marks();
		AltosLatLon centre = null;
		String centre_name = null;
		if (map != null && map.map != null)
			centre = map.map.centre;
		for (AltosLaunchSite site : sites) {
			if (centre != null && centre.lat == site.latitude && centre.lon == site.longitude)
				centre_name = site.name;
			else
				add_mark(site.latitude, site.longitude, AltosLib.ao_flight_main, site.name);
		}
		if (centre != null)
			add_mark(centre.lat, centre.lon, AltosLib.ao_flight_boost, centre_name);
	}

	void center_map(double latitude, double longitude) {
		map.map.centre(new AltosLatLon(latitude, longitude));
		reset_marks();
	}

	void center_map() {
		try {
			center_map(lat.get_value(), lon.get_value());
		} catch (ParseException pe) {
		}
	}

	public void itemStateChanged(ItemEvent e) {
		int		state = e.getStateChange();

		if (state == ItemEvent.SELECTED) {
			Object	o = e.getItem();
			if (o instanceof AltosLaunchSite) {
				AltosLaunchSite	site = (AltosLaunchSite) o;
				lat.set_value(site.latitude);
				lon.set_value(site.longitude);
				center_map(site.latitude, site.longitude);
			}
		}
	}

	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("close")) {
			if (loader != null)
				loader.abort();
			setVisible(false);
		}

		if (cmd.equals("load")) {
			if (loader == null) {
				try {
					latitude = lat.get_value();
					longitude = lon.get_value();
					int min_z = (Integer) min_zoom.getSelectedItem();
					int max_z = (Integer) max_zoom.getSelectedItem();
					if (max_z < min_z)
						max_z = min_z;
					Double r = (Double) radius.getSelectedItem();

					if (AltosPreferences.imperial_units())
						r = AltosConvert.miles_to_meters(r);
					else
						r = r * 1000;

					center_map(latitude, longitude);

					loader = new AltosMapLoader(this,
								    latitude, longitude,
								    min_z, max_z, r,
								    all_types(), scale);

				} catch (ParseException pe) {
					load_button.setSelected(false);
				}
			}
		}
	}

	public void notify_launch_sites(final java.util.List<AltosLaunchSite> sites) {
		this.sites = sites;
		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					int	i = 1;
					for (AltosLaunchSite site : sites) {
						site_list.insertItemAt(site, i);
						i++;
					}
					reset_marks();
				}
			});
	}

	private void set_radius_values() {
		radius_label.setText(String.format("Map Radius (%s)",
						   AltosPreferences.imperial_units() ? "mi" : "km"));

		Double[]	radii;

		if (AltosPreferences.imperial_units())
			radii = radius_mi;
		else
			radii = radius_km;

		radius.removeAllItems();
		for (Double r : radii) {
			radius.addItem(r);
		}
		radius.setSelectedItem(radii[2]);
		radius.setMaximumRowCount(radii.length);
	}

	public void units_changed(boolean imperial_units) {
		map.units_changed(imperial_units);
		set_radius_values();
	}

	public void font_size_changed(int font_size) {
		map.font_size_changed(font_size);
	}

	public AltosUIMapPreload(AltosUIFrame in_owner) {
		owner = in_owner;

		Container		pane = getScrollablePane();
		GridBagConstraints	c = new GridBagConstraints();
		Insets			i = new Insets(4,4,4,4);

		setTitle("AltOS Load Maps");

		pane.setLayout(new GridBagLayout());

		addWindowListener(new WindowAdapter() {
				@Override
				public void windowClosing(WindowEvent e) {
					AltosUIPreferences.unregister_font_listener(AltosUIMapPreload.this);
					AltosPreferences.unregister_units_listener(AltosUIMapPreload.this);
				}
			});


		AltosPreferences.register_units_listener(this);
		AltosUIPreferences.register_font_listener(this);

		map = new AltosUIMap();

		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 1;

		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 10;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(map, c);

		pbar = new JProgressBar();
		pbar.setMinimum(0);
		pbar.setMaximum(1);
		pbar.setValue(0);
		pbar.setString("");
		pbar.setStringPainted(true);

		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 0;
		c.gridy = 1;
		c.gridwidth = 10;

		pane.add(pbar, c);

		site_list_label = new JLabel ("Known Launch Sites:");

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 0;
		c.gridy = 2;
		c.gridwidth = 1;

		pane.add(site_list_label, c);

		site_list = new JComboBox<AltosLaunchSite>(new AltosLaunchSite[] { new AltosLaunchSite("Site List", 0, 0) });
		site_list.addItemListener(this);

		new AltosLaunchSites(this);

		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 1;
		c.gridy = 2;
		c.gridwidth = 1;

		pane.add(site_list, c);

		lat = new AltosUIMapPos(owner, this,
					"Latitude:",
					lat_hemi_names,
					37.167833333);
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 0;
		c.weighty = 0;

		c.gridx = 0;
		c.gridy = 3;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(lat, c);

		lon = new AltosUIMapPos(owner, this,
					"Longitude:",
					lon_hemi_names,
					-97.73975);

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 0;
		c.weighty = 0;

		c.gridx = 1;
		c.gridy = 3;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(lon, c);

		load_button = new JToggleButton("Load Map");
		load_button.addActionListener(this);
		load_button.setActionCommand("load");

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 0;
		c.gridy = 4;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(load_button, c);

		close_button = new JButton("Close");
		close_button.addActionListener(this);
		close_button.setActionCommand("close");

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		c.weightx = 1;
		c.weighty = 0;

		c.gridx = 1;
		c.gridy = 4;
		c.gridwidth = 1;
		c.anchor = GridBagConstraints.CENTER;

		pane.add(close_button, c);

/*
		JLabel	types_label = new JLabel("Map Types");
		c.gridx = 2;
		c.gridwidth = 2;
		c.gridy = 2;
		pane.add(types_label, c);

		c.gridwidth = 1;

		for (int type = AltosMap.maptype_hybrid; type <= AltosMap.maptype_terrain; type++) {
			maptypes[type] = new JCheckBox(AltosMap.maptype_labels[type],
						       type == AltosMap.maptype_hybrid);
			c.gridx = 2 + (type >> 1);
			c.fill = GridBagConstraints.HORIZONTAL;
			c.gridy = (type & 1) + 3;
			pane.add(maptypes[type], c);
		}
*/

		JLabel	min_zoom_label = new JLabel("Minimum Zoom");
		c.gridx = 4;
		c.gridy = 2;
		pane.add(min_zoom_label, c);

		min_zoom = new JComboBox<Integer>(zooms);
		min_zoom.setSelectedItem(zooms[10]);
		min_zoom.setEditable(false);
		c.gridx = 5;
		c.gridy = 2;
		pane.add(min_zoom, c);

		JLabel	max_zoom_label = new JLabel("Maximum Zoom");
		c.gridx = 4;
		c.gridy = 3;
		pane.add(max_zoom_label, c);

		max_zoom = new JComboBox<Integer>(zooms);
		max_zoom.setSelectedItem(zooms[14]);
		max_zoom.setEditable(false);
		c.gridx = 5;
		c.gridy = 3;
		pane.add(max_zoom, c);

		radius_label = new JLabel();

		c.gridx = 4;
		c.gridy = 4;
		pane.add(radius_label, c);

		radius = new JComboBox<Double>();
		radius.setEditable(true);
		c.gridx = 5;
		c.gridy = 4;
		pane.add(radius, c);

		set_radius_values();

		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}
}
