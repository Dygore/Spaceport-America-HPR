/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

package org.altusmetrum.altosuilib_14;

import java.util.*;
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

class AltosUITimeSeriesAxis {
	AltosUILineStyle	line_style;
	boolean			enabled;
	boolean			marker;
	boolean			marker_top;
	AltosUIAxis		axis;
	XYPlot			plot;

	public AltosUITimeSeriesAxis(AltosUILineStyle line_style, boolean enabled,
				     AltosUIAxis axis, XYPlot plot, boolean marker, boolean marker_top) {
		this.line_style = line_style;
		this.enabled = enabled;
		this.axis = axis;
		this.plot = plot;
		this.marker = marker;
		this.marker_top = marker_top;
	}
}

public class AltosUIFlightSeries extends AltosFlightSeries {

	Hashtable<String,AltosUITimeSeriesAxis> axes;

	void fill_axes(String label, AltosUITimeSeriesAxis axis) {
		for (AltosTimeSeries ts : series) {
			AltosUITimeSeries uts = (AltosUITimeSeries) ts;

			if (label.equals(ts.label) || (label.equals("default") && uts.line_style == null)) {
				uts.custom_axis_set = true;
				if (axis.marker)
					uts.set_marker(axis.line_style, axis.enabled, axis.plot, axis.marker_top);
				else
					uts.set_axis(axis.line_style, axis.enabled, axis.axis);
			}
		}
	}

	void check_axes() {
		for (AltosTimeSeries ts : series) {
			AltosUITimeSeries uts = (AltosUITimeSeries) ts;

			if (!uts.custom_axis_set)
				System.out.printf("%s using default axis\n", ts.label);
		}
	}

	public void register_axis(String label,
				  AltosUILineStyle line_style,
				  boolean enabled,
				  AltosUIAxis axis) {
		AltosUITimeSeriesAxis tsa = new AltosUITimeSeriesAxis(line_style,
								      enabled,
								      axis,
								      null,
								      false,
								      false);
		axes.put(label, tsa);
		fill_axes(label, tsa);
	}

	public void register_marker(String label,
				    AltosUILineStyle line_style,
				    boolean enabled,
				    XYPlot plot,
				    boolean marker_top) {
		AltosUITimeSeriesAxis tsa = new AltosUITimeSeriesAxis(line_style,
								      enabled,
								      null,
								      plot,
								      true,
								      marker_top);
		axes.put(label, tsa);
		fill_axes(label, tsa);
	}

	public AltosTimeSeries make_series(String label, AltosUnits units) {

		AltosUITimeSeries time_series = new AltosUITimeSeries(label, units);

		AltosUITimeSeriesAxis tsa = axes.get(label);
		if (tsa == null)
			tsa = axes.get("default");
		else
			time_series.custom_axis_set = true;
		if (tsa != null) {
			if (tsa.marker)
				time_series.set_marker(tsa.line_style, tsa.enabled, tsa.plot, tsa.marker_top);
			else
				time_series.set_axis(tsa.line_style, tsa.enabled, tsa.axis);
		}
		return time_series;
	}

	public AltosUITimeSeries[] series(AltosCalData cal_data) {
		finish();
		return series.toArray(new AltosUITimeSeries[0]);
	}

	public AltosUIFlightSeries (AltosCalData cal_data) {
		super(cal_data);
		axes = new Hashtable<String,AltosUITimeSeriesAxis>();
	}
}
