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

#ifndef _RESULT_STATE_
#define _RESULT_STATE_

// States of a result on a client.
// THESE MUST BE IN NUMERICAL ORDER
// (because of the >= comparison in current_work_buf_days())
//
#define RESULT_NEW               0
    // New result
#define RESULT_FILES_DOWNLOADING 1
    // Input files for result (WU, app version) are being downloaded
#define RESULT_FILES_DOWNLOADED  2
    // Files are downloaded, result can be (or is being) computed
#define RESULT_COMPUTE_ERROR     3
    // computation failed; no file upload
#define RESULT_FILES_UPLOADING   4
    // Output files for result are being uploaded
#define RESULT_FILES_UPLOADED    5
    // Files are uploaded, notify scheduling server at some point

#endif

