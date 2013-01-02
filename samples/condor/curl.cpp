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

int do_http_post(const char* url) {
    CURL *curl;
    CURLcode res;
     
    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;

    curl = curl_easy_init();
    if (!curl) {
        return -1;
    }

    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "sendfile",
        CURLFORM_FILE, "curl.cpp",
        CURLFORM_END
    );
    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "filename",
        CURLFORM_COPYCONTENTS, "curl.cpp",
        CURLFORM_END
    );
    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "sendfile2",
        CURLFORM_FILE, "boinc_gahp.cpp",
        CURLFORM_END
    );
    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "filename2",
        CURLFORM_COPYCONTENTS, "boinc_gahp.cpp",
        CURLFORM_END
    );
 
    headerlist = curl_slist_append(headerlist, "Expect:");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "BOINC Condor adapter");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    curl_slist_free_all(headerlist);
    return 0;
}

int main() {
    do_http_post("http://isaac.ssl.berkeley.edu/foobar.php");
}
