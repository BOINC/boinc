// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

// NOTE:  add new errors to the end of the list and don't change
// old error numbers to avoid confusion between versions
//
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
    // too much time has elapsed without progress on file xfer
#define ERR_GIVEUP_UPLOAD   -115
#define ERR_NULL            -116
    // unexpected NULL pointer
#define ERR_NEG             -117
    // unexpected negative value
#define ERR_BUFF_OVERFLOW   -118
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
#define ERR_QUIT_REQUEST    -126
    // The app exited due to user request and should be restarted later
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
