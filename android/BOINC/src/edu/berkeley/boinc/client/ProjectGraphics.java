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

import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

// this class handles a BOINC project's images.
public class ProjectGraphics {
	private final String TAG = "ProjectGraphics";
	
	// absolute path of BOINC's /projects/PNAME directory
	private String projectDir;
	
	// 40 * 40 pixel project icon, symbolic link in /projects/PNAME/stat_icon
	private Bitmap icon;
	
	// list of mulitple 126 * 29 pixel project slideshow images
	// found in /projects/PNAME/slideshow_appname_n
	// not aware of BOINC application
	private ArrayList<Bitmap> slideshowImages = new ArrayList<Bitmap>();
	
	public ProjectGraphics(String projectDir) {
		this.projectDir = projectDir;
		loadIconFromFile();
		loadSlideshowImagesFromFile();
	}
	
	public Boolean isIconAvailable() {
		if (icon != null) return true;
		return false;
	}
	
	public Boolean isSlideshowAvailable() {
		if (slideshowImages != null && slideshowImages.size() > 0) return true;
		return false;
	}
	
	public Boolean realoadIcon() {
		loadIconFromFile();
		return isIconAvailable();
	}
	
	public Boolean reloadSlideshow() {
		loadSlideshowImagesFromFile();
		return isSlideshowAvailable();
	}
	
	public Bitmap getIcon() {
		return icon;
	}
	
	public ArrayList<Bitmap> getSlideshow() {
		return slideshowImages;
	}
	
	private void loadIconFromFile() {
		// read file name of icon
		String iconAbsPath = parseSoftLinkToAbsPath(projectDir + "/stat_icon");
		//Log.d(TAG, "loadIconFromFile() aboslute path to icon: " + iconAbsPath);
		
		icon = BitmapFactory.decodeFile(iconAbsPath);
		Log.d(TAG, "loadIconFromFile() successful: " + isIconAvailable());
	}
	
	private void loadSlideshowImagesFromFile() {
		// get files
		ArrayList<String> filePaths = parseSlideshowFileNames();
		//Log.d(TAG,"loadSlideshowImagesFromFile() retrieve number file paths: " + filePaths.size());
		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inDither = true;
		options.inSampleSize = 1;
		
		for (String filePath : filePaths) {
			Bitmap tmp = BitmapFactory.decodeFile(filePath, options);
			if(tmp!=null) slideshowImages.add(tmp);
			else Log.d(TAG,"loadSlideshowImagesFromFile(): null for path: " + filePath);
		}
		
		Log.d(TAG,"loadSlideshowImagesFromFile() loaded number of files: " + slideshowImages.size());
	}
	
	// returns a list of all absolute file paths in the project directory at
	// /projects/PNAME starting with slideshow_
	private ArrayList<String> parseSlideshowFileNames() {
		File dir = new File(projectDir);
		File[] foundFiles = dir.listFiles(new FilenameFilter() {
		    public boolean accept(File dir, String name) {
		        return name.startsWith("slideshow_");
		    }
		});
		ArrayList<String> filePaths = new ArrayList<String>();
		if(foundFiles == null) return filePaths; // prevent NPE
		for (File file: foundFiles) {
			String slideshowImagePath = parseSoftLinkToAbsPath(file.getAbsolutePath());
			//check whether path is not empty, and avoid duplicates (slideshow images can 
			//re-occur for multiple apps, since we do not distinct apps, skip duplicates.
			if(!slideshowImagePath.isEmpty() && !filePaths.contains(slideshowImagePath)) filePaths.add(slideshowImagePath);
			//Log.d(TAG, "parseSlideshowFileNames() path: " + slideshowImagePath);
		}
		return filePaths;
	}
	
	// reads the symbolic link provided in pathOfSoftLink file
	// and returns absolute path to image file.
	private String parseSoftLinkToAbsPath(String pathOfSoftLink){
		//Log.d(TAG,"parseSoftLinkToAbsPath() for path: " + pathOfSoftLink);
		
		// reading text of symbolic link
		String softLinkContent = "";
		try {
			FileInputStream stream = new FileInputStream(new File(pathOfSoftLink));
			try {
				FileChannel fc = stream.getChannel();
			    MappedByteBuffer bb = fc.map(FileChannel.MapMode.READ_ONLY, 0, fc.size());
			    /* Instead of using default, pass in a decoder. */
			    softLinkContent =  Charset.defaultCharset().decode(bb).toString();
			} catch (IOException e) {Log.w(TAG,"IOException in parseIconFileName()",e);}
			finally {
				stream.close();
			}
		} catch (Exception e) {Log.w(TAG,"Exception in parseSoftLinkToAbsPath()",e);}
		//Log.d(TAG,"parseSoftLinkToAbsPath() softLinkContent: " + softLinkContent);
		
		// matching relevant path of String
		// matching 1+ word characters and 0 or 1 dot . and 0+ word characters
		// e.g. "icon.png", "icon", "icon.bmp"
		Pattern statIconPattern = Pattern.compile("/(\\w+?\\.?\\w*?)</soft_link>");
		Matcher m = statIconPattern.matcher(softLinkContent);
		if(!m.find()) {
			Log.w(TAG,"parseSoftLinkToAbsPath() could not match pattern in soft link!");
			return null;
		}
		String fileName = m.group(1);
		//Log.d(TAG, "parseSoftLinkToAbsPath() fileName: " + fileName);
		
		return projectDir + "/" + fileName; // CAUTION: breaks if icon is in sub-directory!
	}
}
