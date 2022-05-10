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

public class AltosUILineStyle {
	public Color	color;
	public float[]	dash;

	static private Color color(int r, int g, int b) {
		return new Color(r,g,b);
	}

	static final private Color[] colors = {
		new Color(0,0,0),
		new Color(230,0,0),	// red
		new Color(216,103,0),	// orange
		new Color(200,200,0),	// yellow
		new Color(0,180,0),	// green
		new Color(0,140,140),	// cyan
		new Color(130,0,0),	// dark red
		new Color(0,100,0),	// dark green
		new Color(0,0,255),	// blue
		new Color(140,0,140),	// magenta
		new Color(150,150,150),	// gray
	};

	static final private float[][] dashes = {
		{ 0 },
		{ 2, 4 },
		{ 4, 4 },
		{ 6, 4 },
		{ 6, 4, 2, 4 }
	};

	static int color_index, dash_index;

	public AltosUILineStyle () {
		color = colors[color_index];
		dash = dashes[dash_index];
		color_index = (color_index + 1) % colors.length;
		if (color_index == 0) {
			dash_index = (dash_index + 1) % dashes.length;
			if (dash_index == 0)
				System.out.printf("too many line styles\n");
		}
	}

	public AltosUILineStyle(int index) {
		index = index % (colors.length * dashes.length);
		int c = index % colors.length;
		int d = index / colors.length;
		color = colors[c];
		dash = dashes[d];
	}
}
