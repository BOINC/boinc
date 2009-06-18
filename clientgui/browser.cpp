// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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


#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <string>
#include <vector>
#include <time.h>
#endif

#include "error_numbers.h"
#include "mfile.h"
#include "miofile.h"
#include "str_util.h"
#include "browser.h"


//
// Utility Functions
//

#ifdef _WIN32

// Prototype for SHGetFolderPath() in shell32.dll.
typedef HRESULT (WINAPI *MYSHGETFOLDERPATH)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);

#endif

// retrieve the user's application data directory.
// Win  : C:\Documents and Settings\<username>\Application Data
// Unix: ~/
//
void get_home_dir_path( std::string& path ) {
#ifdef _WIN32
    CHAR                szBuffer[MAX_PATH];
    HMODULE             hShell32;
	MYSHGETFOLDERPATH   pfnMySHGetFolderPath = NULL;

    // Attempt to link to dynamic function if it exists
    hShell32 = LoadLibrary(_T("SHELL32.DLL"));
    if (NULL != hShell32) {
        pfnMySHGetFolderPath = (MYSHGETFOLDERPATH) GetProcAddress(hShell32, "SHGetFolderPathA");
    }

    if (NULL != pfnMySHGetFolderPath) {
		if (SUCCEEDED((pfnMySHGetFolderPath)(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer))) {
            path  = std::string(szBuffer);
            path += std::string("\\");
        }
    }

	// Free the dynamically linked to library
    if (NULL != hShell32) {
    	FreeLibrary(hShell32);
    }
#elif defined(__APPLE__)
    path = std::string(getenv("HOME") )+ std::string("/");
#else
    path = std::string("~/");
#endif
}

// retrieve the user's cookie directory.
// WinXP
//   C:\Documents and Settings\<username>\Cookies
// WinVista/Win7
//   C:\Documents and Settings\<username>\Application Data\Roaming\Microsoft\Windows\Cookies
//   C:\Documents and Settings\<username>\Application Data\Roaming\Microsoft\Windows\Cookies\Low
//
void get_internet_explorer_cookie_path( bool low_rights, std::string& path ) {
#ifdef _WIN32
    CHAR                szBuffer[MAX_PATH];
    HMODULE             hShell32;
	MYSHGETFOLDERPATH   pfnMySHGetFolderPath = NULL;

    // Attempt to link to dynamic function if it exists
    hShell32 = LoadLibrary(_T("SHELL32.DLL"));
    if (NULL != hShell32) {
        pfnMySHGetFolderPath = (MYSHGETFOLDERPATH) GetProcAddress(hShell32, "SHGetFolderPathA");
    }

    if (NULL != pfnMySHGetFolderPath) {
		if (SUCCEEDED((pfnMySHGetFolderPath)(NULL, CSIDL_COOKIES|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer))) {
            path  = std::string(szBuffer);
            path += std::string("\\");
            if (low_rights) {
                path += std::string("Low\\");
            }
        }
    }

	// Free the dynamically linked to library
    if (NULL != hShell32) {
    	FreeLibrary(hShell32);
    }
#endif
}


// retrieve the user's name.
//
void get_user_name( std::string& name ) {
#ifdef _WIN32
    CHAR  szBuffer[256];
    DWORD dwSize = sizeof(szBuffer);

    if (GetUserNameA((LPSTR)&szBuffer, &dwSize)) {
        name = szBuffer;
    }
#endif
}


// is the authenticator valid?
//
bool is_authenticator_valid(const std::string authenticator) {
    std::string tmp = authenticator;

    // Perform some basic validation of the suspect authenticator
    //

    // If the string is null then it is invalid.
    if (0 == tmp.length()) {
        return false;
    }

    // If the string contains non alpha numeric characters it is invalid.
    std::string::iterator it = tmp.begin();
    while (it != tmp.end()) {
        if (!isalpha(*it) && !isdigit(*it)) {
            return false;
        }
        it++;
    }

    return true;
}


// parse name value pairs based on INI file rules.
//
bool parse_name_value_pair(char* buf, std::string& name, std::string& value) {
    std::basic_string<char>::size_type i;
    std::string s;

    s = std::string(buf);
    i = s.find("=", 0);
    if ( i < s.npos ) {
        name = s.substr(0, i);
        value = s.substr(i + 1);
        strip_whitespace(name);
        strip_whitespace(value);
        return true;
    }
    return false;
}


// parse host name from url for Mozilla compatible browsers.
//
bool parse_hostname_mozilla_compatible(std::string& project_url, std::string& hostname) {
    std::basic_string<char>::size_type start;
    std::basic_string<char>::size_type end;

    start = project_url.find("//", 0) + 2;
    end   = project_url.find("/", start);

    hostname = project_url.substr(start, end - start);

    if (starts_with(hostname.c_str(), "www"))
        hostname.erase(0, 3);

    if (!hostname.empty())
        return true;

    return false;
}


// parse host name from url for Internet Explorer compatible browsers.
//
bool parse_hostname_ie_compatible(
    std::string& project_url, std::string& hostname, std::string& domainname
) {
    std::basic_string<char>::size_type start;
    std::basic_string<char>::size_type end;

    // Remove the protocol identifier
    start = project_url.find("//", 0) + 2;
    end   = project_url.find("/", start);

    hostname = project_url.substr(start, end - start);

    // Remove the hostname to extract the domain name
    start = 0;
    end   = hostname.find(".") + 1;

    domainname = hostname;
    domainname.erase(start, end);

    if (!hostname.empty() && !domainname.empty())
        return true;

    return false;
}


//
// Mozilla-Based Browser Support
//

class MOZILLA_PROFILE {
public:
    std::string name;
    std::string path;
    bool        is_relative;
    bool        is_default;

    MOZILLA_PROFILE();

    void clear();
    int parse(MIOFILE& in);
};


class MOZILLA_PROFILES {
public:
    std::vector<MOZILLA_PROFILE*> profiles;

    MOZILLA_PROFILES();

    void clear();
    int parse(MIOFILE& in);
};


class MOZILLA_COOKIE_SQL {
public:
    std::string host;
    std::string name;
    std::string value;

    MOZILLA_COOKIE_SQL();

    void clear();
};


MOZILLA_PROFILE::MOZILLA_PROFILE() {
    clear();
}


void MOZILLA_PROFILE::clear() {
    name.clear();
    path.clear();
    is_relative = false;
    is_default = false;
}


int MOZILLA_PROFILE::parse(MIOFILE& in) {
    char buf[512];
    std::string sn;
    std::string sv;

    while (in.fgets(buf, 512)) {
        if (starts_with(buf, "\n")) return 0;
        if (starts_with(buf, "\r\n")) return 0;
        if (!parse_name_value_pair(buf, sn, sv)) continue;

        if ("Name" == sn) name = sv;
        if ("Path" == sn) path = sv;
        if (("IsRelative" == sn) && ("1" == sv)) is_relative = true;
        if (("Default" == sn) && ("1" == sv)) is_default = true;
    }
    return ERR_FREAD;
}


MOZILLA_PROFILES::MOZILLA_PROFILES() {
    clear();
}


void MOZILLA_PROFILES::clear() {
    unsigned int i;
    for (i=0; i<profiles.size(); i++) {
        delete profiles[i];
    }
    profiles.clear();
}


int MOZILLA_PROFILES::parse(MIOFILE& in) {
    char buf[512];
    MOZILLA_PROFILE* p = NULL;

    while (in.fgets(buf, 512)) {
        if (starts_with(buf, "[Profile")) {
            p = new MOZILLA_PROFILE;
            if (!p->parse( in )) {
                profiles.push_back( p );
            }
        }
    }

    return 0;
}


MOZILLA_COOKIE_SQL::MOZILLA_COOKIE_SQL() {
    clear();
}


void MOZILLA_COOKIE_SQL::clear() {
    host.clear();
    name.clear();
    value.clear();
}


// search for the project specific cookie for mozilla based browsers.
//
// file format is:
// 
// host \t isDomain \t path \t secure \t expires \t name \t cookie
// 
// if this format isn't respected we move onto the next line in the file.
// isDomain is "TRUE" or "FALSE" (default to "FALSE")
// isSecure is "TRUE" or "FALSE" (default to "TRUE")
// expires is a PRInt64 integer
// note 1: cookie can contain tabs.
// note 2: cookies are written in order of lastAccessed time:
//         most-recently used come first; least-recently-used come last.
// 
bool find_site_cookie_mozilla_v2(
    MIOFILE& in, std::string& project_url, std::string& name, std::string& value
) {
    bool retval = false;
    char buf[2048];
    char host[256], domain[16], path[256], secure[16], cookie_name[256], cookie_value[256];
    long long expires;
    std::string hostname;


    strcpy(host, "");
    strcpy(domain, "");
    strcpy(path, "");
    strcpy(secure, "");
    strcpy(cookie_name, "");
    strcpy(cookie_value, "");
    expires = 0;


    // determine the project hostname using the project url
    parse_hostname_mozilla_compatible(project_url, hostname);

    // traverse cookie file
    while (in.fgets(buf, sizeof(buf))) {
        sscanf(
            buf,
#ifdef _WIN32
            "%255s\t%15s\t%255s\t%15s\t%I64d\t%255s\t%255s",
#else
            "%255s\t%15s\t%255s\t%15s\t%Ld\t%255s\t%255s",
#endif
            host, domain, path, secure, &expires, cookie_name, cookie_value
        );

        // is this a real cookie?
        // temporary cookie? these cookies do not trickle back up
        // to the jscript interface, so ignore them.
        if (starts_with(host, "#HttpOnly")) continue;

        // is this the right host?
        if (!strstr(host, hostname.c_str())) continue;

        // has the cookie expired?
        if (time(0) > expires) continue;

        // is this the right cookie?
        if (starts_with(cookie_name, name)) {
            value = cookie_value;
            retval = true;
        }
    }

    return retval;
}


// search for the project specific cookie for mozilla based browsers.
// SELECT host, name, value, expiry from moz_cookies WHERE name = '%s' AND host LIKE '%s'
//
static int find_site_cookie_mozilla_v3(
    void* cookie, int /* argc */, char **argv, char ** /* szColumnName */
) {
    MOZILLA_COOKIE_SQL* _cookie = (MOZILLA_COOKIE_SQL*)cookie;
    char host[256], cookie_name[256], cookie_value[256];
    long long expires;


    strcpy(host, "");
    strcpy(cookie_name, "");
    strcpy(cookie_value, "");
    expires = 0;

    sscanf( argv[0], "%255s", host );
    sscanf( argv[1], "%255s", cookie_name );
    sscanf( argv[2], "%255s", cookie_value );
    sscanf( argv[3],
#ifdef _WIN32
        "%I64d",
#else
        "%Ld",
#endif
        &expires
    );

    // is this a real cookie?
    // temporary cookie? these cookies do not trickle back up
    // to the jscript interface, so ignore them.
    if (starts_with(host, "#HttpOnly")) return 0;

    // is this the right host?
    if (!strstr(host, _cookie->host.c_str())) return 0;

    // has the cookie expired?
    if (time(0) > expires) return 0;

    // is this the right cookie?
    if (starts_with(cookie_name, _cookie->name)) {
        _cookie->value = cookie_value;
    }

    return 0;
}


// traverse the profiles and determine which profile to use.
// this should be compatible with firefox2, firefox3, seamonkey, and netscape.
//
bool get_firefox_profile_root( std::string& profile_root ) {
    bool retval = false;
    FILE* pf = NULL;
   	MIOFILE pmf;
    MOZILLA_PROFILES mps;
    MOZILLA_PROFILE* mp = NULL;
    std::string cookies_root;
    std::string tmp;
    unsigned int i = 0;
    unsigned int default_index = 0;

    get_home_dir_path( profile_root );
#ifdef _WIN32
    profile_root += std::string("Mozilla\\Firefox\\");
#elif defined(__APPLE__)
    profile_root += std::string("Library/Application Support/Firefox/");
#else
    profile_root += std::string(".mozilla/firefox/");
#endif

    // lets see if we can open the profiles configuration file
    tmp = profile_root + "profiles.ini";
    pf = fopen(tmp.c_str(), "r");

    // if profiles configuration file exists, parse it.
    if (pf) {
        pmf.init_file(pf);
        mps.parse(pmf);
    }

    // we need to know which cookie file to look at, so if only
    // one profile exists, use it.
    //
    // if more than one profile exists, look through all the
    // profiles until we find the default profile. even when the
    // user selects a different profile at startup the default
    // profile flag is changed at startup to the new profile.
    if (mps.profiles.size() == 0) {
        if (pf) fclose(pf);
        return retval;          // something is very wrong, don't
                                // risk a crash
    }

    if (mps.profiles.size() == 1) {
        default_index = 0;
    } else {
        for (i=0; i < mps.profiles.size(); i++) {
            if (mps.profiles[i]->is_default) {
                default_index = i;
                break;
            }
        }
    }

    // should the path be treated as an absolute path or a relative
    // path?
    mp = mps.profiles[default_index];
    if (mp->is_relative) {
        cookies_root = profile_root + mp->path + "/";
    } else {
        cookies_root = mp->path + "/";
    }

    profile_root = cookies_root;
    retval = true;

    // cleanup
    if (pf) fclose(pf);

    return retval;
}

// traverse the various cookies looking for the one we want
//
bool detect_cookie_mozilla_v2(
    std::string profile_root, std::string& project_url, std::string& name, std::string& value
) {
    bool retval = false;
    FILE* cf = NULL;
   	MIOFILE cmf;
    std::string tmp;

    // now we should open up the cookie file.
    tmp = profile_root + "cookies.txt";
    cf = fopen(tmp.c_str(), "r");

    // if the cookie file exists, lookup the projects 'Setup' cookie.
    if (cf) {
        cmf.init_file(cf);
        retval = find_site_cookie_mozilla_v2(
            cmf,
            project_url,
            name,
            value
        );
    }

    // cleanup
    if (cf) fclose(cf);

    return retval;
}

    
bool detect_cookie_mozilla_v3(
    std::string profile_root, std::string& project_url, std::string& name, std::string& value
) {
    bool        retval = false;
    std::string tmp;
    std::string hostname;
    char        query[1024];
    sqlite3*    db;
    char*       lpszSQLErrorMessage = NULL;
    int         rc;
    MOZILLA_COOKIE_SQL cookie;

#if defined(__APPLE__)
    // sqlite3 is not available on Mac OS 10.3.9
    if (sqlite3_open == NULL) return false;
#endif

    // determine the project hostname using the project url
    parse_hostname_mozilla_compatible(project_url, hostname);


    // now we should open up the cookie database.
    tmp = profile_root + "cookies.sqlite";
    rc = sqlite3_open(tmp.c_str(), &db);
    if ( rc ) {
        sqlite3_close(db);
        return false;
    }
    
    // construct SQL query to extract the desired cookie
    // SELECT host, name, value, expiry from moz_cookies WHERE name = '%s' AND host LIKE '%%%s'
    snprintf(query, sizeof(query),
        "SELECT host, name, value, expiry from moz_cookies WHERE name = '%s' AND host LIKE '%%%s'",
        name.c_str(),
        hostname.c_str()
    );

    // execute query
    rc = sqlite3_exec(db, query, find_site_cookie_mozilla_v3, &cookie, &lpszSQLErrorMessage);
    if ( rc != SQLITE_OK ){
        sqlite3_free(lpszSQLErrorMessage);
    }

    // cleanup
    sqlite3_close(db);

    if ( !cookie.value.empty() ) {
        value = cookie.value;
        retval = true;
    }

    return retval;
}

    
//
// Firefox Browser Support
//

bool detect_cookie_firefox_2(
    std::string& project_url, std::string& name, std::string& value
) {
    std::string profile_root;
    get_firefox_profile_root(profile_root);

    return detect_cookie_mozilla_v2(
        profile_root,
        project_url,
        name,
        value
    );
}


bool detect_cookie_firefox_3(
    std::string& project_url, std::string& name, std::string& value
) {
    std::string profile_root;
    get_firefox_profile_root(profile_root);

    return detect_cookie_mozilla_v3(
        profile_root,
        project_url,
        name,
        value
    );
}


#ifdef _WIN32
//
// Internet Explorer Browser Support
//

//
// Detect a cookie in Internet Explorer by using the InternetGetCookie API.
// Supports: 3.x, 4.x, 5.x, 6.x, 7.x (UAC Turned Off), and 8.x (UAC Turned Off).
//
bool detect_cookie_ie_supported(std::string& project_url, std::string& name, std::string& value)
{
    bool        bReturnValue = false;
    bool        bCheckDomainName = false;
    char        szCookieBuffer[2048];
    char*       pszCookieFragment = NULL;
    DWORD       dwSize = sizeof(szCookieBuffer)/sizeof(char);
    std::string strCookieFragment;
    std::string strCookieName;
    std::string strCookieValue;
    std::string hostname;
    std::string domainname;
    size_t      uiDelimeterLocation;


    // if we don't find the cookie at the exact project dns name, check one higher
    //   (i.e. www.worldcommunitygrid.org becomes worldcommunitygrid.org
    parse_hostname_ie_compatible(project_url, hostname, domainname);

    // InternetGetCookie expects them in URL format
    hostname = std::string("http://") + hostname + std::string("/");
    domainname = std::string("http://") + domainname + std::string("/");

    // First check to see if the desired cookie is assigned to the hostname.
    bReturnValue = InternetGetCookieA(hostname.c_str(), NULL, szCookieBuffer, &dwSize) == TRUE;
    if (!bReturnValue || (!strstr(szCookieBuffer, name.c_str()))) {
        bCheckDomainName = true;
    }

    // Next check if it was assigned to the domainname.
    if (bCheckDomainName) {
        bReturnValue = InternetGetCookieA(domainname.c_str(), NULL, szCookieBuffer, &dwSize) == TRUE;
        if (!bReturnValue || (!strstr(szCookieBuffer, name.c_str()))) {
            return false;
        }
    }

    // Format of cookie buffer:
    // 'cookie1=value1; cookie2=value2; cookie3=value3;
    //
    pszCookieFragment = strtok(szCookieBuffer, "; ");
    while(pszCookieFragment) {
        // Convert to a std::string so we can go to town
        strCookieFragment = pszCookieFragment;

        // Extract the name & value
        uiDelimeterLocation = strCookieFragment.find("=", 0);
        strCookieName = strCookieFragment.substr(0, uiDelimeterLocation);
        strCookieValue = strCookieFragment.substr(uiDelimeterLocation + 1);

        if (name == strCookieName) {
            // Now we found it!  Yea - auto attach!
            value = strCookieValue;
            bReturnValue = true;
        }

        pszCookieFragment = strtok(NULL, "; ");
    }

    return bReturnValue;
}

// Convert a FILETIME to an epoch time
//
time_t file_time_to_epoch( char* high, char* low ) {
    double dbl = 0.0;
    time_t total = 0;
    unsigned long _high = 0, _low = 0;

    _high = strtoul(high, NULL, 10);
    _low = strtoul(low, NULL, 10);

    dbl = ((double)_high)*(pow((double)2.0,32));
    dbl += (double)(_low);

    if ( dbl==0 ) {
    return 0;
    }

    dbl *= 1.0e-7;
    dbl -= 11644473600;

    total = (time_t)dbl;

    return total;
}


// Traverse the various cookies looking for the one we want, the cookie
//   format looks like the following:
//     Cookie name
//     Cookie value
//     Host/path for the web server setting the cookie
//     Flags
//     Expiration time (low)
//     Expiration time (high)
//     Creation time (low)
//     Creation time (high)
//     Record delimiter (*)
//
// NOTE: While cookies themselves can be 4k in size, valid BOINC cookies
//   are shorter.
//
bool find_site_cookie_ie(
    char* cookie_file, std::string& path, std::string& name, std::string& value
) {
    bool retval = false;
    FILE* cf = NULL;
   	MIOFILE cmf;
    char buf[256];
    char host[256], expiration_low[256], expiration_high[256], cookie_name[256], cookie_value[256];
    time_t expires;

    // now we should open up the cookie file.
    cf = fopen(cookie_file, "r");

    // if the cookie file exists, lookup the projects 'Setup' cookie.
    if (cf) {
        cmf.init_file(cf);
        while (!cmf.eof()) {

            // Extract one complete cookie
            cmf.fgets(cookie_name, sizeof(cookie_name));
            cmf.fgets(cookie_value, sizeof(cookie_value));
            cmf.fgets(host, sizeof(host));
            cmf.fgets(buf, sizeof(buf));
            cmf.fgets(expiration_low, sizeof(expiration_low));
            cmf.fgets(expiration_high, sizeof(expiration_high));
            cmf.fgets(buf, sizeof(buf));
            cmf.fgets(buf, sizeof(buf));
            cmf.fgets(buf, sizeof(buf));

            // Convert the expiration time from a FILETIME structure
            //   to Epoch time.
            expires = file_time_to_epoch((char*)&expiration_high, (char*)&expiration_low);

            // is this the right host?
            if (!strstr(host, path.c_str())) continue;

            // has the cookie expired?
            if (time(0) > expires) continue;

            // is this the right cookie?
            if (starts_with(cookie_name, name)) {
                value = cookie_value;
                retval = true;
                break;
            }
        }
    }

    // cleanup
    if (cf) fclose(cf);

    return retval;
}

    
//
// Detect a cookie in Internet Explorer by parsing cookie files.
// Supports: 7.x (UAC Turned On), and 8.x (UAC Turned On).
//
// Example cookie file names:
//   romw@boinc.berkeley[1].txt
//   romw@boinc.berkeley[2].txt
//   romw@boinc.berkeley[4].txt
//   romw@breitbart[1].txt
//
// Ideally we would parse the index.dat file which is treated as
//   an in memory index which we could then use to find the correct
//   file. However, the contents of the index.dat file is
//   undocumented.
//
bool detect_cookie_ie_unsupported(std::string& project_url, std::string& name, std::string& value) {
    char        buf[512], buf2[512];
    int         i;
    std::string cookie_path;
    std::string username;
    std::string hostname;
    std::string domainname;

    // Initialize variables
    i = 0;
    strcpy(buf, "");
    strcpy(buf2, "");

    // Determine which path to look for the cookie files in
    get_internet_explorer_cookie_path(true, cookie_path);

    // Get the users name
    get_user_name(username);

    // Get the host name and domain name from the url.
    parse_hostname_ie_compatible(project_url, hostname, domainname);

    // Strip the TLDs from both the hostname and domain name.
    hostname.erase(hostname.rfind("."));
    domainname.erase(domainname.rfind("."));

    // Loop through looking for valid cookie files, check for a
    //   host specific one first and then a domain one. If there
    //   are more than ten different cookie paths for an account
    //   manager or BOINC project there is a problem with
    //   the design.
    for ( i = 1; i <= 10; ++i ) {
        //
        // Construct the host cookie file name
        //

        // Original naming scheme
        snprintf(buf, sizeof(buf), "%s%s@%s[%d].txt", cookie_path.c_str(), username.c_str(), hostname.c_str(), i);
        if (find_site_cookie_ie((char*)&buf, hostname, name, value)) {
            break;
        }

        // IE 7.x or better
        string_substitute(buf, buf2, sizeof(buf2), " ", "_");
        if (find_site_cookie_ie((char*)&buf2, hostname, name, value)) {
            break;
        }


        //
        // Construct the domainname cookie file name
        //
        snprintf(buf, sizeof(buf), "%s%s@%s[%d].txt", cookie_path.c_str(), username.c_str(), domainname.c_str(), i);
        if (find_site_cookie_ie((char*)&buf, domainname, name, value)) {
            break;
        }

        // IE 7.x or better
        string_substitute(buf, buf2, sizeof(buf2), " ", "_");
        if (find_site_cookie_ie((char*)&buf2, domainname, name, value)) {
            break;
        }
    }

    if (!name.empty() && !value.empty()) {
        return true;
    }
    return false;
}


//
// Detect a cookie in Internet Explorer.
//
bool detect_cookie_ie(std::string& project_url, std::string& name, std::string& value)
{
    // Check using the supported method first
    if (detect_cookie_ie_supported( project_url, name, value )) return true;

    // If the supported method didn't return a hit we need to use the
    //   unsupported method for Windows Vista machines or better to account
    //   for UAC which stores browser cookies in a different directory
    //   which keeps the InternetGetCookie() API from detecting them.
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);

    if (osvi.dwMajorVersion >= 6) {
        if (detect_cookie_ie_unsupported( project_url, name, value )) return true;
    }
    return false;
}

#endif


//
// walk through the various browsers looking up the
// project cookies until the projects 'Setup' cookie is found.
//
// give preference to the default platform specific browers first before going
// to the platform independant browsers since most people don't switch from
// the default.
// 
bool detect_setup_authenticator(
    std::string& project_url, std::string& authenticator
) {
    bool retval = false;
    std::string strCookieSetup("Setup");

#ifdef _WIN32
    if (detect_cookie_ie(project_url, strCookieSetup, authenticator)) goto END;
#endif
#ifdef __APPLE__
    if (detect_cookie_safari(project_url, strCookieSetup, authenticator)) goto END;
#endif
    if (detect_cookie_firefox_3(project_url, strCookieSetup, authenticator)) goto END;
    if (detect_cookie_firefox_2(project_url, strCookieSetup, authenticator)) goto END;

END:
    if (is_authenticator_valid(authenticator)) {
        retval = true;
    }

    return retval;
}


//
// walk through the various browsers looking up the
// account manager cookies until the account manager's 'Login' and 'Password_Hash'
// cookies are found.
//
// give preference to the default platform specific browers first before going
// to the platform independant browsers since most people don't switch from
// the default.
// 
bool detect_account_manager_credentials(
    std::string& project_url, std::string& login, std::string& password_hash, std::string& return_url
) {
    bool retval = false;
    std::string strCookieLogon("Logon");
    std::string strCookiePasswordHash("PasswordHash");
    std::string strCookieReturnURL("ReturnURL");

#ifdef _WIN32
    if ( detect_cookie_ie(project_url, strCookieLogon, login) && 
         detect_cookie_ie(project_url, strCookiePasswordHash, password_hash)
    ){
        detect_cookie_ie(project_url, strCookieReturnURL, return_url);
        goto END;
    }
#endif
#ifdef __APPLE__
    if ( detect_cookie_safari(project_url, strCookieLogon, login) && 
         detect_cookie_safari(project_url, strCookiePasswordHash, password_hash)
    ){
        detect_cookie_safari(project_url, strCookieReturnURL, return_url);
        goto END;
    }
#endif
    if ( detect_cookie_firefox_3(project_url, strCookieLogon, login) && 
         detect_cookie_firefox_3(project_url, strCookiePasswordHash, password_hash) 
    ){
        detect_cookie_firefox_3(project_url, strCookieReturnURL, return_url);
        goto END;
    }

    if ( detect_cookie_firefox_2(project_url, strCookieLogon, login) && 
         detect_cookie_firefox_2(project_url, strCookiePasswordHash, password_hash)
    ){
        detect_cookie_firefox_2(project_url, strCookieReturnURL, return_url);
        goto END;
    }

END:
    if (!login.empty() && !password_hash.empty()) {
        retval = true;
    }

    return retval;
}

