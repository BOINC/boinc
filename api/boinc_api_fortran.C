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

// This file defines a Fortran wrapper to the BOINC API.

#include "boinc_api.h"

// helper class that makes a C-string from a character array and length,
// automatically deleted on destruction

// Fortran strings are passed as character array plus length
class StringFromFortran {
    char* p;
public:
    StringFromFortran(const char* s, int s_len) {
        p = new char[s_len + 1];
        memcpy(p, s, s_len);
        p[s_len] = 0;
    }
    ~StringFromFortran() { delete [] p; }
    operator char* () const { return p; }
};

extern "C" {

    int boinc_init_(int *standalone) {
        boinc_init(*standalone);
    }

    int boinc_finish_(int* status) {
        boinc_finish(*status);
    }

    int boinc_is_standalone_() {
        return boinc_is_standalone();
    }

    int boinc_resolve_filename_(const char* s, int s_len, char* t, int t_len)
    {
        boinc_resolve_filename(StringFromFortran(s), t, t_len);
    }

    int boinc_parse_init_data_file_()
    {
        return boinc_parse_init_data_file();
    }

    int boinc_write_init_data_file_()
    {
        return boinc_write_init_data_file_();
    }

// TODO: structs? common?
// extern int	boinc_get_init_data(APP_INIT_DATA&);

    int boinc_send_trickle_up_(char* s, int len)
    {
        return boinc_send_trickle_up(StringFromFortran(s,s_len));
    }

    int boinc_receive_trickle_down_(char* buf, int len)
    {
        return boinc_receive_trickle_down(buf, len);
    }

    int boinc_time_to_checkpoint_() {
        return boinc_time_to_checkpoint();
    }

    int boinc_checkpoint_completed_() {
        return boinc_checkpoint_completed();
    }

    int boinc_fraction_done_(double* d) {
        return boinc_fraction_done(* d);
    }

    int boinc_wu_cpu_time_(double* d_out) {
        return boinc_wu_cpu_time(*d_out);
    }

    int boinc_thread_cpu_time_(double* d1_out, double* d2_out)
    {
        return boinc_thread_cpu_time(*d1_out, *d2_out);
    }

}

