// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#ifndef _ERROR_NUMBERS_H_
#define _ERROR_NUMBERS_H_

// The following are process exit codes (not function return values).
// They are used by the core client and apps.
// NOTE: Unix passes only 8 bits of exit code,
// and Unix uses roughly 1-137, so let's start at 192

#define EXIT_STATEFILE_WRITE        192
    // client exit when can't write statefile
#define EXIT_SIGNAL                 193
    // app was killed by signal
#define EXIT_ABORTED_BY_CLIENT      194
#define EXIT_CHILD_FAILED           195
#define EXIT_DISK_LIMIT_EXCEEDED    196
#define EXIT_TIME_LIMIT_EXCEEDED    197
#define EXIT_MEM_LIMIT_EXCEEDED     198
#define EXIT_CLIENT_EXITING         199
#define EXIT_UNSTARTED_LATE         200
#define EXIT_MISSING_COPROC         201
#define EXIT_ABORTED_BY_PROJECT     202
#define EXIT_ABORTED_VIA_GUI        203
#define EXIT_UNKNOWN                204

// Function return values.
// NOTE:  add new errors to the end of the list and don't change
// old error numbers to avoid confusion between versions.
// Add a text description of your error to boincerror() in util.C.
//
#define BOINC_SUCCESS       0
#define ERR_SELECT          -100
#define ERR_MALLOC          -101
#define ERR_READ            -102
#define ERR_WRITE           -103
#define ERR_FREAD           -104
#define ERR_FWRITE          -105
#define ERR_IO              -106
#define ERR_CONNECT         -107
#define ERR_FOPEN           -108
#define ERR_RENAME          -109
#define ERR_UNLINK          -110
#define ERR_OPENDIR         -111
#define ERR_XML_PARSE       -112
    // Unexpected XML tag or XML format
#define ERR_GETHOSTBYNAME   -113
    // Couldn't resolve hostname
#define ERR_GIVEUP_DOWNLOAD -114
    // too much time has elapsed without progress on file xfer,
    // or we got a 404 (not found) status code from server
#define ERR_GIVEUP_UPLOAD   -115
#define ERR_NULL            -116
    // unexpected NULL pointer
#define ERR_NEG             -117
    // unexpected negative value
#define ERR_BUFFER_OVERFLOW -118
    // caught buffer overflow
#define ERR_MD5_FAILED      -119
    // MD5 checksum failed for a file
#define ERR_RSA_FAILED      -120
    // RSA key check failed for a file
#define ERR_OPEN            -121
#define ERR_DUP2            -122
#define ERR_NO_SIGNATURE    -123
#define ERR_THREAD          -124
    // Error creating a thread
#define ERR_SIGNAL_CATCH    -125
#define ERR_UPLOAD_TRANSIENT    -127
#define ERR_UPLOAD_PERMANENT    -128
#define ERR_IDLE_PERIOD     -129
    // can't start work because of user prefs
#define ERR_ALREADY_ATTACHED    -130
#define ERR_FILE_TOO_BIG    -131
    // an output file was bigger than max_nbytes
#define ERR_GETRUSAGE       -132
    // getrusage failed
#define ERR_BENCHMARK_FAILED -133
#define ERR_BAD_HEX_FORMAT  -134
    // hex-format key data is bad
#define ERR_DB_NOT_FOUND    -136
    // no rows found in lookup() or enumerate()
#define ERR_DB_NOT_UNIQUE   -137
    // not unique in lookup()
#define ERR_DB_CANT_CONNECT -138
#define ERR_GETS            -139
    // gets() or fgets()
#define ERR_SCANF           -140
    // scanf() or fscanf()
#define ERR_READDIR         -143
#define ERR_SHMGET          -144
#define ERR_SHMCTL          -145
#define ERR_SHMAT           -146
#define ERR_FORK            -147
#define ERR_EXEC            -148
#define ERR_NOT_EXITED      -149
    // a process didn't exit that was supposed to
#define ERR_NOT_IMPLEMENTED -150
    // a system call not implemented on this platform
#define ERR_GETHOSTNAME     -151
#define ERR_NETOPEN         -152
#define ERR_SOCKET          -153
#define ERR_FCNTL           -154
#define ERR_AUTHENTICATOR   -155
    // scheduler request host ID doesn't match authenticator
#define ERR_SCHED_SHMEM     -156
    // sched shmem has bad contents
#define ERR_ASYNCSELECT     -157
#define ERR_BAD_RESULT_STATE    -158
#define ERR_DB_CANT_INIT    -159
#define ERR_NOT_UNIQUE       -160
    // state files had redundant entries
#define ERR_NOT_FOUND        -161
    // inconsistent client state
#define ERR_NO_EXIT_STATUS   -162
    // exit_status not found in scheduler request 
#define ERR_FILE_MISSING    -163
#define ERR_SEMGET          -165
#define ERR_SEMCTL          -166
#define ERR_SEMOP           -167
#define ERR_FTOK            -168
#define ERR_SOCKS_UNKNOWN_FAILURE -169
#define ERR_SOCKS_REQUEST_FAILED  -170
#define ERR_SOCKS_BAD_USER_PASS   -171
#define ERR_SOCKS_UNKNOWN_SERVER_VERSION -172
#define ERR_SOCKS_UNSUPPORTED     -173
#define ERR_SOCKS_CANT_REACH_HOST -174
#define ERR_SOCKS_CONN_REFUSED    -175
#define ERR_TIMER_INIT            -176
#define ERR_INVALID_PARAM   -178
#define ERR_SIGNAL_OP       -179
#define ERR_BIND            -180
#define ERR_LISTEN          -181
#define ERR_TIMEOUT         -182
#define ERR_PROJECT_DOWN    -183
#define ERR_HTTP_TRANSIENT  -184
    // HTTP errors other than 404 and 416
#define ERR_RESULT_START    -185
#define ERR_RESULT_DOWNLOAD -186
#define ERR_RESULT_UPLOAD   -187
#define ERR_BAD_USER_NAME   -188
#define ERR_INVALID_URL     -189
#define ERR_MAJOR_VERSION   -190
#define ERR_NO_OPTION       -191
#define ERR_MKDIR           -192
#define ERR_INVALID_EVENT   -193
#define ERR_ALREADY_RUNNING -194
#define ERR_NO_APP_VERSION  -195
#define ERR_WU_USER_RULE    -196
#define ERR_ABORTED_VIA_GUI -197
#define ERR_INSUFFICIENT_RESOURCE   -198
#define ERR_RETRY           -199
#define ERR_WRONG_SIZE      -200
#define ERR_USER_PERMISSION -201
    // e.g. user didn't allow network connection
#define ERR_SHMEM_NAME      -202
#define ERR_NO_NETWORK_CONNECTION -203
#define ERR_IN_PROGRESS     -204
#define ERR_BAD_EMAIL_ADDR  -205
#define ERR_BAD_PASSWD      -206
#define ERR_NONUNIQUE_EMAIL -207
#define ERR_ACCT_CREATION_DISABLED  -208
#define ERR_ATTACH_FAIL_INIT       -209
#define ERR_ATTACH_FAIL_DOWNLOAD   -210
#define ERR_ATTACH_FAIL_PARSE      -211
#define ERR_ATTACH_FAIL_BAD_KEY    -212
#define ERR_ATTACH_FAIL_FILE_WRITE -213
#define ERR_ATTACH_FAIL_SERVER_ERROR -214
#define ERR_SIGNING_KEY             -215
#define ERR_FFLUSH          -216 
#define ERR_FSYNC           -217
#define ERR_TRUNCATE        -218
#define ERR_WRONG_URL       -219
#define ERR_DUP_NAME        -220
#define ERR_GETGRNAM        -222
#define ERR_CHOWN           -223
#define ERR_HTTP_PERMANENT  -224
    // represents HTTP 404 or 416 error
#define ERR_BAD_FILENAME    -225
#define ERR_TOO_MANY_EXITS  -226
#define ERR_RMDIR           -227
#define ERR_SYMLINK         -229
#define ERR_DB_CONN_LOST    -230
#define ERR_CRYPTO          -231
#define ERR_ABORTED_ON_EXIT -232
#define ERR_PROC_PARSE      -235

// PLEASE: add a text description of your error to 
// the text description function boincerror() in str_util.cpp.

// returns short text description of error corresponding to
// int errornumber from error_numbers.h
//
extern const char* boincerror(int which_error);

#endif
