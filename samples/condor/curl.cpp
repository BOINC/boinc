// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

#include <curl/curl.h>
#include <stdio.h>
#include <vector>
#include <string>

#include "curl.h"

using std::vector;
using std::string;

// send an HTTP POST request,
// with an optional set of multi-part file attachments
//
int do_http_post(
    const char* url,
    const char* request,
    FILE* reply,
    vector<string> send_files
) {
    CURL *curl;
    CURLcode res;
    char buf[256];
     
    curl = curl_easy_init();
    if (!curl) {
        return -1;
    }

    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;

    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "request",
        CURLFORM_COPYCONTENTS, request,
        CURLFORM_END
    );
    for (unsigned int i=0; i<send_files.size(); i++) {
        sprintf(buf, "file_%d", i);
        string s = send_files[i];
        curl_formadd(&formpost, &lastptr,
            CURLFORM_COPYNAME, buf,
            CURLFORM_FILE, s.c_str(),
            CURLFORM_END
        );
    }
 
    headerlist = curl_slist_append(headerlist, "Expect:");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "BOINC Condor adapter");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_READDATA, request);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reply);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    curl_slist_free_all(headerlist);
    return 0;
}

#if 0
int main() {
    FILE* reply = fopen("reply", "w");
    vector<string> send_files;
    send_files.push_back("curl.cpp");
    send_files.push_back("boinc_gahp.cpp");
    do_http_post(
        "http://isaac.ssl.berkeley.edu/foobar.php",
        "<req>foo</req>",
        reply,
        send_files
    );
}
#endif
