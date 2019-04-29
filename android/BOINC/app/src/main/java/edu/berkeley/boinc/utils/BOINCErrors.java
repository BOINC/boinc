/*
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
 */
package edu.berkeley.boinc.utils;

/*
 * This tries to be the same as lib/error_numbers.h
 */

public class BOINCErrors {

    // Function return values.
    // NOTE:  add new errors to the end of the list and don't change
    // old error numbers to avoid confusion between versions.
    // Add a text description of your error to boincerror() in util.C.
    //
    public final static int ERR_OK = 0;
    public final static int ERR_CONNECT = -107; // connection problems
    public final static int ERR_XML_PARSE = -112; // XML parsing error on client side
    public final static int ERR_GETHOSTBYNAME = -113; // can not resolve name. no DNS -> no Internet?!
    public static final int ERR_GIVEUP_DOWNLOAD = -114;
    public static final int ERR_GIVEUP_UPLOAD = -115;
    public final static int ERR_DB_NOT_FOUND = -136; // e.g. eMail invalid
    public final static int ERR_DB_NOT_UNIQUE = -137; // name not unique, i.e. email already in use
    public final static int ERR_PROJECT_DOWN = -183; // i.e. project error
    public final static int ERR_HTTP_TRANSIENT = -184; // connection problems
    public final static int ERR_BAD_USER_NAME = -188; // i.e. user name required
    public final static int ERR_INVALID_URL = -189;
    public final static int ERR_RETRY = -199; // i.e. client currently busy with another GUI HTTP request
    public final static int ERR_IN_PROGRESS = -204;
    public final static int ERR_BAD_EMAIL_ADDR = -205; // i.e. email has invalid syntax
    public final static int ERR_BAD_PASSWD = -206;
    public final static int ERR_NONUNIQUE_EMAIL = -207;
    public final static int ERR_ACCT_CREATION_DISABLED = -208; // i.e. account creation currently disabled
    public final static int ERR_ACCT_REQUIRE_CONSENT = -242; // project requires consent
}
