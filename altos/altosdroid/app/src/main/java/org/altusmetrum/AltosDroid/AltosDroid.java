/*
 * Copyright © 2012-2013 Mike Beattie <mike@ethernal.org>
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

package org.altusmetrum.AltosDroid;

import java.lang.ref.WeakReference;
import java.util.*;
import java.io.*;

import android.Manifest;
import android.app.Activity;
import android.app.PendingIntent;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.Context;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.content.DialogInterface;
import android.os.IBinder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.os.Parcelable;
import androidx.fragment.app.FragmentActivity;
import androidx.fragment.app.FragmentManager;
import android.view.*;
import android.widget.*;
import android.app.AlertDialog;
import android.location.Location;
import android.location.LocationManager;
import android.location.LocationListener;
import android.hardware.usb.*;
import android.content.pm.PackageManager;
import androidx.core.app.ActivityCompat;
import org.altusmetrum.altoslib_14.*;

class SavedState {
	long	received_time;
	int	state;
	boolean	locked;
	String	callsign;
	int	serial;
	int	flight;
	int	rssi;

	SavedState(AltosState state) {
		received_time = state.received_time;
		this.state = state.state();
		if (state.gps != null)
			locked = state.gps.locked;
		else
			locked = false;
		callsign = state.cal_data().callsign;
		serial = state.cal_data().serial;
		flight = state.cal_data().flight;
		rssi = state.rssi;
	}
}

public class AltosDroid extends FragmentActivity implements AltosUnitsListener, LocationListener, ActivityCompat.OnRequestPermissionsResultCallback {

	// Actions sent to the telemetry server at startup time

	public static final String ACTION_BLUETOOTH = "org.altusmetrum.AltosDroid.BLUETOOTH";
	public static final String ACTION_USB = "org.altusmetrum.AltosDroid.USB";

	// Message types received by our Handler

	public static final int MSG_STATE           = 1;
	public static final int MSG_UPDATE_AGE      = 2;
	public static final int	MSG_IDLE_MODE	    = 3;
	public static final int MSG_IGNITER_STATUS  = 4;
	public static final int MSG_FILE_FAILED     = 5;

	// Intent request codes
	public static final int REQUEST_CONNECT_DEVICE = 1;
	public static final int REQUEST_ENABLE_BT      = 2;
	public static final int REQUEST_PRELOAD_MAPS   = 3;
	public static final int REQUEST_IDLE_MODE      = 5;
	public static final int REQUEST_IGNITERS       = 6;
	public static final int REQUEST_SETUP	       = 7;
	public static final int REQUEST_SELECT_TRACKER = 8;
	public static final int REQUEST_DELETE_TRACKER = 9;

	public static final String EXTRA_IDLE_MODE = "idle_mode";
	public static final String EXTRA_IDLE_RESULT = "idle_result";
	public static final String EXTRA_FREQUENCY = "frequency";
	public static final String EXTRA_TELEMETRY_SERVICE = "telemetry_service";
	public static final String EXTRA_TRACKERS = "trackers";
	public static final String EXTRA_TRACKERS_TITLE = "trackers_title";

	// Setup result bits
	public static final int SETUP_BAUD = 1;
	public static final int SETUP_UNITS = 2;
	public static final int SETUP_MAP_SOURCE = 4;
	public static final int SETUP_MAP_TYPE = 8;
	public static final int SETUP_FONT_SIZE = 16;

	public static FragmentManager	fm;

	private BluetoothAdapter mBluetoothAdapter = null;

	// Flight state values
	private TextView mCallsignView;
	private TextView mRSSIView;
	private TextView mSerialView;
	private TextView mFlightView;
	private RelativeLayout mStateLayout;
	private TextView mStateView;
	private TextView mAgeView;
	private boolean  mAgeViewOld;
	private int mAgeNewColor;
	private int mAgeOldColor;

	public static final String	tab_pad_name = "pad";
	public static final String	tab_flight_name = "flight";
	public static final String	tab_recover_name = "recover";
	public static final String	tab_map_name = "map";

	// field to display the version at the bottom of the screen
	private TextView mVersion;

	private boolean idle_mode = false;

	public Location location = null;

	private AltosState	state;
	private SavedState	saved_state;

	// Tabs
	TabHost         mTabHost;
	AltosViewPager  mViewPager;
	TabsAdapter     mTabsAdapter;
	ArrayList<AltosDroidTab> mTabs = new ArrayList<AltosDroidTab>();
	int             tabHeight;

	// Timer and Saved flight state for Age calculation
	private Timer timer;

	TelemetryState	telemetry_state;
	Tracker[]	trackers;

	UsbDevice	pending_usb_device;
	boolean		start_with_usb;

	// Service
	private boolean mIsBound   = false;
	private Messenger mService = null;
	final Messenger mMessenger = new Messenger(new IncomingHandler(this));

	// Text to Speech
	private AltosVoice mAltosVoice = null;

	// The Handler that gets information back from the Telemetry Service
	static class IncomingHandler extends Handler {
		private final WeakReference<AltosDroid> mAltosDroid;
		IncomingHandler(AltosDroid ad) { mAltosDroid = new WeakReference<AltosDroid>(ad); }

		@Override
		public void handleMessage(Message msg) {
			AltosDroid ad = mAltosDroid.get();

			switch (msg.what) {
			case MSG_STATE:
				if (msg.obj == null) {
					AltosDebug.debug("telemetry_state null!");
					return;
				}
				ad.update_state((TelemetryState) msg.obj);
				break;
			case MSG_UPDATE_AGE:
				ad.update_age();
				break;
			case MSG_IDLE_MODE:
				ad.idle_mode = (Boolean) msg.obj;
				ad.update_state(null);
				break;
			case MSG_FILE_FAILED:
				ad.file_failed((File) msg.obj);
				break;
			}
		}
	};

	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			AltosDebug.debug("onServiceConnected\n");
			mService = new Messenger(service);
			try {
				Message msg = Message.obtain(null, TelemetryService.MSG_REGISTER_CLIENT);
				msg.replyTo = mMessenger;
				mService.send(msg);
			} catch (RemoteException e) {
				AltosDebug.debug("attempt to register telemetry service client failed\n");
				// In this case the service has crashed before we could even do anything with it
			}
			if (pending_usb_device != null) {
				try {
					mService.send(Message.obtain(null, TelemetryService.MSG_OPEN_USB, pending_usb_device));
					pending_usb_device = null;
				} catch (RemoteException e) {
				}
			}
		}

		public void onServiceDisconnected(ComponentName className) {
			AltosDebug.debug("onServiceDisconnected\n");
			// This is called when the connection with the service has been unexpectedly disconnected - process crashed.
			mService = null;
		}
	};

	void doBindService() {
		AltosDebug.debug("doBindService\n");
		bindService(new Intent(this, TelemetryService.class), mConnection, Context.BIND_AUTO_CREATE);
		mIsBound = true;
	}

	void doUnbindService() {
		AltosDebug.debug("doUnbindService\n");
		if (mIsBound) {
			// If we have received the service, and hence registered with it, then now is the time to unregister.
			if (mService != null) {
				try {
					Message msg = Message.obtain(null, TelemetryService.MSG_UNREGISTER_CLIENT);
					msg.replyTo = mMessenger;
					mService.send(msg);
				} catch (RemoteException e) {
					// There is nothing special we need to do if the service has crashed.
				}
			}
			// Detach our existing connection.
			unbindService(mConnection);
			mIsBound = false;
		}
	}

	public AltosDroidTab findTab(String name) {
		for (AltosDroidTab mTab : mTabs)
			if (name.equals(mTab.tab_name()))
				return mTab;
		return null;
	}

	public void registerTab(AltosDroidTab mTab) {
		mTabs.add(mTab);
	}

	public void unregisterTab(AltosDroidTab mTab) {
		mTabs.remove(mTab);
	}

	public void units_changed(boolean imperial_units) {
		for (AltosDroidTab mTab : mTabs)
			mTab.units_changed(imperial_units);
	}

	void update_title(TelemetryState telemetry_state) {
		switch (telemetry_state.connect) {
		case TelemetryState.CONNECT_CONNECTED:
			if (telemetry_state.config != null) {
				String str = String.format("S/N %d %6.3f MHz%s", telemetry_state.config.serial,
							   telemetry_state.frequency, telemetry_state.idle_mode ? " (idle)" : "");
				if (telemetry_state.telemetry_rate != AltosLib.ao_telemetry_rate_38400)
					str = str.concat(String.format(" %d bps",
								       AltosLib.ao_telemetry_rate_values[telemetry_state.telemetry_rate]));
				setTitle(str);
			} else {
				setTitle(R.string.title_connected_to);
			}
			break;
		case TelemetryState.CONNECT_CONNECTING:
			if (telemetry_state.address != null)
				setTitle(String.format("Connecting to %s...", telemetry_state.address.name));
			else
				setTitle("Connecting to something...");
			break;
		case TelemetryState.CONNECT_DISCONNECTED:
		case TelemetryState.CONNECT_NONE:
			setTitle(R.string.title_not_connected);
			break;
		}
	}

	void start_timer() {
		if (timer == null) {
			timer = new Timer();
			timer.scheduleAtFixedRate(new TimerTask(){ public void run() {onTimerTick();}}, 1000L, 1000L);
		}
	}

	void stop_timer() {
		if (timer != null) {
			timer.cancel();
			timer.purge();
			timer = null;
		}
	}

	int	selected_serial = 0;
	long	switch_time;

	void set_switch_time() {
		switch_time = System.currentTimeMillis();
		selected_serial = 0;
	}

	boolean	registered_units_listener;

	void update_state(TelemetryState new_telemetry_state) {

		if (new_telemetry_state != null)
			telemetry_state = new_telemetry_state;

		if (selected_frequency != AltosLib.MISSING) {
			AltosState selected_state = telemetry_state.get(selected_serial);
			AltosState latest_state = telemetry_state.get(telemetry_state.latest_serial);

			if (selected_state != null && selected_state.frequency == selected_frequency) {
				selected_frequency = AltosLib.MISSING;
			} else if ((selected_state == null || selected_state.frequency != selected_frequency) &&
				   (latest_state != null && latest_state.frequency == selected_frequency))
			{
				selected_frequency = AltosLib.MISSING;
				selected_serial = telemetry_state.latest_serial;
			}
		}

		if (!telemetry_state.containsKey(selected_serial)) {
			selected_serial = telemetry_state.latest_serial;
			AltosDebug.debug("selected serial set to %d", selected_serial);
		}

		int shown_serial = selected_serial;

		if (telemetry_state.idle_mode)
			shown_serial = telemetry_state.latest_serial;

		if (!registered_units_listener) {
			registered_units_listener = true;
			AltosPreferences.register_units_listener(this);
		}

		int	num_trackers = 0;

		for (AltosState s : telemetry_state.values()) {
			num_trackers++;
		}

		trackers = new Tracker[num_trackers + 1];

		int n = 0;
		trackers[n++] = new Tracker(0, "auto", 0.0);

		for (AltosState s : telemetry_state.values())
			trackers[n++] = new Tracker(s);

		Arrays.sort(trackers);

		if (telemetry_state.frequency != AltosLib.MISSING)
			telem_frequency = telemetry_state.frequency;

		update_title(telemetry_state);

		AltosState	state = telemetry_state.get(shown_serial);

		update_ui(telemetry_state, state, telemetry_state.quiet);

		start_timer();
	}

	boolean same_string(String a, String b) {
		if (a == null) {
			if (b == null)
				return true;
			return false;
		} else {
			if (b == null)
				return false;
			return a.equals(b);
		}
	}


	private int blend_component(int a, int b, double r, int shift, int mask) {
		return ((int) (((a >> shift) & mask) * r + ((b >> shift) & mask) * (1 - r)) & mask) << shift;
	}
	private int blend_color(int a, int b, double r) {
		return (blend_component(a, b, r, 0, 0xff) |
			blend_component(a, b, r, 8, 0xff) |
			blend_component(a, b, r, 16, 0xff) |
			blend_component(a, b, r, 24, 0xff));
	}

	int state_age(long received_time) {
		return (int) ((System.currentTimeMillis() - received_time + 500) / 1000);
	}

	void set_screen_on(int age) {
		if (age < 60)
			getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		else
			getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	}

	static String age_string(int age) {
		String	text;
		if (age < 60)
			text = String.format("%ds", age);
		else if (age < 60 * 60)
			text = String.format("%dm", age / 60);
		else if (age < 60 * 60 * 24)
			text = String.format("%dh", age / (60 * 60));
		else
			text = String.format("%dd", age / (24 * 60 * 60));
		return text;
	}

	void update_age() {
		if (saved_state != null) {
			int age = state_age(saved_state.received_time);

			double age_scale = age / 100.0;

			if (age_scale > 1.0)
				age_scale = 1.0;

			mAgeView.setTextColor(blend_color(mAgeOldColor, mAgeNewColor, age_scale));

			set_screen_on(age);

			mAgeView.setText(age_string(age));
		}
	}

	void update_ui(TelemetryState telem_state, AltosState state, boolean quiet) {

		AltosDebug.debug("update_ui telem %b state %b quiet %b saved_state %b\n",
				 telem_state != null,
				 state != null,
				 quiet,
				 saved_state != null);

		this.state = state;

		int prev_state = AltosLib.ao_flight_invalid;

		AltosGreatCircle from_receiver = null;

		if (saved_state != null)
			prev_state = saved_state.state;

		if (state != null) {
			set_screen_on(state_age(state.received_time));

			if (state.state() == AltosLib.ao_flight_stateless) {
				boolean	prev_locked = false;
				boolean locked = false;

				if(state.gps != null)
					locked = state.gps.locked;
				if (saved_state != null)
					prev_locked = saved_state.locked;
				if (prev_locked != locked) {
					String currentTab = mTabHost.getCurrentTabTag();
					if (locked) {
						if (currentTab.equals(tab_pad_name)) mTabHost.setCurrentTabByTag(tab_flight_name);
					} else {
						if (currentTab.equals(tab_flight_name)) mTabHost.setCurrentTabByTag(tab_pad_name);
					}
				}
			} else {
				if (prev_state != state.state()) {
					String currentTab = mTabHost.getCurrentTabTag();
					switch (state.state()) {
					case AltosLib.ao_flight_boost:
						if (currentTab.equals(tab_pad_name)) mTabHost.setCurrentTabByTag(tab_flight_name);
						break;
					case AltosLib.ao_flight_landed:
						if (currentTab.equals(tab_flight_name)) mTabHost.setCurrentTabByTag(tab_recover_name);
						break;
					case AltosLib.ao_flight_stateless:
						if (currentTab.equals(tab_pad_name)) mTabHost.setCurrentTabByTag(tab_flight_name);
						break;
					}
				}
			}

			if (location != null && state.gps != null && state.gps.locked) {
				double altitude = 0;
				if (location.hasAltitude())
					altitude = location.getAltitude();
				from_receiver = new AltosGreatCircle(location.getLatitude(),
								     location.getLongitude(),
								     altitude,
								     state.gps.lat,
								     state.gps.lon,
								     state.gps.alt);
			}

			if (saved_state == null || !same_string(saved_state.callsign, state.cal_data().callsign)) {
				mCallsignView.setText(state.cal_data().callsign);
			}
			if (saved_state == null || state.cal_data().serial != saved_state.serial) {
				if (state.cal_data().serial == AltosLib.MISSING)
					mSerialView.setText("");
				else
					mSerialView.setText(String.format("%d", state.cal_data().serial));
			}
			if (saved_state == null || state.cal_data().flight != saved_state.flight) {
				if (state.cal_data().flight == AltosLib.MISSING)
					mFlightView.setText("");
				else
					mFlightView.setText(String.format("%d", state.cal_data().flight));
			}
			if (saved_state == null || state.state() != saved_state.state) {
				if (state.state() == AltosLib.ao_flight_stateless) {
					mStateLayout.setVisibility(View.GONE);
				} else {
					mStateView.setText(state.state_name());
					mStateLayout.setVisibility(View.VISIBLE);
				}
			}
			if (saved_state == null || state.rssi != saved_state.rssi) {
				if (state.rssi == AltosLib.MISSING)
					mRSSIView.setText("");
				else
					mRSSIView.setText(String.format("%d", state.rssi));
			}
			saved_state = new SavedState(state);
		}

		for (AltosDroidTab mTab : mTabs) {
			AltosDebug.debug("mTab %s current %s\n",
					 mTab, mTabsAdapter.currentItem());
			mTab.update_ui(telem_state, state, from_receiver, location, mTab == mTabsAdapter.currentItem());
		}

		if (mAltosVoice != null && mTabsAdapter.currentItem() != null)
			mAltosVoice.tell(telem_state, state, from_receiver, location, (AltosDroidTab) mTabsAdapter.currentItem(), quiet);

	}

	private void onTimerTick() {
		try {
			mMessenger.send(Message.obtain(null, MSG_UPDATE_AGE));
		} catch (RemoteException e) {
		}
	}

	static String pos(double p, String pos, String neg) {
		String	h = pos;
		if (p == AltosLib.MISSING)
			return "";
		if (p < 0) {
			h = neg;
			p = -p;
		}
		int deg = (int) Math.floor(p);
		double min = (p - Math.floor(p)) * 60.0;
		return String.format("%d° %7.4f\" %s", deg, min, h);
	}

	static String number(String format, double value) {
		if (value == AltosLib.MISSING)
			return "";
		return String.format(format, value);
	}

	static String integer(String format, int value) {
		if (value == AltosLib.MISSING)
			return "";
		return String.format(format, value);
	}

	private View create_tab_view(String label) {
		LayoutInflater inflater = (LayoutInflater) this.getLayoutInflater();
		View tab_view = inflater.inflate(R.layout.tab_layout, null);
		TextView text_view = (TextView) tab_view.findViewById (R.id.tabLabel);
		text_view.setText(label);
		return tab_view;
	}

	static public int[] themes = {
		R.style.Small,
		R.style.Medium,
		R.style.Large,
		R.style.Extra
	};

	static public int[] dialog_themes = {
		R.style.Small_Dialog,
		R.style.Medium_Dialog,
		R.style.Large_Dialog,
		R.style.Extra_Dialog
	};

	@Override
	public void onCreate(Bundle savedInstanceState) {
		AltosDebug.init(this);
		AltosDebug.debug("+++ ON CREATE +++");

		// Initialise preferences
		AltosDroidPreferences.init(this);
		setTheme(themes[AltosDroidPreferences.font_size()]);
		super.onCreate(savedInstanceState);

		fm = getSupportFragmentManager();

		// Set up the window layout
		setContentView(R.layout.altosdroid);

		// Create the Tabs and ViewPager
		mTabHost = (TabHost)findViewById(android.R.id.tabhost);
		mTabHost.setup();

		mViewPager = (AltosViewPager)findViewById(R.id.pager);
		mViewPager.setOffscreenPageLimit(4);

		mTabsAdapter = new TabsAdapter(this, mTabHost, mViewPager);

		mTabsAdapter.addTab(mTabHost.newTabSpec(tab_pad_name).setIndicator(create_tab_view("Pad")), TabPad.class, null, findTab(tab_pad_name));
		mTabsAdapter.addTab(mTabHost.newTabSpec(tab_flight_name).setIndicator(create_tab_view("Flight")), TabFlight.class, null, findTab(tab_flight_name));
		mTabsAdapter.addTab(mTabHost.newTabSpec(tab_recover_name).setIndicator(create_tab_view("Recover")), TabRecover.class, null, findTab(tab_recover_name));
		mTabsAdapter.addTab(mTabHost.newTabSpec(tab_map_name).setIndicator(create_tab_view("Map")), TabMap.class, null, findTab(tab_map_name));

		// Display the Version
		mVersion = (TextView) findViewById(R.id.version);
		mVersion.setText("Version: " + BuildInfo.version +
		                 " Built: " + BuildInfo.builddate + " " + BuildInfo.buildtime + " " + BuildInfo.buildtz +
		                 " (" + BuildInfo.branch + "-" + BuildInfo.commitnum + "-" + BuildInfo.commithash + ")");

		mCallsignView  = (TextView) findViewById(R.id.callsign_value);
		mRSSIView      = (TextView) findViewById(R.id.rssi_value);
		mSerialView    = (TextView) findViewById(R.id.serial_value);
		mFlightView    = (TextView) findViewById(R.id.flight_value);
		mStateLayout   = (RelativeLayout) findViewById(R.id.state_container);
		mStateView     = (TextView) findViewById(R.id.state_value);
		mAgeView       = (TextView) findViewById(R.id.age_value);
		mAgeNewColor   = mAgeView.getTextColors().getDefaultColor();
		mAgeOldColor   = getResources().getColor(R.color.old_color);
	}

	private void ensureBluetooth() {
		// Get local Bluetooth adapter
		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

		/* if there is a BT adapter and it isn't turned on, then turn it on */
		if (mBluetoothAdapter != null && !mBluetoothAdapter.isEnabled()) {
			Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableIntent, AltosDroid.REQUEST_ENABLE_BT);
		}
	}

	private boolean check_usb() {
		UsbDevice	device = AltosUsb.find_device(this, AltosLib.product_basestation);

		if (device != null) {
			Intent		i = new Intent(this, AltosDroid.class);
			PendingIntent	pi = PendingIntent.getActivity(this, 0, new Intent("hello world", null, this, AltosDroid.class), 0);

			if (AltosUsb.request_permission(this, device, pi)) {
				connectUsb(device);
			}
			start_with_usb = true;
			return true;
		}

		start_with_usb = false;

		return false;
	}

	private void noticeIntent(Intent intent) {

		/* Ok, this is pretty convenient.
		 *
		 * When a USB device is plugged in, and our 'hotplug'
		 * intent registration fires, we get an Intent with
		 * EXTRA_DEVICE set.
		 *
		 * When we start up and see a usb device and request
		 * permission to access it, that queues a
		 * PendingIntent, which has the EXTRA_DEVICE added in,
		 * along with the EXTRA_PERMISSION_GRANTED field as
		 * well.
		 *
		 * So, in both cases, we get the device name using the
		 * same call. We check to see if access was granted,
		 * in which case we ignore the device field and do our
		 * usual startup thing.
		 */

		UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
		boolean granted = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, true);

		AltosDebug.debug("intent %s device %s granted %s", intent, device, granted);

		if (!granted)
			device = null;

		if (device != null) {
			AltosDebug.debug("intent has usb device " + device.toString());
			connectUsb(device);
		} else {

			/* 'granted' is only false if this intent came
			 * from the request_permission call and
			 * permission was denied. In which case, we
			 * don't want to loop forever...
			 */
			if (granted) {
				AltosDebug.debug("check for a USB device at startup");
				if (check_usb())
					return;
			}
			AltosDebug.debug("Starting by looking for bluetooth devices");
			ensureBluetooth();
		}
	}

	@Override
	public void onStart() {
		super.onStart();
		AltosDebug.debug("++ ON START ++");

		set_switch_time();

		noticeIntent(getIntent());

		// Start Telemetry Service
		String	action = start_with_usb ? ACTION_USB : ACTION_BLUETOOTH;

		startService(new Intent(action, null, AltosDroid.this, TelemetryService.class));

		doBindService();

		if (mAltosVoice == null)
			mAltosVoice = new AltosVoice(this);

	}

	@Override
	public void onNewIntent(Intent intent) {
		super.onNewIntent(intent);
		AltosDebug.debug("+ ON NEW INTENT +");
		noticeIntent(intent);
	}

	private void enable_location_updates(boolean do_update) {
		// Listen for GPS and Network position updates
		LocationManager locationManager = (LocationManager) this.getSystemService(Context.LOCATION_SERVICE);

		if (locationManager != null)
		{
			try {
				locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1000, 1, this);
				location = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
			} catch (Exception e) {
				locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 1000, 1, this);
				location = locationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
			}

			if (location != null)
				AltosDebug.debug("Resume, location is %f,%f\n",
						 location.getLatitude(),
						 location.getLongitude());
			AltosDebug.debug("Failed to get GPS updates\n");
		}

		if (do_update)
			update_ui(telemetry_state, state, true);
	}

	static final int MY_PERMISSION_REQUEST = 1001;

	public boolean have_location_permission = false;
	public boolean have_storage_permission = false;
	public boolean have_bluetooth_permission = false;
	public boolean have_bluetooth_connect_permission = false;
	public boolean asked_permission = false;

	static final String BLUETOOTH_CONNECT = "android.permission.BLUETOOTH_CONNECT";

	AltosMapOnline map_online;

	void
	tell_map_permission(AltosMapOnline map_online) {
		this.map_online = map_online;
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions,
					       int[] grantResults) {
		if (requestCode == MY_PERMISSION_REQUEST) {
			for (int i = 0; i < grantResults.length; i++) {
				if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
					if (permissions[i].equals(Manifest.permission.ACCESS_FINE_LOCATION)) {
						have_location_permission = true;
						enable_location_updates(true);
						if (map_online != null)
							map_online.position_permission();
					}
					if (permissions[i].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
						have_storage_permission = true;
					}
					if (permissions[i].equals(Manifest.permission.BLUETOOTH)) {
						have_bluetooth_permission = true;
					}
					if (permissions[i].equals(BLUETOOTH_CONNECT)) {
						have_bluetooth_connect_permission = true;
					}
				}
			}
		}
	}

	@Override
	public void onResume() {
		AltosDebug.debug("+ ON RESUME +");

		super.onResume();

		if (!asked_permission) {
			asked_permission = true;
			if (ActivityCompat.checkSelfPermission(this,
							      Manifest.permission.ACCESS_FINE_LOCATION)
			    == PackageManager.PERMISSION_GRANTED)
			{
				have_location_permission = true;
			}
			if (ActivityCompat.checkSelfPermission(this,
							       Manifest.permission.WRITE_EXTERNAL_STORAGE)
			    == PackageManager.PERMISSION_GRANTED)
			{
				have_storage_permission = true;
			}
			if (ActivityCompat.checkSelfPermission(this,
							       Manifest.permission.BLUETOOTH)
			    == PackageManager.PERMISSION_GRANTED)
			{
				have_bluetooth_permission = true;
			}
			if (ActivityCompat.checkSelfPermission(this,
							       BLUETOOTH_CONNECT)
			    == PackageManager.PERMISSION_GRANTED)
			{
				have_bluetooth_connect_permission = true;
			}
			int count = 0;
			if (!have_location_permission)
				count += 1;
			if (!have_storage_permission)
				count += 1;
			if (!have_bluetooth_permission)
				count += 1;
			if (!have_bluetooth_connect_permission)
				count += 1;
			if (count > 0)
			{
				String[] permissions = new String[count];
				int i = 0;
				if (!have_location_permission)
					permissions[i++] = Manifest.permission.ACCESS_FINE_LOCATION;
				if (!have_storage_permission)
					permissions[i++] = Manifest.permission.WRITE_EXTERNAL_STORAGE;
				if (!have_bluetooth_permission)
					permissions[i++] = Manifest.permission.BLUETOOTH;
				if (!have_bluetooth_connect_permission)
					permissions[i++] = BLUETOOTH_CONNECT;
				ActivityCompat.requestPermissions(this, permissions, MY_PERMISSION_REQUEST);
			}
		}
		if (have_location_permission)
			enable_location_updates(false);
	}

	@Override
	public void onPause() {
		AltosDebug.debug("- ON PAUSE -");

		super.onPause();

		// Stop listening for location updates
		if (have_location_permission)
			((LocationManager) getSystemService(Context.LOCATION_SERVICE)).removeUpdates(this);
	}

	@Override
	public void onStop() {
		AltosDebug.debug("-- ON STOP --");

		super.onStop();
	}

	@Override
	public void onDestroy() {
		AltosDebug.debug("--- ON DESTROY ---");

		super.onDestroy();

		saved_state = null;

		doUnbindService();
		if (mAltosVoice != null) {
			mAltosVoice.stop();
			mAltosVoice = null;
		}
		stop_timer();
	}

	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		AltosDebug.debug("onActivityResult request %d result %d", requestCode, resultCode);
		switch (requestCode) {
		case REQUEST_CONNECT_DEVICE:
			// When DeviceListActivity returns with a device to connect to
			if (resultCode == Activity.RESULT_OK) {
				connectDevice(data);
			}
			break;
		case REQUEST_ENABLE_BT:
			// When the request to enable Bluetooth returns
			if (resultCode == Activity.RESULT_OK) {
				// Bluetooth is now enabled, so set up a chat session
				//setupChat();
				AltosDebug.debug("BT enabled");
				bluetoothEnabled(data);
			} else {
				// User did not enable Bluetooth or an error occured
				AltosDebug.debug("BT not enabled");
			}
			break;
		case REQUEST_IDLE_MODE:
			if (resultCode == Activity.RESULT_OK)
				idle_mode(data);
			break;
		case REQUEST_IGNITERS:
			break;
		case REQUEST_SETUP:
			if (resultCode == Activity.RESULT_OK)
				note_setup_changes(data);
			break;
		case REQUEST_SELECT_TRACKER:
			if (resultCode == Activity.RESULT_OK)
				select_tracker(data);
			break;
		case REQUEST_DELETE_TRACKER:
			if (resultCode == Activity.RESULT_OK)
				delete_track(data);
			break;
		}
	}

	private void note_setup_changes(Intent data) {
		int changes = data.getIntExtra(SetupActivity.EXTRA_SETUP_CHANGES, 0);

		AltosDebug.debug("note_setup_changes changes %d\n", changes);

		if ((changes & SETUP_BAUD) != 0) {
			try {
				mService.send(Message.obtain(null, TelemetryService.MSG_SETBAUD,
							     AltosPreferences.telemetry_rate(1)));
			} catch (RemoteException re) {
			}
		}
		if ((changes & SETUP_UNITS) != 0) {
			/* nothing to do here */
		}
		if ((changes & SETUP_MAP_SOURCE) != 0) {
			/* nothing to do here */
		}
		if ((changes & SETUP_MAP_TYPE) != 0) {
			/* nothing to do here */
		}
		set_switch_time();
		if ((changes & SETUP_FONT_SIZE) != 0) {
			AltosDebug.debug(" ==== Recreate to switch font sizes ==== ");
			finish();
			startActivity(getIntent());
		}
	}

	private void connectUsb(UsbDevice device) {
		if (mService == null)
			pending_usb_device = device;
		else {
			// Attempt to connect to the device
			try {
				mService.send(Message.obtain(null, TelemetryService.MSG_OPEN_USB, device));
				AltosDebug.debug("Sent OPEN_USB message");
			} catch (RemoteException e) {
				AltosDebug.debug("connect device message failed");
			}
		}
	}

	private void bluetoothEnabled(Intent data) {
		if (mService != null) {
			try {
				mService.send(Message.obtain(null, TelemetryService.MSG_BLUETOOTH_ENABLED, null));
			} catch (RemoteException e) {
				AltosDebug.debug("send BT enabled message failed");
			}
		}
	}

	private void connectDevice(Intent data) {
		// Attempt to connect to the device
		try {
			String address = data.getExtras().getString(DeviceListActivity.EXTRA_DEVICE_ADDRESS);
			String name = data.getExtras().getString(DeviceListActivity.EXTRA_DEVICE_NAME);

			AltosDebug.debug("Connecting to " + address + " " + name);
			DeviceAddress	a = new DeviceAddress(address, name);
			mService.send(Message.obtain(null, TelemetryService.MSG_CONNECT, a));
			AltosDebug.debug("Sent connecting message");
		} catch (RemoteException e) {
			AltosDebug.debug("connect device message failed");
		}
	}

	private void disconnectDevice(boolean remember) {
		try {
			mService.send(Message.obtain(null, TelemetryService.MSG_DISCONNECT, (Boolean) remember));
		} catch (RemoteException e) {
		}
	}

	private void idle_mode(Intent data) {
		int type = data.getIntExtra(IdleModeActivity.EXTRA_IDLE_RESULT, -1);
		Message msg;

		AltosDebug.debug("intent idle_mode %d", type);
		switch (type) {
		case IdleModeActivity.IDLE_MODE_CONNECT:
			msg = Message.obtain(null, TelemetryService.MSG_MONITOR_IDLE_START);
			try {
				mService.send(msg);
			} catch (RemoteException re) {
			}
			break;
		case IdleModeActivity.IDLE_MODE_DISCONNECT:
			msg = Message.obtain(null, TelemetryService.MSG_MONITOR_IDLE_STOP);
			try {
				mService.send(msg);
			} catch (RemoteException re) {
			}
			break;
		case IdleModeActivity.IDLE_MODE_REBOOT:
			msg = Message.obtain(null, TelemetryService.MSG_REBOOT);
			try {
				mService.send(msg);
			} catch (RemoteException re) {
			}
			break;
		case IdleModeActivity.IDLE_MODE_IGNITERS:
			Intent serverIntent = new Intent(this, IgniterActivity.class);
			startActivityForResult(serverIntent, REQUEST_IGNITERS);
			break;
		}
	}

	boolean fail_shown;

	private void file_failed(File file) {
		if (!fail_shown) {
			fail_shown = true;
			AlertDialog fail = new AlertDialog.Builder(this).create();
			fail.setTitle("Failed to Create Log File");
			fail.setMessage(file.getPath());
			fail.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
				       new DialogInterface.OnClickListener() {
					       public void onClick(DialogInterface dialog, int which) {
						       dialog.dismiss();
					       }
				       });
			fail.show();
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.option_menu, menu);
		return true;
	}

	double telem_frequency = 434.550;
	double selected_frequency = AltosLib.MISSING;

	void setFrequency(double freq) {
		telem_frequency = freq;
		selected_frequency = AltosLib.MISSING;
		try {
			mService.send(Message.obtain(null, TelemetryService.MSG_SETFREQUENCY, freq));
			set_switch_time();
		} catch (RemoteException e) {
		}
	}

	void setFrequency(AltosFrequency frequency) {
		setFrequency (frequency.frequency);
	}

	void setBaud(int baud) {
		try {
			mService.send(Message.obtain(null, TelemetryService.MSG_SETBAUD, baud));
			set_switch_time();
		} catch (RemoteException e) {
		}
	}

	void setBaud(String baud) {
		try {
			int	value = Integer.parseInt(baud);
			int	rate = AltosLib.ao_telemetry_rate_38400;
			switch (value) {
			case 2400:
				rate = AltosLib.ao_telemetry_rate_2400;
				break;
			case 9600:
				rate = AltosLib.ao_telemetry_rate_9600;
				break;
			case 38400:
				rate = AltosLib.ao_telemetry_rate_38400;
				break;
			}
			setBaud(rate);
		} catch (NumberFormatException e) {
		}
	}

	void select_tracker(int serial, double frequency) {

		AltosDebug.debug("select tracker %d %7.3f\n", serial, frequency);

		if (serial == selected_serial) {
			AltosDebug.debug("%d already selected\n", serial);
			return;
		}

		if (serial != 0) {
			int i;
			for (i = 0; i < trackers.length; i++)
				if (trackers[i].serial == serial)
					break;

			if (i == trackers.length) {
				AltosDebug.debug("attempt to select unknown tracker %d\n", serial);
				return;
			}
			if (frequency != 0.0 && frequency != AltosLib.MISSING)
				setFrequency(frequency);
		}

		selected_serial = serial;
		update_state(null);
	}

	void select_tracker(Intent data) {
		int serial = data.getIntExtra(SelectTrackerActivity.EXTRA_SERIAL_NUMBER, 0);
		double frequency = data.getDoubleExtra(SelectTrackerActivity.EXTRA_FREQUENCY, 0.0);
		select_tracker(serial, frequency);
	}

	void delete_track(int serial) {
		try {
			mService.send(Message.obtain(null, TelemetryService.MSG_DELETE_SERIAL, (Integer) serial));
		} catch (Exception ex) {
		}
	}

	void delete_track(Intent data) {
		int serial = data.getIntExtra(SelectTrackerActivity.EXTRA_SERIAL_NUMBER, 0);
		if (serial != 0)
			delete_track(serial);
	}

	void start_select_tracker(Tracker[] select_trackers, int title_id, int request) {
		Intent intent = new Intent(this, SelectTrackerActivity.class);
		AltosDebug.debug("put title id 0x%x %s", title_id, getResources().getString(title_id));
		intent.putExtra(EXTRA_TRACKERS_TITLE, title_id);
		if (select_trackers != null) {
			ArrayList<Tracker> tracker_array = new ArrayList<Tracker>(Arrays.asList(select_trackers));
			intent.putParcelableArrayListExtra(EXTRA_TRACKERS, tracker_array);
		} else {
			intent.putExtra(EXTRA_TRACKERS, (Parcelable[]) null);
		}
		startActivityForResult(intent, request);
	}

	void start_select_tracker(Tracker[] select_trackers) {
		start_select_tracker(select_trackers, R.string.select_tracker, REQUEST_SELECT_TRACKER);
	}

	void touch_trackers(Integer[] serials) {
		Tracker[] my_trackers = new Tracker[serials.length];

		for (int i = 0; i < serials.length; i++) {
			AltosState	s = telemetry_state.get(serials[i]);
			my_trackers[i] = new Tracker(s);
		}
		start_select_tracker(my_trackers);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		Intent serverIntent = null;
		switch (item.getItemId()) {
		case R.id.connect_scan:
			ensureBluetooth();
			// Launch the DeviceListActivity to see devices and do scan
			serverIntent = new Intent(this, DeviceListActivity.class);
			startActivityForResult(serverIntent, REQUEST_CONNECT_DEVICE);
			return true;
		case R.id.disconnect:
			/* Disconnect the device
			 */
			disconnectDevice(false);
			return true;
		case R.id.quit:
			AltosDebug.debug("R.id.quit");
			disconnectDevice(true);
			finish();
			return true;
		case R.id.setup:
			serverIntent = new Intent(this, SetupActivity.class);
			startActivityForResult(serverIntent, REQUEST_SETUP);
			return true;
		case R.id.select_freq:
			// Set the TBT radio frequency

			final AltosFrequency[] frequencies = AltosPreferences.common_frequencies();
			String[] frequency_strings = new String[frequencies.length];
			for (int i = 0; i < frequencies.length; i++)
				frequency_strings[i] = frequencies[i].toString();

			AlertDialog.Builder builder_freq = new AlertDialog.Builder(this);
			builder_freq.setTitle("Select Frequency");
			builder_freq.setItems(frequency_strings,
					 new DialogInterface.OnClickListener() {
						 public void onClick(DialogInterface dialog, int item) {
							 setFrequency(frequencies[item]);
							 selected_frequency = frequencies[item].frequency;
						 }
					 });
			AlertDialog alert_freq = builder_freq.create();
			alert_freq.show();
			return true;
		case R.id.select_tracker:
			start_select_tracker(trackers);
			return true;
		case R.id.delete_track:
			if (trackers != null && trackers.length > 0)
				start_select_tracker(trackers, R.string.delete_track, REQUEST_DELETE_TRACKER);
			return true;
		case R.id.idle_mode:
			serverIntent = new Intent(this, IdleModeActivity.class);
			serverIntent.putExtra(EXTRA_IDLE_MODE, idle_mode);
			serverIntent.putExtra(EXTRA_FREQUENCY, telem_frequency);
			startActivityForResult(serverIntent, REQUEST_IDLE_MODE);
			return true;
		}
		return false;
	}

	static String direction(AltosGreatCircle from_receiver,
				Location receiver) {
		if (from_receiver == null)
			return null;

		if (receiver == null)
			return null;

		if (!receiver.hasBearing())
			return null;

		float	bearing = receiver.getBearing();
		float	heading = (float) from_receiver.bearing - bearing;

		while (heading <= -180.0f)
			heading += 360.0f;
		while (heading > 180.0f)
			heading -= 360.0f;

		int iheading = (int) (heading + 0.5f);

		if (-1 < iheading && iheading < 1)
			return "ahead";
		else if (iheading < -179 || 179 < iheading)
			return "backwards";
		else if (iheading < 0)
			return String.format("left %d°", -iheading);
		else
			return String.format("right %d°", iheading);
	}

	public void onLocationChanged(Location location) {
		this.location = location;
		AltosDebug.debug("Location changed to %f,%f",
				 location.getLatitude(),
				 location.getLongitude());
		update_ui(telemetry_state, state, false);
	}

	public void onStatusChanged(String provider, int status, Bundle extras) {
		AltosDebug.debug("Location status now %d\n", status);
	}

	public void onProviderEnabled(String provider) {
		AltosDebug.debug("Location provider enabled %s\n", provider);
	}

	public void onProviderDisabled(String provider) {
		AltosDebug.debug("Location provider disabled %s\n", provider);
	}
}
