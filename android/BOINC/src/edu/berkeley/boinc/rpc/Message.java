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

public class Message {
	public static final int MSG_INFO =1;
	public static final int MSG_USER_ALERT = 2;
	public static final int MSG_INTERNAL_ERROR = 3;
	// internally used by client
	public static final int MSG_SCHEDULER_ALERT = 4;
	
	public String project = "";
	public int    priority;
	public int    seqno;
	public long   timestamp;
	public String body;
}
