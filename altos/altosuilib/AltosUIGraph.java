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

package org.altusmetrum.altosuilib_14;

import java.io.*;
import java.util.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
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

public class AltosUIGraph implements AltosUnitsListener, AltosShapeListener {

	XYPlot				plot;
	JFreeChart			chart;
	public ChartPanel		panel;
	NumberAxis			xAxis;
	AltosUIEnable			enable;
	AltosUITimeSeries[]		series;
	int				axis_index;
	int				series_index;
	Hashtable<Integer,Boolean>	axes_added;

	static final private Color gridline_color = new Color(0, 0, 0);
	static final private Color border_color = new Color(255, 255, 255);
	static final private Color background_color = new Color(255, 255, 255);

	public JPanel panel() {
		return panel;
	}

	public AltosUIAxis newAxis(String label, AltosUnits units, AltosUILineStyle line_style, int flags) {
		AltosUIAxis axis = new AltosUIAxis(label, units, line_style, axis_index++, flags);
		plot.setRangeAxis(axis.index, axis);
		return axis;
	}

	public AltosUIAxis newAxis(String label, AltosUnits units, AltosUILineStyle line_style) {
		return newAxis(label, units, line_style, AltosUIAxis.axis_default);
	}

	void addAxis(AltosUIAxis axis) {
		if (!axes_added.containsKey(axis.index)) {
			axes_added.put(axis.index, true);
			plot.setRangeAxis(axis.index, axis);
		}
	}

	public void addSeries(AltosUITimeSeries series) {
		XYSeriesCollection	dataset = new XYSeriesCollection(series.xy_series());

		addAxis(series.axis);

		series.renderer.setPlot(plot);
		plot.setDataset(series_index, dataset);
		plot.setRenderer(series_index, series.renderer);
		plot.mapDatasetToRangeAxis(series_index, series.axis.index);
		if (enable != null)
			enable.add(series.label, series, series.enable);
		series_index++;
	}

	public void addMarker(AltosUITimeSeries series) {
	}

	public void units_changed(boolean imperial_units) {
		for (AltosUITimeSeries s : series)
			s.set_units();
	}

	public void filter_changed() {
		units_changed(false);
	}

	public void set_shapes_visible(boolean visible) {
		for (AltosUITimeSeries s : series)
			s.set_shapes_visible(visible);
	}

	public void set_line_width(float width) {
		for (AltosUITimeSeries s : series)
			s.set_line_width(width);
	}

	public void setName (String name) {
		chart.setTitle(name);
	}

	public void set_series(AltosUITimeSeries[] series) {
		this.series = series;
		boolean any_enabled = false;

		for (AltosUITimeSeries s : series)
			if (s.enable)
				any_enabled = true;

		if (!any_enabled)
			for (AltosUITimeSeries s : series)
				s.set_enable(true);

		for (AltosUITimeSeries s : series)
			addSeries(s);

		units_changed(false);
	}

	public AltosUIGraph(AltosUIEnable enable, String title) {

		this.enable = enable;
		this.series = null;
		this.axis_index = 0;

		if (enable != null)
			enable.register_shape_listener(this);

		axes_added = new Hashtable<Integer,Boolean>();

		xAxis = new NumberAxis("Time (s)");

		xAxis.setAutoRangeIncludesZero(true);

		plot = new XYPlot();
		plot.setDomainAxis(xAxis);
		plot.setOrientation(PlotOrientation.VERTICAL);
		plot.setDomainPannable(true);
		plot.setRangePannable(true);

		chart = new JFreeChart(title, JFreeChart.DEFAULT_TITLE_FONT,
				       plot, true);

		ChartUtilities.applyCurrentTheme(chart);

		plot.setDomainGridlinePaint(gridline_color);
		plot.setRangeGridlinePaint(gridline_color);
		plot.setBackgroundPaint(background_color);
		plot.setBackgroundAlpha((float) 1);

		chart.setBackgroundPaint(background_color);
		chart.setBorderPaint(border_color);
		panel = new ChartPanel(chart);
		panel.setMouseWheelEnabled(true);
		panel.setPreferredSize(new java.awt.Dimension(800, 500));

		AltosPreferences.register_units_listener(this);

	}
}
