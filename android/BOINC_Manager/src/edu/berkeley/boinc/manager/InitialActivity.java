package edu.berkeley.boinc.manager;

import java.util.ArrayList;

import edu.berkeley.boinc.client.ClientStatus;
import edu.berkeley.boinc.client.ClientStatusMonitor;
import android.os.Bundle;
import android.os.IBinder;
import android.app.ActionBar;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;


public class InitialActivity extends FragmentActivity implements ActionBar.OnNavigationListener{
	
	private final String TAG = "InitialActivity"; 
	
	private ClientStatusMonitor monitor;
	public static ClientStatus client;
	
	private Boolean mIsBound;

	private ServiceConnection mConnection = new ServiceConnection() {
	    public void onServiceConnected(ComponentName className, IBinder service) {
	        // This is called when the connection with the service has been established, getService returns the Monitor object that is needed to call functions.
	        monitor = ((ClientStatusMonitor.LocalBinder)service).getService();
	        Log.d(TAG, "onServiceConnected: ClientStatusMonitor bound.");
	    }

	    public void onServiceDisconnected(ComponentName className) {
	        // This should not happen
	        monitor = null;
	        Toast.makeText(getApplicationContext(), "service disconnected", Toast.LENGTH_SHORT).show();
	    }
	};

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_initial);
        Log.d(TAG, "onCreate"); 
        
        bindMonitorService();
        
        // populate data for drop down (spinner) menu
        ArrayList<SpinnerMenuEntry> menuData = new ArrayList<SpinnerMenuEntry>();
        if(getResources().getBoolean(R.bool.menu_show_home)) menuData.add(new SpinnerMenuEntry(R.drawable.homew, R.string.menu_home));
        if(getResources().getBoolean(R.bool.menu_show_tasks)) menuData.add(new SpinnerMenuEntry(R.drawable.chartw, R.string.menu_tasks));
        if(getResources().getBoolean(R.bool.menu_show_transfers)) menuData.add(new SpinnerMenuEntry(R.drawable.exportw, R.string.menu_transfers));
        if(getResources().getBoolean(R.bool.menu_show_messages)) menuData.add(new SpinnerMenuEntry(R.drawable.mailw, R.string.menu_messages));
        if(getResources().getBoolean(R.bool.menu_show_debug)) menuData.add(new SpinnerMenuEntry(R.drawable.bugw, R.string.menu_debug));
        if(getResources().getBoolean(R.bool.menu_show_settings)) menuData.add(new SpinnerMenuEntry(R.drawable.wrenchw, R.string.menu_settings));
        SpinnerMenuAdapter mSpinnerAdapter = new SpinnerMenuAdapter(menuData);

        final ActionBar ab = getActionBar();

        // set defaults for logo & home up
        ab.setDisplayUseLogoEnabled(true);
        ab.setDisplayOptions(ActionBar.DISPLAY_SHOW_TITLE);
        
        // set spinner list
        ab.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
        ab.setListNavigationCallbacks(mSpinnerAdapter, this);
        
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
    	MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.activity_initial, menu);
        return true;
    }


    @Override
    /*
     * gets called when user selects action item ( see onCreateOptions) passing its ID
     */
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            // TODO handle clicking the app icon/logo
            return false;
        case R.id.menu_resume_computation:
        	Log.d(TAG,"menu_resume_computation clicked");
            return false;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    @Override
	protected void onDestroy() {
    	Log.d(TAG, "onDestroy");
	    super.onDestroy();
	    
	    unbindMonitorService();
	}
	
	private void bindMonitorService() {
	    // Establish a connection with the service, onServiceConnected gets called when finished.
		Intent i = new Intent(this,edu.berkeley.boinc.client.ClientStatusMonitor.class);
		this.startService(i);
		this.bindService(i, mConnection, Context.BIND_AUTO_CREATE);
	    mIsBound = true;
	}
	
	private void unbindMonitorService() {
	    if (mIsBound) {
	        // Detach existing connection.
	        unbindService(mConnection);
	        mIsBound = false;
	    }
	}
	
	public class SpinnerMenuEntry {
		public int iconId;
		public int titleId;
		public SpinnerMenuEntry(int icon, int title) {
			this.iconId = icon;
			this.titleId = title;
		}
	}
	
	public class SpinnerMenuAdapter extends BaseAdapter{
		ArrayList<SpinnerMenuEntry> data;
		LayoutInflater inflater;

		public SpinnerMenuAdapter(ArrayList<SpinnerMenuEntry> data) {
			this.data = data;
			inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);

		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			//setup view
			View v = convertView;
			v = inflater.inflate(R.layout.menu_spinner_item_layout, null);
			
			//get data
			SpinnerMenuEntry sme = data.get(position);
			
			//modify view elements according to data in sme:
			TextView title = (TextView) v.findViewById(R.id.name);
			title.setText(sme.titleId);
			ImageView image = (ImageView) v.findViewById(R.id.icon);
			image.setImageResource(sme.iconId);
			return v;
		}

		@Override
		public int getCount() {
			return data.size();
		}

		@Override
		public Object getItem(int arg0) {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public long getItemId(int arg0) {
			// TODO Auto-generated method stub
			return 0;
		}
	}

	@Override
	public boolean onNavigationItemSelected(int itemPosition, long itemId) {
		// TODO Auto-generated method stub
		Log.d(TAG,"onNavigationItemSelected at position " + itemPosition);
		return false;
	}
	
}
