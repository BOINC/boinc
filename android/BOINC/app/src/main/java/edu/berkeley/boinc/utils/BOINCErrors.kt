/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2025 University of California
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

/*
 * This tries to be the same as lib/error_numbers.h
 */
package edu.berkeley.boinc.utils

// Function return values.
// NOTE:  add new errors to the end of the list and don't change
// old error numbers to avoid confusion between versions.
// Add a text description of your error to boincerror() in util.C.
//
const val ERR_OK = 0
const val ERR_CONNECT = -107 // connection problems
const val ERR_GETHOSTBYNAME = -113

// can not resolve name. no DNS -> no Internet?!
const val ERR_DB_NOT_FOUND = -136 // e.g. email invalid
const val ERR_DB_NOT_UNIQUE = -137 // name not unique, i.e. email already in use
const val ERR_PROJECT_DOWN = -183 // i.e. project error
const val ERR_HTTP_TRANSIENT = -184 // connection problems
const val ERR_BAD_USER_NAME = -188 // i.e. user name required
const val ERR_INVALID_URL = -189
const val ERR_RETRY = -199 // i.e. client currently busy with another GUI HTTP request
const val ERR_IN_PROGRESS = -204
const val ERR_BAD_EMAIL_ADDR = -205 // i.e. email has invalid syntax
const val ERR_BAD_PASSWD = -206
const val ERR_NONUNIQUE_EMAIL = -207
const val ERR_ACCT_CREATION_DISABLED = -208 // i.e. account creation currently disabled
