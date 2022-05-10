/*
 * Copyright © 2010 Anthony Towns <aj@erisian.com.au>
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
import java.awt.image.*;
import javax.swing.*;
import java.io.*;
import java.lang.Math;
import java.awt.geom.*;
import java.util.*;
import java.util.concurrent.*;
import javax.imageio.*;
import org.altusmetrum.altoslib_14.*;

public class AltosUIMap extends JComponent implements AltosFlightDisplay, AltosMapInterface {

	AltosMap	map;
	Graphics2D	g;
	Font		tile_font;
	Font		line_font;
	AltosMapMark	nearest_mark;

	static Point2D.Double point2d(AltosPointDouble pt) {
		return new Point2D.Double(pt.x, pt.y);
	}

	static final AltosPointDouble point_double(Point pt) {
		return new AltosPointDouble(pt.x, pt.y);
	}

	class MapMark extends AltosMapMark {
		public void paint(AltosMapTransform t) {
			double lat = lat_lon.lat;
			double lon;
			double first_lon = t.first_lon(lat_lon.lon);
			double last_lon = t.last_lon(lat_lon.lon);
			for (lon = first_lon; lon <= last_lon; lon += 360.0) {
				AltosPointDouble pt = t.screen(lat, lon);

				g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
						   RenderingHints.VALUE_ANTIALIAS_ON);
				g.setStroke(new BasicStroke(stroke_width, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));

				if (0 <= state && state < AltosUIMap.stateColors.length)
					g.setColor(AltosUIMap.stateColors[state]);
				else
					g.setColor(AltosUIMap.stateColors[AltosLib.ao_flight_invalid]);

				g.drawOval((int)pt.x-5, (int)pt.y-5, 10, 10);
				g.drawOval((int)pt.x-20, (int)pt.y-20, 40, 40);
				g.drawOval((int)pt.x-35, (int)pt.y-35, 70, 70);

				if (label != null) {
					Rectangle2D	bounds;
					bounds = line_font.getStringBounds(label, g.getFontRenderContext());
					float x = (float) pt.x;
					float y = (float) pt.y + (float) bounds.getHeight() / 2.0f;

					g.setFont(line_font);
					g.setColor(Color.WHITE);
					for (int dy = -2; dy <= 2; dy += 2)
						for (int dx = -2; dx <= 2; dx += 2)
							g.drawString(label, x + dx, y + dy);
					if (0 <= state && state < AltosUIMap.stateColors.length)
						g.setColor(AltosUIMap.stateColors[state]);
					else
						g.setColor(AltosUIMap.stateColors[AltosLib.ao_flight_invalid]);
					g.drawString(label, x, y);
				}
			}
		}

		MapMark(double lat, double lon, int state, String label) {
			super(lat, lon, state, label);
		}

		MapMark(double lat, double lon, int state) {
			super(lat, lon, state);
		}
	}

	class MapView extends JComponent implements MouseMotionListener, MouseListener, ComponentListener, MouseWheelListener {

		private VolatileImage create_back_buffer() {
			return getGraphicsConfiguration().createCompatibleVolatileImage(getWidth(), getHeight());
		}

		private void do_paint(Graphics my_g) {
			g = (Graphics2D) my_g;

			map.paint();
		}

		public void paint(Graphics my_g) {
			VolatileImage	back_buffer = create_back_buffer();

			Graphics2D	top_g = (Graphics2D) my_g;

			do {
				GraphicsConfiguration gc = getGraphicsConfiguration();
				int code = back_buffer.validate(gc);
				if (code == VolatileImage.IMAGE_INCOMPATIBLE)
					back_buffer = create_back_buffer();

				Graphics g_back = back_buffer.getGraphics();
				g_back.setClip(top_g.getClip());
				do_paint(g_back);
				g_back.dispose();

				top_g.drawImage(back_buffer, 0, 0, this);
			} while (back_buffer.contentsLost());
			back_buffer.flush();
		}

		public void repaint(AltosRectangle damage) {
			repaint(damage.x, damage.y, damage.width, damage.height);
		}

		private boolean is_drag_event(MouseEvent e) {
			return e.getModifiersEx() == InputEvent.BUTTON1_DOWN_MASK;
		}

		/* MouseMotionListener methods */

		public void mouseDragged(MouseEvent e) {
			map.touch_continue(e.getPoint().x, e.getPoint().y, is_drag_event(e));
		}

		String pos(double p, String pos, String neg) {
			if (p == AltosLib.MISSING)
				return "";
			String	h = pos;
			if (p < 0) {
				h = neg;
				p = -p;
			}
			int deg = (int) Math.floor(p);
			double min = (p - Math.floor(p)) * 60.0;
			return String.format("%s %4d° %9.6f'", h, deg, min);
		}

		String height(double h, String label) {
			if (h == AltosLib.MISSING)
				return "";
			return String.format(" %s%s",
					     AltosConvert.height.show(6, h),
					     label);
		}

		String speed(double s, String label) {
			if (s == AltosLib.MISSING)
				return "";
			return String.format(" %s%s",
					     AltosConvert.speed.show(6, s),
					     label);
		}

		public void mouseMoved(MouseEvent e) {
			AltosMapPathPoint point = map.nearest(e.getPoint().x, e.getPoint().y);

			if (point != null) {
				if (nearest_mark == null)
					nearest_mark = map.add_mark(point.gps.lat,
								    point.gps.lon,
								    point.state);
				else {
					nearest_mark.lat_lon.lat = point.gps.lat;
					nearest_mark.lat_lon.lon = point.gps.lon;
					nearest_mark.state = point.state;
				}
				nearest_label.setText(String.format("%9.2f sec %s%s%s%s",
								    point.time,
								    pos(point.gps.lat,
									"N", "S"),
								    pos(point.gps.lon,
									"E", "W"),
								    height(point.gps_height, ""),
								    speed(point.gps.ground_speed, "(h)"),
								    speed(point.gps.climb_rate, "(v)")));
			} else {
				nearest_label.setText("");
			}
			repaint();
		}

		/* MouseListener methods */
		public void mouseClicked(MouseEvent e) {
		}

		public void mouseEntered(MouseEvent e) {
		}

		public void mouseExited(MouseEvent e) {
		}

		public void mousePressed(MouseEvent e) {
			map.touch_start(e.getPoint().x, e.getPoint().y, is_drag_event(e));
		}

		public void mouseReleased(MouseEvent e) {
		}

		/* MouseWheelListener methods */

		public void mouseWheelMoved(MouseWheelEvent e) {
			int	zoom_change = e.getWheelRotation();

			map.set_zoom_centre(map.get_zoom() - zoom_change, new AltosPointInt(e.getPoint().x, e.getPoint().y));
		}

		/* ComponentListener methods */

		public void componentHidden(ComponentEvent e) {
		}

		public void componentMoved(ComponentEvent e) {
		}

		public void componentResized(ComponentEvent e) {
			map.set_transform();
		}

		public void componentShown(ComponentEvent e) {
			map.set_transform();
		}

		MapView() {
			addComponentListener(this);
			addMouseMotionListener(this);
			addMouseListener(this);
			addMouseWheelListener(this);
		}
	}

	class MapLine extends AltosMapLine {

		public void paint(AltosMapTransform t) {

			if (start == null || end == null)
				return;

			g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);

			Line2D.Double line = new Line2D.Double(point2d(t.screen(start)),
							       point2d(t.screen(end)));

			g.setColor(Color.WHITE);
			g.setStroke(new BasicStroke(stroke_width+4, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
			g.draw(line);

			g.setColor(Color.BLUE);
			g.setStroke(new BasicStroke(stroke_width, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
			g.draw(line);

			String	message = line_dist();
			Rectangle2D	bounds;
			bounds = line_font.getStringBounds(message, g.getFontRenderContext());

			float x = (float) line.x1;
			float y = (float) line.y1 + (float) bounds.getHeight() / 2.0f;

			if (line.x1 < line.x2) {
				x -= (float) bounds.getWidth() + 2.0f;
			} else {
				x += 2.0f;
			}

			g.setFont(line_font);
			g.setColor(Color.WHITE);
			for (int dy = -2; dy <= 2; dy += 2)
				for (int dx = -2; dx <= 2; dx += 2)
					g.drawString(message, x + dx, y + dy);
			g.setColor(Color.BLUE);
			g.drawString(message, x, y);
		}

		public MapLine() {
		}
	}

	class MapPath extends AltosMapPath {
		public void paint(AltosMapTransform t) {
			Point2D.Double	prev = null;

			g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
					   RenderingHints.VALUE_ANTIALIAS_ON);
			g.setStroke(new BasicStroke(stroke_width, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));

			for (AltosMapPathPoint point : points) {
				Point2D.Double	cur = point2d(t.screen(point.gps.lat, point.gps.lon));
				if (prev != null) {
					Line2D.Double	line = new Line2D.Double (prev, cur);
					Rectangle	bounds = line.getBounds();

					if (g.hitClip(bounds.x, bounds.y, bounds.width, bounds.height)) {
						if (0 <= point.state && point.state < AltosUIMap.stateColors.length)
							g.setColor(AltosUIMap.stateColors[point.state]);
						else
							g.setColor(AltosUIMap.stateColors[AltosLib.ao_flight_invalid]);

						g.draw(line);
					}
				}
				prev = cur;
			}
		}
	}

	class MapTile extends AltosMapTile {
		public MapTile(AltosMapCache cache, AltosLatLon upper_left, AltosLatLon center, int zoom, int maptype, int px_size, int scale) {
			super(cache, upper_left, center, zoom, maptype, px_size, scale);
		}

		public void paint(AltosMapTransform t) {

			AltosPointDouble	point_double = t.screen(upper_left);
			Point			point = new Point((int) (point_double.x + 0.5),
								  (int) (point_double.y + 0.5));

			if (!g.hitClip(point.x, point.y, px_size, px_size))
				return;

			AltosImage	altos_image = get_image();
			AltosUIImage	ui_image = (AltosUIImage) altos_image;
			Image		image = null;

			if (ui_image != null)
				image = ui_image.image;

			if (image != null) {
				g.drawImage(image, point.x, point.y, null);
/*
 * Useful when debugging map fetching problems
 *
				String message = String.format("%.6f %.6f", center.lat, center.lon);
				g.setFont(tile_font);
				g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
				Rectangle2D bounds = tile_font.getStringBounds(message, g.getFontRenderContext());

				float x = px_size / 2.0f;
				float y = px_size / 2.0f;
				x = x - (float) bounds.getWidth() / 2.0f;
				y = y + (float) bounds.getHeight() / 2.0f;
				g.setColor(Color.RED);
				g.drawString(message, (float) point_double.x + x, (float) point_double.y + y);
*/
			} else {
				g.setColor(Color.GRAY);
				g.fillRect(point.x, point.y, px_size, px_size);

				if (t.has_location()) {
					String	message = null;
					switch (status) {
					case AltosMapTile.fetching:
						message = "Fetching...";
						break;
					case AltosMapTile.bad_request:
						message = "Internal error";
						break;
					case AltosMapTile.failed:
						message = "Network error";
						break;
					case AltosMapTile.forbidden:
						message = "Outside of known launch areas";
						break;
					}
					if (message != null && tile_font != null) {
						g.setFont(tile_font);
						g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
						Rectangle2D bounds = tile_font.getStringBounds(message, g.getFontRenderContext());

						float x = px_size / 2.0f;
						float y = px_size / 2.0f;
						x = x - (float) bounds.getWidth() / 2.0f;
						y = y + (float) bounds.getHeight() / 2.0f;
						g.setColor(Color.BLACK);
						g.drawString(message, (float) point_double.x + x, (float) point_double.y + y);
					}
				}
			}
		}
	}

	public static final Color stateColors[] = {
		Color.WHITE,  // startup
		Color.WHITE,  // idle
		Color.WHITE,  // pad
		Color.RED,    // boost
		Color.PINK,   // fast
		Color.YELLOW, // coast
		Color.CYAN,   // drogue
		Color.BLUE,   // main
		Color.BLACK,  // landed
		Color.BLACK,  // invalid
		Color.CYAN,   // stateless
	};

	/* AltosMapInterface functions */

	public AltosMapPath new_path() {
		return new MapPath();
	}

	public AltosMapLine new_line() {
		return new MapLine();
	}

	public AltosImage load_image(File file) throws Exception {
		return new AltosUIImage(ImageIO.read(file));
	}

	public AltosMapMark new_mark(double lat, double lon, int state) {
		return new MapMark(lat, lon, state);
	}

	public AltosMapMark new_mark(double lat, double lon, int state, String label) {
		return new MapMark(lat, lon, state, label);
	}

	public AltosMapTile new_tile(AltosMapCache cache, AltosLatLon upper_left, AltosLatLon center, int zoom, int maptype, int px_size, int scale) {
		return new MapTile(cache, upper_left, center, zoom, maptype, px_size, scale);
	}

	public int width() {
		return view.getWidth();
	}

	public int height() {
		return view.getHeight();
	}

	public void repaint() {
		view.repaint();
	}

	public void repaint(AltosRectangle damage) {
		view.repaint(damage);
	}

	public void set_zoom_label(String label) {
		zoom_label.setText(label);
	}

	public void select_object(AltosLatLon latlon) {
		debug("select at %f,%f\n", latlon.lat, latlon.lon);
	}

	public void debug(String format, Object ... arguments) {
		if (AltosUIPreferences.serial_debug())
			System.out.printf(format, arguments);
	}


	/* AltosFlightDisplay interface */

	public void set_font() {
		tile_font = AltosUILib.value_font;
		line_font = AltosUILib.status_font;
		if (nearest_label != null)
			nearest_label.setFont(AltosUILib.value_font);
	}

	public void font_size_changed(int font_size) {
		set_font();
		repaint();
	}

	public void units_changed(boolean imperial_units) {
		repaint();
	}

	JLabel	zoom_label;

	JLabel	nearest_label;

	public void set_maptype(int type) {
/*
		map.set_maptype(type);
		maptype_combo.setSelectedIndex(type);
*/
	}

	/* AltosUIMapPreload functions */

	public void set_zoom(int zoom) {
		map.set_zoom(zoom);
	}

	public void add_mark(double lat, double lon, int status) {
		map.add_mark(lat, lon, status);
	}

	public void add_mark(double lat, double lon, int status, String label) {
		map.add_mark(lat, lon, status, label);
	}

	public void clear_marks() {
		map.clear_marks();
	}

	/* AltosFlightDisplay interface */
	public void reset() {
		// nothing
	}

	public void show(AltosState state, AltosListenerState listener_state) {
		map.show(state, listener_state);
	}

	public void show(AltosGPS gps, double time, int state, double gps_height) {
		map.show(gps, time, state, gps_height);
	}

	public String getName() {
		return "Map";
	}

	/* AltosGraphUI interface */
	public void centre(AltosState state) {
		map.centre(state);
	}

	public void centre(AltosGPS gps) {
		map.centre(gps);
	}

	/* internal layout bits */
	private GridBagLayout layout = new GridBagLayout();

/*
	JComboBox<String>	maptype_combo;
*/

	MapView	view;

	public AltosUIMap() {

		set_font();

		view = new MapView();

		view.setPreferredSize(new Dimension(500,500));
		view.setVisible(true);
		view.setEnabled(true);

		GridBagLayout	my_layout = new GridBagLayout();

		setLayout(my_layout);

		GridBagConstraints c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.BOTH;
		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 1;
		c.gridheight = 10;
		c.weightx = 1;
		c.weighty = 1;
		add(view, c);

		int	y = 0;

		zoom_label = new JLabel("", JLabel.CENTER);

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_label, c);

		JButton zoom_reset = new JButton("0");
		zoom_reset.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					map.set_zoom(map.default_zoom);
				}
			});

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_reset, c);

		JButton zoom_in = new JButton("+");
		zoom_in.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					map.set_zoom(map.get_zoom() + 1);
				}
			});

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_in, c);

		JButton zoom_out = new JButton("-");
		zoom_out.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					map.set_zoom(map.get_zoom() - 1);
				}
			});
		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(zoom_out, c);


		nearest_label = new JLabel("", JLabel.LEFT);
		nearest_label.setFont(tile_font);

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 0;
		c.gridy = 11;
		c.weightx = 0;
		c.weighty = 0;
		c.gridwidth = 1;
		c.gridheight = 1;
		add(nearest_label, c);
/*
		maptype_combo = new JComboBox<String>(map.maptype_labels);

		maptype_combo.setEditable(false);
		maptype_combo.setMaximumRowCount(maptype_combo.getItemCount());
		maptype_combo.addItemListener(new ItemListener() {
				public void itemStateChanged(ItemEvent e) {
					map.set_maptype(maptype_combo.getSelectedIndex());
				}
			});

		c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = 1;
		c.gridy = y++;
		c.weightx = 0;
		c.weighty = 0;
		add(maptype_combo, c);
*/
		map = new AltosMap(this);
	}
}
