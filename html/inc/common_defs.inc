<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// values for various database fields

// see db/boinc_db.h
//
define('BATCH_STATE_INIT', 0);
define('BATCH_STATE_IN_PROGRESS', 1);
define('BATCH_STATE_COMPLETE', 2);
define('BATCH_STATE_ABORTED', 3);
define('BATCH_STATE_RETIRED', 4);

define('ANON_PLATFORM_UNKNOWN', -1);
define('ANON_PLATFORM_CPU',     -2);
define('ANON_PLATFORM_NVIDIA',  -3);
define('ANON_PLATFORM_ATI',     -4);
define('ANON_PLATFORM_INTEL',   -5);

define('RESULT_SERVER_STATE_UNSENT',         2);
define('RESULT_SERVER_STATE_IN_PROGRESS',    4);
define('RESULT_SERVER_STATE_OVER',           5);

define('RESULT_OUTCOME_INIT',             0);
define('RESULT_OUTCOME_SUCCESS',          1);
define('RESULT_OUTCOME_COULDNT_SEND',     2);
define('RESULT_OUTCOME_CLIENT_ERROR',     3);
define('RESULT_OUTCOME_NO_REPLY',         4);
define('RESULT_OUTCOME_DIDNT_NEED',       5);
define('RESULT_OUTCOME_VALIDATE_ERROR',   6);
define('RESULT_OUTCOME_CLIENT_DETACHED',  7);

define('VALIDATE_STATE_INIT',        0);
define('VALIDATE_STATE_VALID',       1);
define('VALIDATE_STATE_INVALID',     2);
define('VALIDATE_STATE_NO_CHECK',    3);
define('VALIDATE_STATE_INCONCLUSIVE',4);
define('VALIDATE_STATE_TOO_LATE',    5);

// from lib/common_defs.h
//
define('RESULT_NEW',                 0);
define('RESULT_FILES_DOWNLOADING',   1);
define('RESULT_FILES_DOWNLOADED',    2);
define('RESULT_COMPUTE_ERROR',       3);
define('RESULT_FILES_UPLOADING',     4);
define('RESULT_FILES_UPLOADED',      5);
define('RESULT_ABORTED',             6);
define('RESULT_UPLOAD_FAILED',       7);

?>
