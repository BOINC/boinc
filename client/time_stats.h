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

#include "miofile.h"

class TIME_STATS {
    int last_update;
    bool first;
public:
// we maintain an exponentially weighted average of these quantities:
    double on_frac;
        // the fraction of total time this host runs the core client
    double connected_frac;
        // the fraction of total time the host is connected to the Internet
    double active_frac;
        // the fraction of total time the core client is able to work
        // (due to preferences, manual suspend/resume, etc.)

    void update(bool is_connected, bool is_active);

    TIME_STATS();
    int write(MIOFILE&, bool to_server);
    int parse(MIOFILE&);
};
