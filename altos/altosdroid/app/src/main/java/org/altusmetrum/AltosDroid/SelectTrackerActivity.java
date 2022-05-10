/*
 * Copyright © 2020 Keith Packard <keithp@keithp.com>
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
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package org.altusmetrum.AltosDroid;

import java.util.*;
import android.app.Activity;
import android.content.*;
import android.os.*;
import android.util.*;
import android.view.*;
import android.view.View.*;
import android.widget.*;
import android.graphics.*;
import android.graphics.drawable.*;
import android.widget.CompoundButton.OnCheckedChangeListener;

import org.altusmetrum.altoslib_14.*;

class TrackerComparatorCall implements Comparator<Tracker> {
	public int compare(Tracker a, Tracker b) {
		int v;

		v = a.compareCall(b);
		if (v != 0)
			return v;
		v = a.compareAge(b);
		if (v != 0)
			return v;
		v = a.compareSerial(b);
		if (v != 0)
			return v;
		return a.compareFrequency(b);
	}
	public boolean equals(Object o) {
		return o instanceof TrackerComparatorCall;
	}
}

class TrackerComparatorSerial implements Comparator<Tracker> {
	public int compare(Tracker a, Tracker b) {
		int v;

		v = a.compareSerial(b);
		if (v != 0)
			return v;
		v = a.compareAge(b);
		if (v != 0)
			return v;
		v = a.compareCall(b);
		if (v != 0)
			return v;
		return a.compareFrequency(b);
	}
	public boolean equals(Object o) {
		return o instanceof TrackerComparatorSerial;
	}
}

class TrackerComparatorAge implements Comparator<Tracker> {
	public int compare(Tracker a, Tracker b) {
		int v;

		v = a.compareAge(b);
		if (v != 0)
			return v;
		v = a.compareCall(b);
		if (v != 0)
			return v;
		v = a.compareSerial(b);
		if (v != 0)
			return v;
		return a.compareFrequency(b);
	}
	public boolean equals(Object o) {
		return o instanceof TrackerComparatorAge;
	}
}

class TrackerComparatorFrequency implements Comparator<Tracker> {
	public int compare(Tracker a, Tracker b) {
		int v;

		v = a.compareFrequency(b);
		if (v != 0)
			return v;
		v = a.compareAge(b);
		if (v != 0)
			return v;
		v = a.compareCall(b);
		if (v != 0)
			return v;
		return a.compareSerial(b);
	}
	public boolean equals(Object o) {
		return o instanceof TrackerComparatorFrequency;
	}
}

public class SelectTrackerActivity extends Activity implements OnTouchListener {
	// Return Intent extra
	public static final String EXTRA_SERIAL_NUMBER = "serial_number";
	public static final String EXTRA_FREQUENCY = "frequency";

	private int button_ids[] = {
		R.id.call_button,
		R.id.serial_button,
		R.id.frequency_button,
		R.id.age_button
	};

	private static final int call_button = 0;
	private static final int serial_button = 1;
	private static final int freq_button = 2;
	private static final int age_button = 3;
	private RadioButton radio_buttons[] = new RadioButton[4];
	private TableLayout table;

	private Tracker[] trackers;

	private void set_sort(int id) {
		AltosDroidPreferences.set_tracker_sort(id);
		resort();
	}

	private void resort() {
		Comparator<Tracker> compare;
		int tracker_sort = AltosDroidPreferences.tracker_sort();
		AltosDebug.debug("sort %d", tracker_sort);
		switch (tracker_sort) {
		case call_button:
		default:
			compare = new TrackerComparatorCall();
			break;
		case serial_button:
			compare = new TrackerComparatorSerial();
			break;
		case freq_button:
			compare = new TrackerComparatorFrequency();
			break;
		case age_button:
			compare = new TrackerComparatorAge();
			break;
		}
		Arrays.sort(trackers, compare);
		set_trackers();
	}

	void init_button_state() {
		int tracker_sort = AltosDroidPreferences.tracker_sort();
		for (int i = 0; i < 4; i++)
			radio_buttons[i].setChecked(i == tracker_sort);
	}

	OnCheckedChangeListener button_listener = new OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				int id = buttonView.getId();
				if (isChecked) {
					int sort_id = -1;
					for (int i = 0; i < 4; i++) {
						if (id == button_ids[i])
							sort_id = i;
						else
							radio_buttons[i].setChecked(false);
					}
					if (sort_id != -1)
						set_sort(sort_id);
				}
			}
		};

	long start_time;

	private void
	insert_tracker(Tracker tracker) {
		TableRow row = (TableRow) getLayoutInflater().inflate(R.layout.tracker_ent, null);

		((TextView) row.findViewById(R.id.call_view)).setText(tracker.call);
		if (tracker.serial == 0)
			((TextView) row.findViewById(R.id.serial_view)).setText("");
		else
			((TextView) row.findViewById(R.id.serial_view)).setText(String.format("%d", tracker.serial));
		if (tracker.frequency == 0.0)
			((TextView) row.findViewById(R.id.frequency_view)).setText("");
		else if (tracker.frequency == AltosLib.MISSING)
			((TextView) row.findViewById(R.id.frequency_view)).setText("");
		else
			((TextView) row.findViewById(R.id.frequency_view)).setText(String.format("%7.3f", tracker.frequency));
		if (tracker.received_time != 0) {
			int age = (int) ((start_time - tracker.received_time + 500) / 1000);
			((TextView) row.findViewById(R.id.age_view)).setText(AltosDroid.age_string(age));
		} else {
			((TextView) row.findViewById(R.id.age_view)).setText("");
		}
		row.setClickable(true);
		row.setOnTouchListener(this);
		table.addView(row);
	}

	private void set_trackers() {
		for (int i = table.getChildCount() - 1; i >= 1; i--)
			table.removeViewAt(i);
		for (Tracker tracker : trackers)
			insert_tracker(tracker);
	}

	private void done(View v) {
		int result = Activity.RESULT_CANCELED;
		Intent intent = new Intent();
		for (int i = 1; i < table.getChildCount(); i++) {
			View child = table.getChildAt(i);
			if (child == v) {
				Tracker tracker = trackers[i - 1];
				intent.putExtra(EXTRA_SERIAL_NUMBER, tracker.serial);
				intent.putExtra(EXTRA_FREQUENCY, tracker.frequency);
				result = Activity.RESULT_OK;
				break;
			}
		}
		setResult(Activity.RESULT_OK, intent);
		finish();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		int title_id = getIntent().getIntExtra(AltosDroid.EXTRA_TRACKERS_TITLE, R.id.select_tracker);
		AltosDebug.debug("get title id 0x%x %s", title_id, getResources().getText(title_id));
		setTitle(getResources().getText(title_id));
		setTheme(AltosDroid.dialog_themes[AltosDroidPreferences.font_size()]);
		super.onCreate(savedInstanceState);

		setContentView(R.layout.tracker_list);
		// Set result CANCELED incase the user backs out
		setResult(Activity.RESULT_CANCELED);

		for (int i = 0; i < 4; i++) {
			radio_buttons[i] = (RadioButton) findViewById(button_ids[i]);
			radio_buttons[i].setOnCheckedChangeListener(button_listener);
		}

		ArrayList<Parcelable> tracker_array = (ArrayList<Parcelable>) getIntent().getParcelableArrayListExtra(AltosDroid.EXTRA_TRACKERS);
		if (tracker_array != null) {
			Object[] array = tracker_array.toArray();
			trackers = new Tracker[array.length];
			for (int i = 0; i < array.length; i++)
				trackers[i] = (Tracker) array[i];
		}

		start_time = System.currentTimeMillis();

		table = (TableLayout) findViewById(R.id.tracker_list);

		init_button_state();

		resort();

		set_trackers();
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		int action = event.getAction() & MotionEvent.ACTION_MASK;
		switch (action) {
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_CANCEL:
		case MotionEvent.ACTION_OUTSIDE:
			v.setBackgroundColor(0);
			v.invalidate();
			break;
		case MotionEvent.ACTION_DOWN:
			v.setBackgroundColor(Color.RED);
			v.invalidate();
			break;
		}
		if (action == MotionEvent.ACTION_UP) {
			done(v);
			return true;
		}
		return false;
	}
}
