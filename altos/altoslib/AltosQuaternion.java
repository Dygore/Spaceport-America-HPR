/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

public class AltosQuaternion {
	double	r;		/* real bit */
	double	x, y, z;	/* imaginary bits */

	/* Multiply by b */
	public AltosQuaternion multiply(AltosQuaternion b) {
		return new AltosQuaternion(
			this.r * b.r - this.x * b.x - this.y * b.y - this.z * b.z,
			this.r * b.x + this.x * b.r + this.y * b.z - this.z * b.y,
			this.r * b.y - this.x * b.z + this.y * b.r + this.z * b.x,
			this.r * b.z + this.x * b.y - this.y * b.x + this.z * b.r);
	}

	public AltosQuaternion conjugate() {
		return new AltosQuaternion(this.r,
					   -this.x,
					   -this.y,
					   -this.z);
	}

	public double normal() {
		return Math.sqrt(this.r * this.r +
				 this.x * this.x +
				 this.y * this.y +
				 this.z * this.z);
	}

	/* Scale by a real value */
	public AltosQuaternion scale(double b) {
		return new AltosQuaternion(this.r * b,
					   this.x * b,
					   this.y * b,
					   this.z * b);
	}

	/* Divide by the length to end up with a quaternion of length 1 */
	public AltosQuaternion normalize() {
		double	n = normal();
		if (n <= 0)
			return this;
		return scale(1/n);
	}

	/* dot product */
	public double dot(AltosQuaternion b) {
		return (this.r * b.r +
			this.x * b.x +
			this.y * b.y +
			this.z * b.z);
	}

	/* Rotate 'this' by 'b' */
	public AltosQuaternion rotate(AltosQuaternion b) {
		return (b.multiply(this)).multiply(b.conjugate());
	}

	/* Given two vectors (this and b), compute a quaternion
	 * representing the rotation between them
	 */
	public AltosQuaternion vectors_to_rotation(AltosQuaternion b) {
		/*
		 * The cross product will point orthogonally to the two
		 * vectors, forming our rotation axis. The length will be
		 * sin(θ), so these values are already multiplied by that.
		 */

		double x = this.y * b.z - this.z * b.y;
		double y = this.z * b.x - this.x * b.z;
		double z = this.x * b.y - this.y * b.x;

		double s_2 = x*x + y*y + z*z;
		double s = Math.sqrt(s_2);

		/* cos(θ) = a · b / (|a| |b|).
		 *
		 * a and b are both unit vectors, so the divisor is one
		 */
		double c = this.x*b.x + this.y*b.y + this.z*b.z;

		double c_half = Math.sqrt ((1 + c) / 2);
		double s_half = Math.sqrt ((1 - c) / 2);

		/*
		 * Divide out the sine factor from the
		 * cross product, then multiply in the
		 * half sine factor needed for the quaternion
		 */
		double s_scale = s_half / s;

		AltosQuaternion r = new AltosQuaternion(c_half,
							x * s_scale,
							y * s_scale,
							z * s_scale);
		return r.normalize();
	}

	public AltosQuaternion(double r, double x, double y, double z) {
		this.r = r;
		this.x = x;
		this.y = y;
		this.z = z;
	}

	public AltosQuaternion(AltosQuaternion q) {
		r = q.r;
		x = q.x;
		y = q.y;
		z = q.z;
	}

	public AltosQuaternion() {
		r = 1;
		x = 0;
		y = 0;
		z = 0;
	}

	static public AltosQuaternion vector(double x, double y, double z) {
		return new AltosQuaternion(0, x, y, z);
	}

	static public AltosQuaternion rotation(double x, double y, double z,
					       double s, double c) {
		return new AltosQuaternion(c,
					   s*x,
					   s*y,
					   s*z);
	}

	static public AltosQuaternion zero_rotation() {
		return new AltosQuaternion(1, 0, 0, 0);
	}

	static public AltosQuaternion euler(double x, double y, double z) {

		/* Halve the euler angles */
		x = x / 2.0;
		y = y / 2.0;
		z = z / 2.0;

		double	s_x = Math.sin(x), c_x = Math.cos(x);
		double	s_y = Math.sin(y), c_y = Math.cos(y);
		double	s_z = Math.sin(z), c_z = Math.cos(z);;

		return new AltosQuaternion(c_x * c_y * c_z + s_x * s_y * s_z,
					   s_x * c_y * c_z - c_x * s_y * s_z,
					   c_x * s_y * c_z + s_x * c_y * s_z,
					   c_x * c_y * s_z - s_x * s_y * c_z);
	}
}
