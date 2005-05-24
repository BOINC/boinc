// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _RESULT_STATE_
#define _RESULT_STATE_

// States of a result on a client.
// THESE MUST BE IN NUMERICAL ORDER
// (because of the > comparison in RESULT::computing_done())
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

