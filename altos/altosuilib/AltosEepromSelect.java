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

import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import org.altusmetrum.altoslib_14.*;

class AltosEepromItem implements ActionListener {
	AltosEepromLog	log;
	JLabel		label;
	JCheckBox	download;
	JCheckBox	delete;
	JCheckBox	graph;

	public void actionPerformed(ActionEvent e) {
		log.download_selected = download.isSelected();
		log.delete_selected = delete.isSelected();
		log.graph_selected = graph.isSelected();
	}

	public AltosEepromItem(AltosEepromLog in_log) {
		log = in_log;

		String	text;
		if (log.flight >= 0)
			text = String.format("Flight #%02d", log.flight);
		else
			text = String.format("Corrupt #%02d", -log.flight);

		label = new JLabel(text);

		download = new JCheckBox("", log.download_selected);
		download.addActionListener(this);

		delete = new JCheckBox("", log.delete_selected);
		delete.addActionListener(this);

		graph = new JCheckBox("", log.graph_selected);
		graph.addActionListener(this);
	}
}

public class AltosEepromSelect extends AltosUIDialog implements ActionListener {
	//private JList			list;
	private JFrame			frame;
	JButton				ok;
	JButton				cancel;
	boolean				success;

	/* Listen for events from our buttons */
	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("ok"))
			success = true;
		setVisible(false);
	}

	public boolean run() {
		success = false;
		setLocationRelativeTo(frame);
		setVisible(true);
		return success;
	}

	public AltosEepromSelect (JFrame in_frame,
				  AltosEepromList flights,
				  boolean has_graph) {

		super(in_frame, String.format("Flight list for serial %d", flights.config_data.serial), true);
		frame = in_frame;

		/* Create the container for the dialog */
		Container contentPane = getContentPane();

		/* First, we create a pane containing the dialog's header/title */
		JLabel	selectLabel = new JLabel(String.format ("Select flights"), SwingConstants.CENTER);

		JPanel	labelPane = new JPanel();
		labelPane.setLayout(new BoxLayout(labelPane, BoxLayout.X_AXIS));
		labelPane.setBorder(BorderFactory.createEmptyBorder(10, 0, 10, 0));
		labelPane.add(Box.createHorizontalGlue());
		labelPane.add(selectLabel);
		labelPane.add(Box.createHorizontalGlue());

		/* Add the header to the container. */
		contentPane.add(labelPane, BorderLayout.PAGE_START);


		/* Now we create the evilness that is a GridBag for the flight details */
		GridBagConstraints c;
		Insets i = new Insets(4,4,4,4);
		JPanel flightPane = new JPanel();
		flightPane.setLayout(new GridBagLayout());
		flightPane.setBorder(BorderFactory.createBevelBorder(BevelBorder.LOWERED));

		/* Flight Header */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 0;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0.5;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		JLabel flightHeaderLabel = new JLabel("Flight");
		flightPane.add(flightHeaderLabel, c);

		/* Download Header */
		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = 0;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0.5;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		JLabel downloadHeaderLabel = new JLabel("Download");
		flightPane.add(downloadHeaderLabel, c);

		/* Delete Header */
		c = new GridBagConstraints();
		c.gridx = 2; c.gridy = 0;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0.5;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = i;
		JLabel deleteHeaderLabel = new JLabel("Delete");
		flightPane.add(deleteHeaderLabel, c);

		if (has_graph) {
			/* Graph Header */
			c = new GridBagConstraints();
			c.gridx = 3; c.gridy = 0;
			c.fill = GridBagConstraints.NONE;
			c.weightx = 0.5;
			c.anchor = GridBagConstraints.CENTER;
			c.insets = i;
			JLabel graphHeaderLabel = new JLabel("Graph");
			flightPane.add(graphHeaderLabel, c);
		}

		/* Add the flights to the GridBag */
		AltosEepromItem item;
		int itemNumber = 1;
		for (AltosEepromLog flight : flights) {
			/* Create a flight object with handlers and
			 * appropriate UI items
			 */
			item = new AltosEepromItem(flight);

			/* Add a decriptive label for the flight */
			c = new GridBagConstraints();
			c.gridx = 0; c.gridy = itemNumber;
			c.fill = GridBagConstraints.NONE;
			c.weightx = 0.5;
			c.anchor = GridBagConstraints.CENTER;
			c.insets = i;
			flightPane.add(item.label, c);

			/* Add download checkbox for the flight */
			c = new GridBagConstraints();
			c.gridx = 1; c.gridy = itemNumber;
			c.fill = GridBagConstraints.NONE;
			c.weightx = 0.5;
			c.anchor = GridBagConstraints.CENTER;
			c.insets = i;
			flightPane.add(item.download, c);

			/* Add delete checkbox for the flight */
			c = new GridBagConstraints();
			c.gridx = 2; c.gridy = itemNumber;
			c.fill = GridBagConstraints.NONE;
			c.weightx = 0.5;
			c.anchor = GridBagConstraints.CENTER;
			c.insets = i;
			flightPane.add(item.delete, c);

			if (has_graph) {
				/* Add graph checkbox for the flight */
				c = new GridBagConstraints();
				c.gridx = 3; c.gridy = itemNumber;
				c.fill = GridBagConstraints.NONE;
				c.weightx = 0.5;
				c.anchor = GridBagConstraints.CENTER;
				c.insets = i;
				flightPane.add(item.graph, c);
			}

			itemNumber++;
		}

		/* Add the GridBag to the container */
		contentPane.add(flightPane, BorderLayout.CENTER);

		/* Create the dialog buttons */
		ok = new JButton("OK");
		ok.addActionListener(this);
		ok.setActionCommand("ok");

		cancel = new JButton("Cancel");
		cancel.addActionListener(this);
		cancel.setActionCommand("cancel");

		JPanel	buttonPane = new JPanel();
		buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.X_AXIS));
		buttonPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
		buttonPane.add(Box.createHorizontalGlue());
		buttonPane.add(cancel);
		buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
		buttonPane.add(ok);

		/* Add the buttons to the container */
		contentPane.add(buttonPane, BorderLayout.PAGE_END);

		/* Pack the window! */
		pack();
	}
}
