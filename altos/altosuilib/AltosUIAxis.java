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

import java.io.*;
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

public class AltosUIAxis extends NumberAxis {
	String			label;
	AltosUnits		units;
	AltosUILineStyle	line_style;
	int			ref;
	int			visible;
	int			index;

	public final static int	axis_integer = 1;
	public final static int axis_include_zero = 2;

	public final static int axis_default = axis_include_zero;

	public void set_units() {
		if (units != null) {
			String u = units.parse_units();
			if (u != null) {
				setLabel(String.format("%s (%s)", label, u));
				return;
			}
		}
		setLabel(label);
	}

	public void set_enable(boolean enable) {
		if (enable) {
			visible++;
			if (visible > ref)
				System.out.printf("too many visible\n");
		} else {
			visible--;
			if (visible < 0)
				System.out.printf("too few visible\n");
		}
		setVisible(visible > 0);
		if (enable)
			autoAdjustRange();
	}

	public void ref(boolean enable) {
		++ref;
		if (enable) {
			++visible;
			setVisible(visible > 0);
		}
	}

	public AltosUIAxis(String label, AltosUnits units, AltosUILineStyle line_style, int index, int flags) {
		this.label = label;
		this.units = units;
		this.line_style = line_style;
		this.index = index;
		this.visible = 0;
		this.ref = 0;
		setLabelPaint(line_style.color);
		setTickLabelPaint(line_style.color);
		setVisible(false);
		if ((flags & axis_integer) != 0)
			setStandardTickUnits(NumberAxis.createIntegerTickUnits());
		setAutoRangeIncludesZero((flags & axis_include_zero) != 0);
	}

	public AltosUIAxis(String label, AltosUnits units, AltosUILineStyle line_style, int index) {
		this(label, units, line_style, index, axis_default);
	}
}
