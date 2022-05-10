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
import java.util.*;

class AltosUIFrameListener extends WindowAdapter {
	@Override
	public void windowClosing (WindowEvent e) {
		AltosUIFrame frame = (AltosUIFrame) e.getWindow();
		AltosUIPreferences.unregister_ui_listener(frame);
		AltosUIFrame.frame_closed();
		frame.setVisible(false);
		frame.dispose();
	}
}

public class AltosUIFrame extends JFrame implements AltosUIListener, AltosPositionListener {

	public void ui_changed(String look_and_feel) {
		SwingUtilities.updateComponentTreeUI(this);
		this.pack();
	}

	private Container scrollPane;

	public Container getScrollablePane() {
		if (scrollPane == null) {
			Container content = super.getContentPane();
			/* Create a container to hold the dialog contents */
			scrollPane = new Container();

			/* Make an opaque box to use the right color */
			Box box = new Box(BoxLayout.X_AXIS);
			box.add(scrollPane);
			box.setOpaque(true);

			/* Create a scrollpane to hold the box */
			JScrollPane scroll = new JScrollPane();
			JViewport view = scroll.getViewport();
			view.add(box);

			/* Add the scroll pane to the top level */
			content.add(scroll);
		}
		return (Container) scrollPane;
	}

	static String[] altos_icon_names = {
		"/altusmetrum-altosui-16.png",
		"/altusmetrum-altosui-32.png",
		"/altusmetrum-altosui-48.png",
		"/altusmetrum-altosui-64.png",
		"/altusmetrum-altosui-128.png",
		"/altusmetrum-altosui-256.png"
	};

	static public String[] icon_names;

	static public void set_icon_names(String[] new_icon_names) { icon_names = new_icon_names; }

	public String[] icon_names() {
		if (icon_names == null)
			set_icon_names(altos_icon_names);
		return icon_names;
	}

	public void set_icon() {
		ArrayList<Image> icons = new ArrayList<Image>();
		String[] icon_names = icon_names();

		for (int i = 0; i < icon_names.length; i++) {
			java.net.URL imgURL = AltosUIFrame.class.getResource(icon_names[i]);
			if (imgURL != null)
				icons.add(new ImageIcon(imgURL).getImage());
		}
		setIconImages(icons);
	}

	private boolean location_by_platform = true;

	public void setLocationByPlatform(boolean lbp) {
		location_by_platform = lbp;
		super.setLocationByPlatform(lbp);
	}

	public void scan_device_selected(AltosDevice device) {
	}

	public void setSize() {
		/* Smash sizes around so that the window comes up in the right shape */
		Insets i = getInsets();
		Dimension ps = rootPane.getPreferredSize();
		ps.width += i.left + i.right;
		ps.height += i.top + i.bottom;
		setPreferredSize(ps);
		setSize(ps);
	}

	public void setPosition (int position) {
		Insets i = getInsets();
		Dimension ps = getSize();

		/* Stick the window in the desired location on the screen */
		setLocationByPlatform(false);
		GraphicsDevice gd = GraphicsEnvironment.getLocalGraphicsEnvironment().getDefaultScreenDevice();
		GraphicsConfiguration gc = gd.getDefaultConfiguration();
		Rectangle r = gc.getBounds();

		/* compute X position */
		int x = 0;
		int y = 0;
		switch (position) {
		case AltosUILib.position_top_left:
		case AltosUILib.position_left:
		case AltosUILib.position_bottom_left:
			x = 0;
			break;
		case AltosUILib.position_top:
		case AltosUILib.position_center:
		case AltosUILib.position_bottom:
			x = (r.width - ps.width) / 2;
			break;
		case AltosUILib.position_top_right:
		case AltosUILib.position_right:
		case AltosUILib.position_bottom_right:
			x = r.width - ps.width + i.right;
			break;
		}

		/* compute Y position */
		switch (position) {
		case AltosUILib.position_top_left:
		case AltosUILib.position_top:
		case AltosUILib.position_top_right:
			y = 0;
			break;
		case AltosUILib.position_left:
		case AltosUILib.position_center:
		case AltosUILib.position_right:
			y = (r.height - ps.height) / 2;
			break;
		case AltosUILib.position_bottom_left:
		case AltosUILib.position_bottom:
		case AltosUILib.position_bottom_right:
			y = r.height - ps.height + i.bottom;
			break;
		}
		setLocation(x, y);
	}

	int position;

	public void position_changed(int position) {
		this.position = position;
		if (!location_by_platform)
			setPosition(position);
	}

	public void setVisible (boolean visible) {
		if (visible)
			setLocationByPlatform(location_by_platform);
		super.setVisible(visible);
		if (visible) {
			setSize();
			if (!location_by_platform)
				setPosition(position);
		}
	}

	static boolean global_settings_done;

	public String getName() {
		return "Altus Metrum";
	}

	public void macosx_quit_handler() {
		System.out.printf("Got quit handler\n");
	}

	public void macosx_about_handler() {
		System.out.printf("Got about handler\n");
	}

	public void macosx_preferences_handler() {
		System.out.printf("Got preferences handler\n");
	}

	public void macosx_file_handler(String path) {
		System.out.printf("Got file handler with \"%s\"\n", path);
	}

	/* Check that we are on Mac OS X.  This is crucial to loading and using the OSXAdapter class.
	 */
	public static boolean MAC_OS_X = (System.getProperty("os.name").toLowerCase().startsWith("mac os x"));

	private static boolean registered_for_macosx_events;

	/* Generic registration with the Mac OS X application menu
	 * Checks the platform, then attempts to register with the Apple EAWT
	 * See OSXAdapter.java to see how this is done without directly referencing any Apple APIs
	 */
	public synchronized void register_for_macosx_events() {
		if (registered_for_macosx_events)
			return;
		registered_for_macosx_events = true;
		if (MAC_OS_X) {
			try {
				// Generate and register the OSXAdapter, passing it a hash of all the methods we wish to
				// use as delegates for various com.apple.eawt.ApplicationListener methods
				OSXAdapter.setQuitHandler(this, getClass().getDeclaredMethod("macosx_quit_handler", (Class[])null));
//				OSXAdapter.setAboutHandler(this, getClass().getDeclaredMethod("macosx_about_handler", (Class[])null));
				OSXAdapter.setPreferencesHandler(this, getClass().getDeclaredMethod("macosx_preferences_handler", (Class[])null));
				OSXAdapter.setFileHandler(this, getClass().getDeclaredMethod("macosx_file_handler", new Class[] { String.class }));
			} catch (Exception e) {
				System.err.println("Error while loading the OSXAdapter:");
				e.printStackTrace();
			}
		}
	}

	int row = 0;

	public void next_row() {
		row++;
	}

	int inset = 0;

	public void set_inset(int i) {
		inset = i;
	}

	public GridBagConstraints constraints (int x, int width, int fill, int anchor, double weightx, double weighty) {
		return new GridBagConstraints(x,			/* x */
					      row,			/* y */
					      width,			/* width */
					      1,			/* height */
					      weightx,			/* weightx */
					      weighty,			/* weighty */
					      anchor,			/* anchor */
					      fill,			/* fill */
					      new Insets(inset,inset,inset,inset),	/* insets */
					      0,			/* ipadx */
					      0);			/* ipady */
	}

	public GridBagConstraints constraints (int x, int width, int fill, int anchor) {
		double	weightx = 0;
		double	weighty = 0;

		if (fill == GridBagConstraints.NONE) {
			weightx = 0;
			weighty = 0;
		} else if (fill == GridBagConstraints.HORIZONTAL) {
			weightx = 1;
			weighty = 0;
		} else if (fill == GridBagConstraints.VERTICAL) {
			weightx = 0;
			weighty = 1;
		} else if (fill == GridBagConstraints.BOTH) {
			weightx = 1;
			weighty = 1;
		}

		return constraints (x, width, fill, anchor, weightx, weighty);
	}

	public GridBagConstraints constraints (int x, int width, int fill) {
		return constraints (x, width, fill, GridBagConstraints.WEST);
	}

	public GridBagConstraints constraints(int x, int width) {
		return constraints(x, width, GridBagConstraints.NONE);
	}

	static int open_frames;

	public static void frame_opened() {
		++open_frames;
	}

	public static void frame_closed() {
		--open_frames;
		if (open_frames == 0)
			System.exit(0);
	}

	void init() {
		AltosUIPreferences.register_ui_listener(this);
		AltosUIPreferences.register_position_listener(this);
		position = AltosUIPreferences.position();
		frame_opened();
		addWindowListener(new AltosUIFrameListener());

		/* Try to make menus live in the menu bar like regular Mac apps */
		if (!global_settings_done) {
			try {
				global_settings_done = true;
				System.setProperty("com.apple.mrj.application.apple.menu.about.name", getName());
				System.setProperty("com.apple.macos.useScreenMenuBar", "true");
				System.setProperty("apple.laf.useScreenMenuBar", "true" ); // for older versions of Java
			} catch (Exception e) {
			}
		}
		set_icon();
	}

	public AltosUIFrame() {
		init();
	}

	public AltosUIFrame(String name) {
		super(name);
		init();
	}
}
