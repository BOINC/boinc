/*******************************************************************************
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
 * 
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
package edu.berkeley.boinc.client;

import edu.berkeley.boinc.utils.*;

import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.wifi.WifiManager;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.Log;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.CcStatus;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.rpc.Result;
import edu.berkeley.boinc.rpc.Transfer;
import edu.berkeley.boinc.rpc.HostInfo;
import edu.berkeley.boinc.utils.BOINCDefs;

/*
 * Singleton that holds the client status data, as determined by the Monitor.
 * To get instance call Monitor.getClientStatus()
 */
public class ClientStatus {
	
	private Context ctx; // application context in order to fire broadcast events
	
	// CPU WakeLock
	private WakeLock wakeLock;
	// WiFi lock
	private WifiManager.WifiLock wifiLock;
	
	//RPC wrapper
	private CcStatus status;
	private ArrayList<Result> results;
	private ArrayList<Project> projects;
	private ArrayList<Transfer> transfers;
	private GlobalPreferences prefs;
	private HostInfo hostinfo;
	
	// setup status
	public Integer setupStatus = 0;
	public static final int SETUP_STATUS_LAUNCHING = 0; // 0 = client is in setup routine (default)
	public static final int SETUP_STATUS_AVAILABLE = 1; // 1 = client is launched and available for RPC (connected and authorized)
	public static final int SETUP_STATUS_ERROR = 2; // 2 = client is in a permanent error state
	public static final int SETUP_STATUS_NOPROJECT = 3; // 3 = client is launched but not attached to a project (login)
	public static final int SETUP_STATUS_CLOSING = 4; // 4 = client is shutting down
	public static final int SETUP_STATUS_CLOSED = 5; // 5 = client shut down
	private Boolean setupStatusParseError = false;
	
	// computing status
	public Integer computingStatus = 2;
	public static final int COMPUTING_STATUS_NEVER = 0;
	public static final int COMPUTING_STATUS_SUSPENDED = 1;
	public static final int COMPUTING_STATUS_IDLE = 2;
	public static final int COMPUTING_STATUS_COMPUTING = 3;
	public Integer computingSuspendReason = 0; //reason why computing got suspended, only if COMPUTING_STATUS_SUSPENDED
	private Boolean computingParseError = false; //indicates that status could not be parsed and is therefore invalid
	
	// network status
	public Integer networkStatus = 2;
	public static final int NETWORK_STATUS_NEVER = 0;
	public static final int NETWORK_STATUS_SUSPENDED = 1;
	public static final int NETWORK_STATUS_AVAILABLE = 2;
	public Integer networkSuspendReason = 0; //reason why network activity got suspended, only if NETWORK_STATUS_SUSPENDED
	private Boolean networkParseError = false; //indicates that status could not be parsed and is therefore invalid
	
	// supported projects
	private ArrayList<ProjectInfo> supportedProjects = new ArrayList<ProjectInfo>();
	
	public ClientStatus(Context ctx) {
		this.ctx = ctx;
		
		// set up CPU Wake Lock
		// see documentation at http://developer.android.com/reference/android/os/PowerManager.html
		PowerManager pm = (PowerManager) ctx.getSystemService(Context.POWER_SERVICE);
		wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, Logging.TAG);
		wakeLock.setReferenceCounted(false); // "one call to release() is sufficient to undo the effect of all previous calls to acquire()"
		
		// set up Wifi wake lock
		WifiManager wm = (WifiManager) ctx.getSystemService(Context.WIFI_SERVICE);
		wifiLock = wm.createWifiLock(WifiManager.WIFI_MODE_FULL , "MyWifiLock");
		wifiLock.setReferenceCounted(false);
	}
	
	// call to acquire or release resources held by the WakeLock.
	// acquisition: every time the Monitor loop calls setClientStatus and computingStatus == COMPUTING_STATUS_COMPUTING
	// release: every time acquisition criteria is not met , and in Monitor.onDestroy()
	@SuppressLint("Wakelock")
	public void setWakeLock(Boolean acquire) {
		try {
			if(wakeLock.isHeld() == acquire) return; // wakeLock already in desired state
			
			if(acquire) { // acquire wakeLock
				wakeLock.acquire();
				if(Logging.DEBUG) Log.d(Logging.TAG, "wakeLock acquired");
			} else { // release wakeLock
				wakeLock.release();
				if(Logging.DEBUG) Log.d(Logging.TAG, "wakeLock released");
			}
		} catch (Exception e) {if(Logging.WARNING) Log.w(Logging.TAG, "Exception durign setWakeLock " + acquire, e);}
	}
	
	// call to acquire or release resources held by the WifiLock.
	// acquisition: every time the Monitor loop calls setClientStatus: computingStatus == COMPUTING_STATUS_COMPUTING || COMPUTING_STATUS_IDLE
	// release: every time acquisition criteria is not met , and in Monitor.onDestroy()
	public void setWifiLock(Boolean acquire) {
		try {
			if(wifiLock.isHeld() == acquire) return; // wifiLock already in desired state
			
			if(acquire) { // acquire wakeLock
				wifiLock.acquire();
				if(Logging.DEBUG) Log.d(Logging.TAG, "wifiLock acquired");
			} else { // release wakeLock
				wifiLock.release();
				if(Logging.DEBUG) Log.d(Logging.TAG, "wifiLock released");
			}
		} catch (Exception e) {if(Logging.WARNING) Log.w(Logging.TAG, "Exception durign setWifiLock " + acquire, e);}
	}
	
	/*
	 * fires "clientstatuschange" broadcast, so registered Activities can update their model.
	 */
	public synchronized void fire() {
		if(ctx!=null) {
	        Intent clientChanged = new Intent();
	        clientChanged.setAction("edu.berkeley.boinc.clientstatuschange");
			ctx.sendBroadcast(clientChanged,null);
		}else {
			if(Logging.DEBUG) Log.d(Logging.TAG,"ClientStatus cant fire, not context set!");
		}
	}
	
	/*
	 * called frequently by Monitor to set the RPC data. These objects are used to determine the client status and parse it in the data model of this class.
	 */
	public synchronized void setClientStatus(CcStatus status,ArrayList<Result> results,ArrayList<Project> projects, ArrayList<Transfer> transfers, HostInfo hostinfo) {
		this.status = status;
		this.results = results;
		this.projects = projects;
		this.transfers = transfers;
		this.hostinfo = hostinfo;
		parseClientStatus();
		if(Logging.VERBOSE) Log.v(Logging.TAG,"setClientStatus: #results:" + results.size() + " #projects:" + projects.size() + " #transfers:" + transfers.size() + " // computing: " + computingParseError + computingStatus + computingSuspendReason + " - network: " + networkParseError + networkStatus + networkSuspendReason);
		if(!computingParseError && !networkParseError && !setupStatusParseError) {
			fire(); // broadcast that status has changed
		} else {
			if(Logging.DEBUG) Log.d(Logging.TAG, "ClientStatus discard status change due to parse error" + computingParseError + computingStatus + computingSuspendReason + "-" + networkParseError + networkStatus + networkSuspendReason + "-" + setupStatusParseError);
		}
	}
	
	/*
	 * called when setup status needs to be manipulated by Java routine
	 * either during setup or closing of client.
	 * this function does not effect the state of the client! 
	 */
	public synchronized void setSetupStatus(Integer newStatus, Boolean fireStatusChangeEvent) {
		setupStatus = newStatus;
		if (fireStatusChangeEvent) fire();
	}
	
	/* 
	 * called after reading global preferences, e.g. during ClientStartAsync
	 */
	public synchronized void setPrefs(GlobalPreferences prefs) {
		//if(Logging.DEBUG) Log.d(Logging.TAG, "setPrefs");
		this.prefs = prefs;
	}
	
	public synchronized void setSupportedProjects (ArrayList<ProjectInfo> projects) {
		this.supportedProjects = projects;
	}
	
	public synchronized ArrayList<ProjectInfo> getSupportedProjects () {
		return supportedProjects;
	}
	
	public synchronized CcStatus getClientStatus() {
		if(results == null) { //check in case monitor is not set up yet (e.g. while logging in)
			if(Logging.DEBUG) Log.d(Logging.TAG, "state is null");
			return null;
		}
		return status;
	}
	
	public synchronized ArrayList<Result> getTasks() {
		if(results == null) { //check in case monitor is not set up yet (e.g. while logging in)
			if(Logging.DEBUG) Log.d(Logging.TAG, "state is null");
			return null;
		}
		return results;
	}
	
	public synchronized ArrayList<Transfer> getTransfers() {
		if(transfers == null) { //check in case monitor is not set up yet (e.g. while logging in)
			if(Logging.DEBUG) Log.d(Logging.TAG, "transfers is null");
			return null;
		}
		return transfers;
	}
	
	public synchronized GlobalPreferences getPrefs() {
		if(prefs == null) { //check in case monitor is not set up yet (e.g. while logging in)
			if(Logging.DEBUG) Log.d(Logging.TAG, "prefs is null");
			return null;
		}
		return prefs;
	}
	
	public synchronized ArrayList<Project> getProjects() {
		if(projects == null) { //check in case monitor is not set up yet (e.g. while logging in)
			if(Logging.DEBUG) Log.d(Logging.TAG, "getProject() state is null");
			return null;
		}
		return projects;
	}
	
	public synchronized HostInfo getHostInfo() {
		if(hostinfo == null) {
			if(Logging.DEBUG) Log.d(Logging.TAG, "getHostInfo() state is null");
			return null;
		}
		return hostinfo;
	}

	// updates list of slideshow images of all projects
	// takes list and updates content
	// returns true, if changes found.
	// 126 * 290 pixel from /projects/PNAME/slideshow_appname_n
	// not aware of project or application!
	public synchronized Boolean updateSlideshowImages(ArrayList<ImageWrapper> slideshowImages) {
		if(Logging.DEBUG) Log.d(Logging.TAG, "updateSlideshowImages()");

		int maxImagesPerProject = ctx.getResources().getInteger(R.integer.status_max_slideshow_images_per_project);
		Boolean change = false;
		
		// removing images of detached projects
		// use iterator to safely remove while iterating
		Iterator<ImageWrapper> iImage = slideshowImages.iterator();
		Integer counter = 0;
		while(iImage.hasNext()) {
			Boolean found = false;
			ImageWrapper image = iImage.next();
			for (Project project: projects) {
				if(project.project_name.equals(image.projectName)){
					found = true;
					continue;
				}
			}
			if(!found) {
				iImage.remove();
				counter++;
			}
		}
		if(Logging.DEBUG) Log.d(Logging.TAG, "updateSlideshowImages() has removed " + counter + " images.");
		
		// adding new images
		counter = 0;
		for (Project project: projects) {
			try{
				// check how many images project is allowed to add
				int numberOfLoadedImages = 0;
				for(ImageWrapper image: slideshowImages){
					if(image.projectName.equals(project.project_name)) numberOfLoadedImages++;
				}
				if(numberOfLoadedImages >= maxImagesPerProject) continue;
				
				// get file paths of soft link files
				File dir = new File(project.project_dir);
				File[] foundFiles = dir.listFiles(new FilenameFilter() {
				    public boolean accept(File dir, String name) {
				        return name.startsWith("slideshow_") && !name.endsWith(".png");
				    }
				});
				if(foundFiles == null) continue; // prevent NPE
				
				ArrayList<String> allImagePaths = new ArrayList<String>();
				for (File file: foundFiles) {
					String slideshowImagePath = parseSoftLinkToAbsPath(file.getAbsolutePath(), project.project_dir);
					//check whether path is not empty, and avoid duplicates (slideshow images can 
					//re-occur for multiple apps, since we do not distinct apps, skip duplicates.
					if(slideshowImagePath != null && !slideshowImagePath.isEmpty() && !allImagePaths.contains(slideshowImagePath)) allImagePaths.add(slideshowImagePath);
					//if(Logging.DEBUG) Log.d(Logging.TAG, "getSlideshowImages() path: " + slideshowImagePath);
				}
				//if(Logging.DEBUG) Log.d(Logging.TAG,"getSlideshowImages() retrieve number file paths: " + filePaths.size());
				
				// load images from paths
				for (String filePath : allImagePaths) {
					Boolean load = true;
					if(numberOfLoadedImages >= maxImagesPerProject) load = false;
					// check whether image is new
					for (ImageWrapper image: slideshowImages) {
						if(image.path.equals(filePath)) load = false;
					}
					
					// project is allowed to add new images
					// this image is not loaded yet
					// -> load!
					if(load){
						Bitmap tmp = BitmapFactory.decodeFile(filePath);
						if(tmp!=null) {
							slideshowImages.add(new ImageWrapper(tmp,project.project_name, filePath));
							numberOfLoadedImages++;
							change = true;
							counter++;
						}
						else if(Logging.DEBUG) Log.d(Logging.TAG,"loadSlideshowImagesFromFile(): null for path: " + filePath);
					}
				}
			} catch(Exception e) {if(Logging.WARNING) Log.w(Logging.TAG,"exception for project " + project.master_url,e);}
		}
		if(Logging.DEBUG) Log.d(Logging.TAG, "updateSlideshowImages() has added " + counter + " images.");
		if(Logging.DEBUG) Log.d(Logging.TAG,"updateSlideshowImages() slideshow contains " + slideshowImages.size() + " bitmaps.");
		return change;
	}
	
	// returns project icon for given master url
	// bitmap: 40 * 40 pixel, symbolic link in /projects/PNAME/stat_icon
	public synchronized Bitmap getProjectIcon (String masterUrl) {
		if(Logging.VERBOSE) Log.v(Logging.TAG, "getProjectIcon for: " + masterUrl);
		try{
			// loop through all projects
			for (Project project: projects) {
				if(project.master_url.equals(masterUrl)) {
					// read file name of icon
					String iconAbsPath = parseSoftLinkToAbsPath(project.project_dir + "/stat_icon", project.project_dir);
					if (iconAbsPath == null) {
						if(Logging.VERBOSE) Log.v(Logging.TAG, "getProjectIcon could not parse sym link for project: " + masterUrl);
						return null;
					}
					//if(Logging.DEBUG) Log.d(Logging.TAG, "getProjectIcons() absolute path to icon: " + iconAbsPath);
					Bitmap icon = BitmapFactory.decodeFile(iconAbsPath);
					return icon;
				}
			}
		} catch (Exception e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "getProjectIcon failed", e);
		}
		if(Logging.WARNING) Log.w(Logging.TAG, "getProjectIcon: project not found.");
		return null;
	}
	
	// returns a string describing the current client status.
	// use this method, to harmonize UI text, e.g. in Notification, Status Tab, App Title.
	public String getCurrentStatusString() {
		String statusString = "";
		try {
			switch(setupStatus) {
			case SETUP_STATUS_AVAILABLE:
				switch(computingStatus) {
				case COMPUTING_STATUS_COMPUTING:
					statusString = ctx.getString(R.string.status_running);
					break;
				case COMPUTING_STATUS_IDLE:
					statusString = ctx.getString(R.string.status_idle);
					break;
				case COMPUTING_STATUS_SUSPENDED:
					switch(computingSuspendReason) {
					case BOINCDefs.SUSPEND_REASON_USER_REQ:
						// restarting after user has previously manually suspended computation
						statusString = ctx.getString(R.string.suspend_user_req);
						break;
					case BOINCDefs.SUSPEND_REASON_BENCHMARKS:
						statusString = ctx.getString(R.string.status_benchmarking);
						break;
					default:
						statusString = ctx.getString(R.string.status_paused);
						break;
					}
					break;
				case COMPUTING_STATUS_NEVER:
					statusString = ctx.getString(R.string.status_computing_disabled);
					break;
				}
				break;
			case SETUP_STATUS_CLOSING:
				statusString = ctx.getString(R.string.status_closing);
				break;
			case SETUP_STATUS_LAUNCHING:
				statusString = ctx.getString(R.string.status_launching);
				break;
			case SETUP_STATUS_NOPROJECT:
				statusString = ctx.getString(R.string.status_noproject);
				break;
			}
		} catch (Exception e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error parsing setup status string",e);
		}
		return statusString;
	}
	
	/*
	 * parses RPC data to ClientStatus data model.
	 */
	private void parseClientStatus() {
		parseComputingStatus();
		parseProjectStatus();
		parseNetworkStatus();
	}
	
	private void parseProjectStatus() {
		try {
			if (projects.size() > 0) { 
				setupStatus = SETUP_STATUS_AVAILABLE;
				setupStatusParseError = false;
			} else { //not projects attached
				setupStatus = SETUP_STATUS_NOPROJECT;
				setupStatusParseError = false;
			}
		} catch (Exception e) {
			setupStatusParseError = true;
			if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "parseProjectStatus - Exception", e);
			if(Logging.DEBUG) Log.d(Logging.TAG, "error parsing setup status (project state)");
		}
	}
	
	private void parseComputingStatus() {
		computingParseError = true;
		try {
			if(status.task_mode==BOINCDefs.RUN_MODE_NEVER) {
				computingStatus = COMPUTING_STATUS_NEVER;
				computingSuspendReason = status.task_suspend_reason; // = 4 - SUSPEND_REASON_USER_REQ????
				computingParseError = false;
				return;
			}
			if(status.task_mode == BOINCDefs.RUN_MODE_AUTO && status.task_suspend_reason == BOINCDefs.SUSPEND_REASON_CPU_THROTTLE) {
				// suspended due to CPU throttling, treat as if was running!
				computingStatus = COMPUTING_STATUS_COMPUTING;
				computingSuspendReason = status.task_suspend_reason; // = 64 - SUSPEND_REASON_CPU_THROTTLE
				computingParseError = false;
				return;
				
			}
			if((status.task_mode == BOINCDefs.RUN_MODE_AUTO) && (status.task_suspend_reason != BOINCDefs.SUSPEND_NOT_SUSPENDED)) {
				computingStatus = COMPUTING_STATUS_SUSPENDED;
				computingSuspendReason = status.task_suspend_reason;
				computingParseError = false;
				return;
			}
			if((status.task_mode == BOINCDefs.RUN_MODE_AUTO) && (status.task_suspend_reason == BOINCDefs.SUSPEND_NOT_SUSPENDED)) {
				//figure out whether we have an active task
				Boolean activeTask = false;
				if(results!=null) {
					for(Result task: results) {
						if(task.active_task) { // this result has corresponding "active task" in RPC XML
							activeTask = true;
							continue; // amount of active tasks does not matter.
						}
					}
				}
				
				if(activeTask) { // client is currently computing
					computingStatus = COMPUTING_STATUS_COMPUTING;
					computingSuspendReason = status.task_suspend_reason; // = 0 - SUSPEND_NOT_SUSPENDED
					computingParseError = false;
					return;
				} else { // client "is able but idle"
					computingStatus = COMPUTING_STATUS_IDLE;
					computingSuspendReason = status.task_suspend_reason; // = 0 - SUSPEND_NOT_SUSPENDED
					computingParseError = false;
					return;
				}
			}
		} catch (Exception e) {
			if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "ClientStatus parseComputingStatus - Exception", e);
			if(Logging.DEBUG) Log.d(Logging.TAG, "ClientStatus error - client computing status");
		}
	}
	
	private void parseNetworkStatus() {
		networkParseError = true;
		try {
			if(status.network_mode==BOINCDefs.RUN_MODE_NEVER) {
				networkStatus = NETWORK_STATUS_NEVER;
				networkSuspendReason = status.network_suspend_reason; // = 4 - SUSPEND_REASON_USER_REQ????
				networkParseError = false;
				return;
			}
			if((status.network_mode == BOINCDefs.RUN_MODE_AUTO) && (status.network_suspend_reason != BOINCDefs.SUSPEND_NOT_SUSPENDED)) {
				networkStatus = NETWORK_STATUS_SUSPENDED;
				networkSuspendReason = status.network_suspend_reason;
				networkParseError = false;
				return;
			}
			if((status.network_mode == BOINCDefs.RUN_MODE_AUTO) && (status.network_suspend_reason == BOINCDefs.SUSPEND_NOT_SUSPENDED)) {
				networkStatus = NETWORK_STATUS_AVAILABLE;
				networkSuspendReason = status.network_suspend_reason; // = 0 - SUSPEND_NOT_SUSPENDED
				networkParseError = false;
				return;
			}
		} catch (Exception e) {
			if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "ClientStatus parseNetworkStatus - Exception", e);
			if(Logging.DEBUG) Log.d(Logging.TAG, "ClientStatus error - client network status");
		}
	}

	// helper method for loading images from file
	// reads the symbolic link provided in pathOfSoftLink file
	// and returns absolute path to an image file.
	private String parseSoftLinkToAbsPath(String pathOfSoftLink, String projectDir){
		//if(Logging.DEBUG) Log.d(Logging.TAG,"parseSoftLinkToAbsPath() for path: " + pathOfSoftLink);
		// setup file
		File softLink = new File(pathOfSoftLink);
		if (!softLink.exists()) return null; // return if file does not exist
		
		// reading text of symbolic link
		String softLinkContent = "";
		try {
			FileInputStream stream = new FileInputStream(softLink);
			try {
				FileChannel fc = stream.getChannel();
			    MappedByteBuffer bb = fc.map(FileChannel.MapMode.READ_ONLY, 0, fc.size());
			    /* Instead of using default, pass in a decoder. */
			    softLinkContent =  Charset.defaultCharset().decode(bb).toString();
			} catch (IOException e) {if(Logging.WARNING) Log.w(Logging.TAG,"IOException in parseIconFileName()",e);}
			finally {
				stream.close();
			}
		} catch (Exception e) {
			// probably FileNotFoundException
			// if(Logging.DEBUG) Log.d(Logging.TAG,"Exception in parseSoftLinkToAbsPath() " + e.getMessage());
			return null;
		}
		//if(Logging.DEBUG) Log.d(Logging.TAG,"parseSoftLinkToAbsPath() softLinkContent: " + softLinkContent);
		
		// matching relevant path of String
		// matching 1+ word characters and 0 or 1 dot . and 0+ word characters
		// e.g. "icon.png", "icon", "icon.bmp"
		Pattern statIconPattern = Pattern.compile("/(\\w+?\\.?\\w*?)</soft_link>");
		Matcher m = statIconPattern.matcher(softLinkContent);
		if(!m.find()) {
			if(Logging.WARNING) Log.w(Logging.TAG,"parseSoftLinkToAbsPath() could not match pattern in soft link file: " + pathOfSoftLink);
			return null;
		}
		String fileName = m.group(1);
		//if(Logging.DEBUG) Log.d(Logging.TAG, "parseSoftLinkToAbsPath() fileName: " + fileName);
		
		return projectDir + "/" + fileName;
	}
	
	// Wrapper for slideshow images
	public class ImageWrapper {
		public Bitmap image;
		public String projectName;
		public String path;
		public ImageWrapper(Bitmap image, String projectName, String path) {
			this.image = image;
			this.projectName = projectName;
			this.path = path;
		}
	}
}
