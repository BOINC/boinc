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
define('ANON_PLATFORM_INTEL_GPU',   -5);
define('ANON_PLATFORM_APPLE_GPU',   -6);

define('RESULT_SERVER_STATE_INACTIVE',       1);
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

define('WU_ERROR_COULDNT_SEND_RESULT',           1);
define('WU_ERROR_TOO_MANY_ERROR_RESULTS',        2);
define('WU_ERROR_TOO_MANY_SUCCESS_RESULTS',      4);
define('WU_ERROR_TOO_MANY_TOTAL_RESULTS',        8);
define('WU_ERROR_CANCELLED',                     16);
define('WU_ERROR_NO_CANONICAL_RESULT',           32);

define('FILE_DELETE_INIT',       0);
define('FILE_DELETE_READY',      1);
define('FILE_DELETE_DONE',       2);
define('FILE_DELETE_ERROR',      3);

define('ASSIMILATE_INIT',       0);
define('ASSIMILATE_READY',      1);
define('ASSIMILATE_DONE',       2);

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

// from lib/error_numbers.h
// returned by some web RPCs
//
define('ERR_XML_PARSE',               -112);
define('ERR_DB_NOT_FOUND',            -136);
define('ERR_DB_NOT_UNIQUE',           -137);
define('ERR_DB_CANT_CONNECT',         -138);
define('ERR_PROJECT_DOWN',            -183);
define('ERR_BAD_USER_NAME',           -188);
define('ERR_NO_OPTION',               -191);
define('ERR_BAD_EMAIL_ADDR',          -205);
define('ERR_BAD_PASSWD',              -206);
define('ERR_ACCT_CREATION_DISABLED',  -208);
define('ERR_ATTACH_FAIL_INIT',        -209);
define('ERR_ATTACH_FAIL_DOWNLOAD',    -210);
define('ERR_ACCT_REQUIRE_CONSENT',    -242);
?>
