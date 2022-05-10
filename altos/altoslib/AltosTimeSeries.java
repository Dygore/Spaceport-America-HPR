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

package org.altusmetrum.altoslib_14;

import java.util.*;

public class AltosTimeSeries implements Iterable<AltosTimeValue>, Comparable<AltosTimeSeries> {
	public String			label;
	public AltosUnits		units;
	ArrayList<AltosTimeValue>	values;
	boolean				data_changed;
	double				min_time = -2;

	public int compareTo(AltosTimeSeries other) {
		return label.compareTo(other.label);
	}

	public void add(AltosTimeValue tv) {
		if (tv.time >= min_time) {
			data_changed = true;
			values.add(tv);
		}
	}

	public void erase_values() {
		data_changed = true;
		this.values = new ArrayList<AltosTimeValue>();
	}

	public void clear_changed() {
		data_changed = false;
	}

//	public boolean changed() {
//		return data_changed;
//	}

	public void add(double time, double value) {
		add(new AltosTimeValue(time, value));
	}

	public AltosTimeValue get(int i) {
		return values.get(i);
	}

	private double lerp(AltosTimeValue v0, AltosTimeValue v1, double t) {
		/* degenerate case */
		if (v0.time == v1.time)
			return (v0.value + v1.value) / 2;

		return (v0.value * (v1.time - t) + v1.value * (t - v0.time)) / (v1.time - v0.time);
	}

	private int after_index(double time) {
		int	lo = 0;
		int	hi = values.size() - 1;

		while (lo <= hi) {
			int mid = (lo + hi) / 2;

			if (values.get(mid).time < time)
				lo = mid + 1;
			else
				hi = mid - 1;
		}
		return lo;
	}

	/* Compute a value for an arbitrary time */
	public double value(double time) {
		int after = after_index(time);
		double ret;

		if (after == 0)
			ret = values.get(0).value;
		else if (after == values.size())
			ret = values.get(after - 1).value;
		else {
			AltosTimeValue b = values.get(after-1);
			AltosTimeValue a = values.get(after);
			ret = lerp(b, a, time);
		}
		return ret;
	}

	/* Find the value just before an arbitrary time */
	public double value_before(double time) {
		int after = after_index(time);

		if (after == 0)
			return values.get(0).value;
		return values.get(after-1).value;
	}

	/* Find the value just after an arbitrary time */
	public double value_after(double time) {
		int after = after_index(time);

		if (after == values.size())
			return values.get(after-1).value;
		return values.get(after).value;
	}

	public double time_of(double value) {
		double	last = AltosLib.MISSING;
		for (AltosTimeValue v : values) {
			if (v.value >= value)
				return v.time;
			last = v.time;
		}
		return last;
	}

	public int size() {
		return values.size();
	}

	public Iterator<AltosTimeValue> iterator() {
		return values.iterator();
	}

	public AltosTimeValue max() {
		AltosTimeValue max = null;
		for (AltosTimeValue tv : values)
			if (max == null || tv.value > max.value)
				max = tv;
		return max;
	}

	public AltosTimeValue max(double start_time, double end_time) {
		AltosTimeValue max = null;
		for (AltosTimeValue tv : values) {
			if (start_time <= tv.time && tv.time <= end_time)
				if (max == null || tv.value > max.value)
					max = tv;
		}
		return max;
	}

	public AltosTimeValue min() {
		AltosTimeValue min = null;
		for (AltosTimeValue tv : values) {
			if (min == null || tv.value < min.value)
				min = tv;
		}
		return min;
	}

	public AltosTimeValue min(double start_time, double end_time) {
		AltosTimeValue min = null;
		for (AltosTimeValue tv : values) {
			if (start_time <= tv.time && tv.time <= end_time)
				if (min == null || tv.value < min.value)
					min = tv;
		}
		return min;
	}

	public AltosTimeValue first() {
		if (values.size() > 0)
			return values.get(0);
		return null;
	}

	public AltosTimeValue last() {
		if (values.size() > 0)
			return values.get(values.size() - 1);
		return null;
	}

	public double average() {
		double total_value = 0;
		double total_time = 0;
		AltosTimeValue prev = null;
		for (AltosTimeValue tv : values) {
			if (prev != null) {
				total_value += (tv.value + prev.value) / 2 * (tv.time - prev.time);
				total_time += (tv.time - prev.time);
			}
			prev = tv;
		}
		if (total_time == 0)
			return AltosLib.MISSING;
		return total_value / total_time;
	}

	public double average(double start_time, double end_time) {
		double total_value = 0;
		double total_time = 0;
		AltosTimeValue prev = null;
		for (AltosTimeValue tv : values) {
			if (start_time <= tv.time && tv.time <= end_time) {
				if (prev != null) {
					total_value += (tv.value + prev.value) / 2 * (tv.time - start_time);
					total_time += (tv.time - start_time);
				}
				start_time = tv.time;
			}
			prev = tv;
		}
		if (total_time == 0)
			return AltosLib.MISSING;
		return total_value / total_time;
	}

	public AltosTimeSeries integrate(AltosTimeSeries integral) {
		double	value = 0.0;
		double	pvalue = 0.0;
		double 	time = 0.0;
		boolean start = true;

		for (AltosTimeValue v : values) {
			if (start) {
				value = 0.0;
				start = false;
			} else {
				value += (pvalue + v.value) / 2.0 * (v.time - time);
			}
			pvalue = v.value;
			time = v.time;
			integral.add(time, value);

		}
		return integral;
	}

	public AltosTimeSeries differentiate(AltosTimeSeries diff) {
		double value = 0.0;
		double time = 0.0;
		boolean start = true;

		for (AltosTimeValue v: values) {
			if (start) {
				value = v.value;
				time = v.time;
				start = false;
			} else {
				double	dx = v.time - time;
				double	dy = v.value - value;

				if (dx != 0)
					diff.add(time, dy/dx);

				time = v.time;
				value = v.value;
			}
		}
		return diff;
	}

	private int find_left(int i, double dt) {
		int j;
		double t = values.get(i).time - dt;
		for (j = i; j >= 0; j--)	{
			if (values.get(j).time < t)
				break;
		}
		return j + 1;

	}

	private int find_right(int i, double dt) {
		int j;
		double t = values.get(i).time + dt;
		for (j = i; j < values.size(); j++) {
			if (values.get(j).time > t)
				break;
		}
		return j - 1;

	}

	private static double i0(double x) {
		double	ds = 1, d = 0, s = 0;

		do {
			d += 2;
			ds = ds * (x * x) / (d * d);
			s += ds;
		} while (ds - 0.2e-8 * s > 0);
		return s;
	}

	private static double kaiser(double n, double m, double beta) {
		double alpha = m / 2;
		double t = (n - alpha) / alpha;

		if (t > 1 || t < -1)
			t = 1;
		double k = i0 (beta * Math.sqrt (1 - t*t)) / i0(beta);
		return k;
	}

	private double filter_coeff(double dist, double width) {
		return kaiser(dist + width/2.0, width, 2 * Math.PI);
	}

	public AltosTimeSeries filter(AltosTimeSeries f, double width) {

		double	half_width = width/2;
		int half_point = values.size() / 2;
		for (int i = 0; i < values.size(); i++) {
			double	center_time = values.get(i).time;
			double	left_time = center_time - half_width;
			double	right_time = center_time + half_width;
			double	total_coeff = 0.0;
			double	total_value = 0.0;

			int	left = find_left(i, half_width);
			int	right = find_right(i, half_width);

			for (int j = left; j <= right; j++) {
				double	j_time = values.get(j).time;

				if (left_time <= j_time && j_time <= right_time) {
					double	j_left = j == left ? left_time : values.get(j-1).time;
					double	j_right = j == right ? right_time : values.get(j+1).time;
					double	interval = (j_right - j_left) / 2.0;
					double	coeff = filter_coeff(j_time - center_time, width) * interval;
					double	value = values.get(j).value;
					double	partial = value * coeff;

					total_coeff += coeff;
					total_value += partial;
				}
			}
			if (total_coeff != 0.0)
				f.add(center_time, total_value / total_coeff);
		}
		return f;
	}

	public AltosTimeSeries clip(AltosTimeSeries clip, double min, double max) {
		for (AltosTimeValue v: values) {
			double value = v.value;
			if (value < min) value = min;
			if (value > max) value = max;
			clip.add(v.time, value);
		}
		return clip;
	}

	public AltosTimeSeries(String label, AltosUnits units) {
		this.label = label;
		this.units = units;
		this.values = new ArrayList<AltosTimeValue>();
	}
}
