// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

// keep track of the fraction of time this host is
// "on" (i.e. the core client is running)
// "connected" (to the Internet)
// "active" (not suspended by user activity or prefs)
//
// We maintain an exponentially weighted mean of these quantities

class TIME_STATS {
    int last_update;
    bool first;
public:
    double on_frac;
    double connected_frac;
    double active_frac;

    void update(bool is_connected, bool is_active);

    TIME_STATS();
    int write(FILE*, bool to_server);
    int parse(FILE*);
};
