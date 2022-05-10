/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
import javax.swing.event.*;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import org.altusmetrum.altoslib_14.*;

import org.jfree.ui.*;
import org.jfree.chart.*;
import org.jfree.chart.plot.*;
import org.jfree.chart.axis.*;
import org.jfree.chart.renderer.*;
import org.jfree.chart.renderer.xy.*;
import org.jfree.chart.labels.*;
import org.jfree.data.xy.*;
import org.jfree.data.*;

public class AltosUIEnable extends Container implements ChangeListener {

	Insets		il, ir;
	int		y;
	int		x;
	JCheckBox	imperial_units;
	JCheckBox	show_shapes;
	JLabel		line_width_label;
	JSpinner	line_width;
	JLabel		speed_filter_label;
	JSlider		speed_filter;
	JLabel		accel_filter_label;
	JSlider		accel_filter;
	AltosFilterListener	filter_listener;
	AltosShapeListener	shape_listener;

	static final int max_rows = 14;

	public void units_changed(boolean imperial_units) {
		if (this.imperial_units != null) {
			this.imperial_units.setSelected(imperial_units);
		}
	}

	class GraphElement implements ActionListener {
		AltosUIGrapher	grapher;
		JCheckBox	enable;
		String		name;

		public void actionPerformed(ActionEvent ae) {
			grapher.set_enable(enable.isSelected());
		}

		GraphElement (String name, AltosUIGrapher grapher, boolean enabled) {
			this.name = name;
			this.grapher = grapher;
			enable = new JCheckBox(name, enabled);
			grapher.set_enable(enabled);
			enable.addActionListener(this);
		}
	}

	LinkedList<GraphElement> elements = new LinkedList<GraphElement>();

	public void add(String name, AltosUIGrapher grapher, boolean enabled) {

		GraphElement	e = new GraphElement(name, grapher, enabled);
		GridBagConstraints c = new GridBagConstraints();

		elements.add(e);

		/* Add element */
		c = new GridBagConstraints();
		c.gridx = x; c.gridy = y;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = ir;
		add(e.enable, c);

		/* Next row */
		y++;
		if (y == max_rows) {
			x++;
			y = 0;
		}
	}

	public void stateChanged(ChangeEvent e) {
		JSlider filter = (JSlider) e.getSource();
		if (!filter.getValueIsAdjusting()) {
			double	speed_value = (int) speed_filter.getValue() / 1000.0;
			double	accel_value = (int) accel_filter.getValue() / 1000.0;
			if (filter_listener != null) {
				filter_listener.filter_changed(speed_value, accel_value);
			}
		}
	}

	public void set_shapes_visible(boolean visible) {
		if (shape_listener != null)
			shape_listener.set_shapes_visible(visible);
	}

	public void set_line_width(float width) {
		if (shape_listener != null)
			shape_listener.set_line_width(width);
	}

	public void register_shape_listener(AltosShapeListener shape_listener) {
		this.shape_listener = shape_listener;
	}

	public void add_units() {
		/* Imperial units setting */

		/* Add label */
		imperial_units = new JCheckBox("Imperial Units", AltosUIPreferences.imperial_units());
		imperial_units.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JCheckBox item = (JCheckBox) e.getSource();
					boolean enabled = item.isSelected();
					AltosUIPreferences.set_imperial_units(enabled);
				}
			});
		imperial_units.setToolTipText("Use Imperial units instead of metric");
		GridBagConstraints c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 1000;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(imperial_units, c);

		show_shapes = new JCheckBox("Show Markers", false);
		show_shapes.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					JCheckBox item = (JCheckBox) e.getSource();
					boolean enabled = item.isSelected();
					set_shapes_visible(enabled);
				}
			});
		show_shapes.setToolTipText("Show marker Use Imperial units instead of metric");
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 1001;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(show_shapes, c);


		line_width_label = new JLabel("Line Width");
		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = 1001;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(line_width_label, c);

		line_width = new JSpinner();
		line_width.setValue(1);
		line_width.addChangeListener(new ChangeListener() {
				public void stateChanged(ChangeEvent e) {
					int w = (Integer) line_width.getValue();
					if (w < 1) {
						w = 1;
						line_width.setValue(w);
					}
					set_line_width(w);
				}
			});
		c = new GridBagConstraints();
		c.gridx = 2; c.gridy = 1001;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(line_width, c);

		speed_filter_label = new JLabel("Speed Filter(ms)");
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 1002;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(speed_filter_label, c);

		speed_filter = new JSlider(JSlider.HORIZONTAL, 0, 10000, (int) (filter_listener.speed_filter() * 1000.0));
		Hashtable<Integer,JLabel> label_table = new Hashtable<Integer,JLabel>();
		for (int i = 0; i <= 10000; i += 5000) {
			label_table.put(i, new JLabel(String.format("%d", i)));
		}
		speed_filter.setPaintTicks(true);
		speed_filter.setMajorTickSpacing(1000);
		speed_filter.setMinorTickSpacing(250);
		speed_filter.setLabelTable(label_table);
		speed_filter.setPaintTrack(false);
		speed_filter.setSnapToTicks(true);
		speed_filter.setPaintLabels(true);
		speed_filter.addChangeListener(this);

		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = 1002;
		c.gridwidth = 1000; c.gridheight = 1;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(speed_filter, c);

		accel_filter_label = new JLabel("Acceleration Filter(ms)");
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 1003;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(accel_filter_label, c);

		accel_filter = new JSlider(JSlider.HORIZONTAL, 0, 10000, (int) (filter_listener.accel_filter() * 1000.0));
		accel_filter.setPaintTicks(true);
		accel_filter.setMajorTickSpacing(1000);
		accel_filter.setMinorTickSpacing(250);
		accel_filter.setLabelTable(label_table);
		accel_filter.setPaintTrack(false);
		accel_filter.setSnapToTicks(true);
		accel_filter.setPaintLabels(true);
		accel_filter.addChangeListener(this);

		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = 1003;
		c.gridwidth = 1000; c.gridheight = 1;
		c.fill = GridBagConstraints.BOTH;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		add(accel_filter, c);
	}

	public AltosUIEnable(AltosFilterListener filter_listener) {
		this.filter_listener = filter_listener;
		il = new Insets(4,4,4,4);
		ir = new Insets(4,4,4,4);
		x = 0;
		y = 0;
		setLayout(new GridBagLayout());
		add_units();
	}
}
