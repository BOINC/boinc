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

package edu.berkeley.boinc.rpc;

import java.io.Serializable;
import java.util.ArrayList;

// needs to be serializable to be put into Activity start Intent
public class ProjectInfo implements Serializable{
	private static final long serialVersionUID = -5944047529950035455L; // auto generated
	public String name = "";
	public String url = "";
	public String generalArea;
	public String specificArea;
	public String description;
	public String home;
	public ArrayList<String> platforms;
	public String imageUrl;
	
	@Override
	public String toString() {
		String platformString = "";
		for (String platform : platforms) { platformString = platformString + platform + "/";}
		return "ProjectInfo: " + name + " ; " + url + " ; " + generalArea + " ; " + specificArea + " ; " + description + " ; " + home + " ; " + platformString + " ; " + imageUrl;
	}
}
