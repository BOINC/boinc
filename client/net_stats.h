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

// keep track of the network performance of this host,
// namely exponentially weighted averages of upload and download speeds
//

#include <stdio.h>

class NET_STATS {
    int last_update_up;
    int last_update_down;
public:
    double bwup;
    double bwdown;

    NET_STATS();
    void update(bool is_upload, double nbytes, double nsecs);
        // report a file transfer
    
    int write(FILE*, bool to_server);
    int parse(FILE*);
};
