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

#include <stdio.h>

#include "types.h"
#include "error_numbers.h"
#include "parse.h"
#include "scheduler_reply.h"

int SCHEDULER_REPLY::parse(FILE* in) {
    char buf[256];

    strcpy(message, "");
    strcpy(message_priority, "");
    request_delay = 0;
    hostid = 0;

    fgets(buf, 256, in);
    if (!match_tag(buf, "<scheduler_reply>")) {
        fprintf(stderr, "SCHEDULER_REPLY::parse(): bad first tag %s\n", buf);
        return ERR_XML_PARSE;
    }
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</scheduler_reply>")) {
            return 0;
        } else if (parse_int(buf, "<hostid>", hostid)) {
            continue;
        } else if (parse_int(buf, "<request_delay>", request_delay)) {
            continue;
        } else if (match_tag(buf, "<prefs>")) {
            prefs.parse(in);
        } else if (match_tag(buf, "<app>")) {
            APP app;
            app.parse(in);
            apps.push_back(app);
        } else if (match_tag(buf, "<file_info>")) {
            FILE_INFO file_info;
            file_info.parse(in);
            file_infos.push_back(file_info);
        } else if (match_tag(buf, "<app_version>")) {
            APP_VERSION av;
            av.parse(in);
            app_versions.push_back(av);
        } else if (match_tag(buf, "<workunit>")) {
            WORKUNIT wu;
            wu.parse(in);
            workunits.push_back(wu);
        } else if (match_tag(buf, "<result>")) {
            RESULT result;      // make sure this is here so constructor
                                // gets called each time
            result.parse(in, "</result>");
            results.push_back(result);
        } else if (match_tag(buf, "<result_ack>")) {
            RESULT result;
            result.parse(in, "</result_ack>");
            result_acks.push_back(result);
        } else if (parse_str(buf, "<message", message)) {
            parse_attr(buf, "priority", message_priority);
            continue;
        } else {
            fprintf(stderr, "SCHEDULER_REPLY::parse: unrecognized %s\n", buf);
        }
    }
    fprintf(stderr, "SCHEDULER_REPLY::parse: no close tag\n");
    return ERR_XML_PARSE;
}
