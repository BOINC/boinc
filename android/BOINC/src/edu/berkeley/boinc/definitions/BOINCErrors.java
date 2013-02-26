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
package edu.berkeley.boinc.definitions;

/*
 * This tries to be the same as lib/error_numbers.h
 */

public class BOINCErrors {

	// Function return values.
	// NOTE:  add new errors to the end of the list and don't change
	// old error numbers to avoid confusion between versions.
	// Add a text description of your error to boincerror() in util.C.
	//
	public static final int ERR_GIVEUP_DOWNLOAD                     = -114;
	public static final int ERR_GIVEUP_UPLOAD                       = -115;



}
