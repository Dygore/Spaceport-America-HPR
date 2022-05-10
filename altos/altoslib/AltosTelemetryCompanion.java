/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
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

public class AltosTelemetryCompanion extends AltosTelemetryStandard {

	AltosCompanion	companion() {
		int	channels = uint8(7);

		if (channels > max_channels)
			channels = max_channels;

		AltosCompanion companion = new AltosCompanion(channels);

		companion.tick = tick();
		companion.board_id = uint8(5);
		companion.update_period = uint8(6);

		if (channels == 0)
			companion.companion_data = null;
		else {
			companion.companion_data = new int[channels];

			for (int i = 0; i < channels; i++)
				companion.companion_data[i] = uint16(8 + i * 2);
		}
		return companion;
	}

	static final public int max_channels = 12;

	public AltosTelemetryCompanion(int[] bytes) throws AltosCRCException {
		super(bytes);
	}

	public void provide_data(AltosDataListener listener) {
		super.provide_data(listener);
		listener.set_companion(companion());
	}
}
