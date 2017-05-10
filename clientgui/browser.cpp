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

#ifdef _WIN32
#include "win_util.h"

#ifndef InternetGetCookie

#if defined(__cplusplus)
extern "C" {
#endif

BOOL WINAPI InternetGetCookieA( LPCSTR lpszUrl, LPCSTR lpszCookieName, LPSTR lpszCookieData, LPDWORD lpdwSize );

#if defined(__cplusplus)
}
#endif

#endif
#else
#include <string>
#include <vector>
#include <time.h>
#endif

#include <sqlite3.h>
#include "error_numbers.h"
#include "str_replace.h"
#include "mfile.h"
#include "miofile.h"
#include "str_util.h"
#include "filesys.h"
#include "browser.h"


//
// Utility Functions
//

// retrieve the user's application data directory.
// Win  : C:\Documents and Settings\<username>\Application Data
// Unix: ~/
//
// Google Chrome decided to put its application data in the 'local' application data
// store on Windows.  'local' only has meaning for Windows.
//
void get_home_dir_path( std::string& path, bool local = false ) {
#ifdef _WIN32
    CHAR szBuffer[MAX_PATH];
    int  iCSIDLFlags = CSIDL_FLAG_CREATE;

    if (local) {
        iCSIDLFlags |= CSIDL_LOCAL_APPDATA;
    } else {
        iCSIDLFlags |= CSIDL_APPDATA;
    }

	if (SUCCEEDED(SHGetFolderPathA(NULL, iCSIDLFlags, NULL, SHGFP_TYPE_CURRENT, szBuffer))) {
        path  = std::string(szBuffer);
        path += std::string("\\");
    }
#elif defined(__APPLE__)
    path = std::string(getenv("HOME")) + std::string("/");
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
    CHAR szBuffer[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COOKIES|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer))) {
        path  = std::string(szBuffer);
        path += std::string("\\");
        if (low_rights) {
            path += std::string("Low\\");
        }
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
        ++it;
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


// parse host name from url for Chrome compatible browsers.
//
bool parse_hostname_chrome_compatible(std::string& project_url, std::string& hostname) {
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
// Generic Browser Support
//

class COOKIE_SQL {
public:
    std::string host;
    std::string name;
    std::string value;

    COOKIE_SQL();

    void clear();
};


COOKIE_SQL::COOKIE_SQL() {
    clear();
}


void COOKIE_SQL::clear() {
    host.clear();
    name.clear();
    value.clear();
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
            } else {
                delete p;
            }
        }
    }

    return 0;
}


// search for the project specific cookie for mozilla based browsers.
// SELECT host, name, value, expiry from moz_cookies WHERE name = '%s' AND host LIKE '%s'
//
static int find_site_cookie_mozilla_v3(
    void* cookie, int /* argc */, char **argv, char ** /* szColumnName */
) {
    COOKIE_SQL* _cookie = (COOKIE_SQL*)cookie;
    char host[256], cookie_name[256], cookie_value[4096];
    long long expires;


    safe_strcpy(host, "");
    safe_strcpy(cookie_name, "");
    safe_strcpy(cookie_value, "");
    expires = 0;

    sscanf( argv[0], "%255s", host );
    sscanf( argv[1], "%255s", cookie_name );
    sscanf( argv[2], "%4095s", cookie_value );
    sscanf( argv[3],
#ifdef _WIN32
        "%I64d",
#elif defined(__APPLE__)
        "%lld",
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
// this should be compatible with firefox2, firefox3, firefox4, seamonkey, and netscape.
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
    COOKIE_SQL  cookie;


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
    sqlite3_snprintf(sizeof(query), query,
        "SELECT host, name, value, expiry from moz_cookies WHERE name = '%q' AND host LIKE '%%%q'",
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


//
// Chrome-Based Browser Support
//

// search for the project specific cookie for chrome based browsers.
// SELECT host_key, name, value, expires_utc, httponly from cookies WHERE name = '%s' AND host_key LIKE '%s'
//
static int find_site_cookie_chrome(
    void* cookie, int /* argc */, char **argv, char ** /* szColumnName */
) {
    COOKIE_SQL* _cookie = (COOKIE_SQL*)cookie;
    char host[256], cookie_name[256], cookie_value[4096];
    long long expires;
    long httponly;

    safe_strcpy(host, "");
    safe_strcpy(cookie_name, "");
    safe_strcpy(cookie_value, "");
    expires = 0;

    sscanf( argv[0], "%255s", host );
    sscanf( argv[1], "%255s", cookie_name );
    sscanf( argv[2], "%4095s", cookie_value );
    sscanf( argv[3],
#ifdef _WIN32
        "%I64d",
#elif defined(__APPLE__)
        "%lld",
#else
        "%Ld",
#endif
        &expires
    );
    sscanf( argv[4],
        "%ld",
        &httponly
    );

    // Convert Google Chrome time (microseconds since January 1, 1601) 
    // to UNIX time (seconds since January 1, 1970)
    expires = (expires / 1000000) - 11644473600LL;

    // is this a real cookie?
    // temporary cookie? these cookies do not trickle back up
    // to the jscript interface, so ignore them.
    if (httponly) return 0;

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
// this should be compatible with chrome.
//
bool get_chrome_profile_root( std::string& profile_root ) {
    get_home_dir_path( profile_root, true );

#ifdef _WIN32
    profile_root += std::string("Google\\Chrome\\User Data\\Default\\");
#elif defined(__APPLE__)
    profile_root += std::string("Library/Application Support/Google/Chrome/Default/");
#else
    profile_root += std::string(".google/chrome/");
#endif

    return true;
}


bool detect_cookie_chrome(
    std::string profile_root, std::string& project_url, std::string& name, std::string& value
) {
    bool        retval = false;
    std::string tmp;
    std::string hostname;
    char        query[1024];
    sqlite3*    db;
    char*       lpszSQLErrorMessage = NULL;
    int         rc;
    COOKIE_SQL  cookie;


    // determine the project hostname using the project url
    parse_hostname_chrome_compatible(project_url, hostname);


    // now we should open up the cookie database.
    tmp = profile_root + "Cookies";
    rc = sqlite3_open(tmp.c_str(), &db);
    if ( rc ) {
        sqlite3_close(db);
        tmp = profile_root + "Safe Browsing Cookies";
        rc = sqlite3_open(tmp.c_str(), &db);
        if ( rc ) {
            sqlite3_close(db);
            return false;
        }
    }
    
    // construct SQL query to extract the desired cookie
    // SELECT host_key, name, value, expires_utc, httponly from cookies WHERE name = '%s' AND host_key LIKE '%%%s'
    sqlite3_snprintf(sizeof(query), query,
        "SELECT host_key, name, value, expires_utc, httponly from cookies WHERE name = '%q' AND host_key LIKE '%%%q'",
        name.c_str(),
        hostname.c_str()
    );

    // execute query
    rc = sqlite3_exec(db, query, find_site_cookie_chrome, &cookie, &lpszSQLErrorMessage);
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


bool detect_cookie_chrome(
    std::string& project_url, std::string& name, std::string& value
) {
    std::string profile_root;
    get_chrome_profile_root(profile_root);

    return detect_cookie_chrome(
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
// Supports: 3.x, 4.x, 5.x, 6.x, 7.x (UAC Turned Off), 8.x (UAC Turned Off), 9.x (UAC Turned Off), and 
// 10.x (UAC Turned Off).
//
bool detect_cookie_ie_supported(std::string& project_url, std::string& name, std::string& value)
{
    bool        bReturnValue = false;
    bool        bCheckDomainName = false;
    char        szCookieBuffer[4096];
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

        if (0 == strcmp(name.c_str(), strCookieName.c_str())) {
            // Now we found it!  Yea - auto attach!
            value = strCookieValue;
            bReturnValue = true;
        }

        pszCookieFragment = strtok(NULL, "; ");
    }

    return bReturnValue;
}

//
// Detect a cookie in Internet Explorer by using the InternetGetCookie API.
// Supports: 8.x (UAC Turned On), 9.x (UAC Turned On), 10.x (UAC Turned On).
//
typedef HRESULT (__stdcall *tIEGPMC)( IN LPCWSTR, IN LPCWSTR, OUT LPWSTR, OUT DWORD*, IN DWORD );

bool detect_cookie_ie_supported_uac(std::string& project_url, std::string& name, std::string& value)
{
    static       HMODULE ieframelib = NULL;
    static       tIEGPMC pIEGPMC = NULL;
    bool         bReturnValue = false;
    bool         bCheckDomainName = false;
    HRESULT      rc;
    WCHAR        szCookieBuffer[4096];
    WCHAR*       pszCookieFragment = NULL;
    DWORD        dwSize = sizeof(szCookieBuffer)/sizeof(WCHAR);
    std::wstring strCookieFragment;
    std::wstring strCookieName;
    std::wstring strCookieValue;
    std::string  hostname;
    std::wstring hostname_w;
    std::string  domainname;
    std::wstring domainname_w;
    std::wstring name_w;
    size_t       uiDelimeterLocation;

    if (!ieframelib) {
        ieframelib = LoadLibraryA("ieframe.dll");
        if (ieframelib) {
            pIEGPMC = (tIEGPMC)GetProcAddress(ieframelib, "IEGetProtectedModeCookie");
        }
    }

    if (!pIEGPMC) {
        return false;
    }

    // Convert name into wide character string
    name_w = boinc_ascii_to_wide(name);

    // if we don't find the cookie at the exact project dns name, check one higher
    //   (i.e. www.worldcommunitygrid.org becomes worldcommunitygrid.org
    parse_hostname_ie_compatible(project_url, hostname, domainname);

    // InternetGetCookie expects them in URL format
    hostname_w = std::wstring(_T("http://")) + boinc_ascii_to_wide(hostname) + std::wstring(_T("/"));
    domainname_w = std::wstring(_T("http://")) + boinc_ascii_to_wide(domainname) + std::wstring(_T("/"));

    // First check to see if the desired cookie is assigned to the hostname.
    rc = pIEGPMC(hostname_w.c_str(), NULL, szCookieBuffer, &dwSize, NULL) == TRUE;
    if (!SUCCEEDED(rc) || (!wcsstr(szCookieBuffer, name_w.c_str()))) {
        bCheckDomainName = true;
    }

    // Next check if it was assigned to the domainname.
    if (bCheckDomainName) {
        rc = pIEGPMC(domainname_w.c_str(), NULL, szCookieBuffer, &dwSize, NULL) == TRUE;
        if (!SUCCEEDED(rc) || (!wcsstr(szCookieBuffer, name_w.c_str()))) {
            return false;
        }
    }

    // Format of cookie buffer:
    // 'cookie1=value1; cookie2=value2; cookie3=value3;
    //
    pszCookieFragment = wcstok(szCookieBuffer, _T("; "));
    while(pszCookieFragment) {
        // Convert to a std::string so we can go to town
        strCookieFragment = pszCookieFragment;

        // Extract the name & value
        uiDelimeterLocation = strCookieFragment.find(_T("="), 0);
        strCookieName = strCookieFragment.substr(0, uiDelimeterLocation);
        strCookieValue = strCookieFragment.substr(uiDelimeterLocation + 1);

        if (0 == wcscmp(name_w.c_str(), strCookieName.c_str())) {
            // Now we found it!  Yea - auto attach!
            value = boinc_wide_to_ascii(strCookieValue);
            bReturnValue = true;
        }

        pszCookieFragment = wcstok(NULL, _T("; "));
    }

    return bReturnValue;
}


//
// Detect a cookie in Internet Explorer.
//
bool detect_cookie_ie(std::string& project_url, std::string& name, std::string& value)
{
    // Check using the supported methods first
    if (detect_cookie_ie_supported( project_url, name, value )) return true;
    if (detect_cookie_ie_supported_uac( project_url, name, value )) return true;
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
    if (detect_cookie_chrome(project_url, strCookieSetup, authenticator)) goto END;
    if (detect_cookie_firefox_3(project_url, strCookieSetup, authenticator)) goto END;

END:
    if (is_authenticator_valid(authenticator)) {
        retval = true;
    }

    return retval;
}


//
// walk through the various browsers looking up the
// various cookies that make up the simple account creation scheme.
//
// give preference to the default platform specific browers first before going
// to the platform independant browsers since most people don't switch from
// the default.
// 
bool detect_simple_account_credentials(
    std::string& project_name, std::string& project_url, std::string& authenticator, 
    std::string& project_institution, std::string& project_description, std::string& known
) {
    bool retval = false;
    std::string strCookieServer("http://boinc.berkeley.edu");
    std::string strCookieProjectName("attach_project_name");
    std::string strCookieProjectURL("attach_master_url");
    std::string strCookieAuthenticator("attach_auth");
    std::string strCookieProjectInstitution("attach_project_inst");
    std::string strCookieProjectDescription("attach_project_desc");
    std::string strCookieKnown("attach_known");

#ifdef _WIN32
    if ( detect_cookie_ie(strCookieServer, strCookieProjectName, project_name) &&
         detect_cookie_ie(strCookieServer, strCookieProjectURL, project_url)
    ){
        detect_cookie_ie(strCookieServer, strCookieAuthenticator, authenticator);
        detect_cookie_ie(strCookieServer, strCookieProjectInstitution, project_institution);
        detect_cookie_ie(strCookieServer, strCookieProjectDescription, project_description);
        detect_cookie_ie(strCookieServer, strCookieKnown, known);
        goto END;
    }
#endif
#ifdef __APPLE__
    if ( detect_cookie_safari(strCookieServer, strCookieProjectName, project_name) &&
         detect_cookie_safari(strCookieServer, strCookieProjectURL, project_url)
    ){
        detect_cookie_safari(strCookieServer, strCookieAuthenticator, authenticator);
        detect_cookie_safari(strCookieServer, strCookieProjectInstitution, project_institution);
        detect_cookie_safari(strCookieServer, strCookieProjectDescription, project_description);
        detect_cookie_safari(strCookieServer, strCookieKnown, known);
        goto END;
    }
#endif
    if ( detect_cookie_chrome(strCookieServer, strCookieProjectName, project_name) &&
         detect_cookie_chrome(strCookieServer, strCookieProjectURL, project_url)
    ){
        detect_cookie_chrome(strCookieServer, strCookieAuthenticator, authenticator);
        detect_cookie_chrome(strCookieServer, strCookieProjectInstitution, project_institution);
        detect_cookie_chrome(strCookieServer, strCookieProjectDescription, project_description);
        detect_cookie_chrome(strCookieServer, strCookieKnown, known);
        goto END;
    }
    if ( detect_cookie_firefox_3(strCookieServer, strCookieProjectName, project_name) &&
         detect_cookie_firefox_3(strCookieServer, strCookieProjectURL, project_url)
    ){
        detect_cookie_firefox_3(strCookieServer, strCookieAuthenticator, authenticator);
        detect_cookie_firefox_3(strCookieServer, strCookieProjectInstitution, project_institution);
        detect_cookie_firefox_3(strCookieServer, strCookieProjectDescription, project_description);
        detect_cookie_firefox_3(strCookieServer, strCookieKnown, known);
        goto END;
    }
END:
    if (!project_name.empty() && !project_url.empty()) {
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
    if ( detect_cookie_chrome(project_url, strCookieLogon, login) && 
         detect_cookie_chrome(project_url, strCookiePasswordHash, password_hash) 
    ){
        detect_cookie_chrome(project_url, strCookieReturnURL, return_url);
        goto END;
    }

    if ( detect_cookie_firefox_3(project_url, strCookieLogon, login) && 
         detect_cookie_firefox_3(project_url, strCookiePasswordHash, password_hash) 
    ){
        detect_cookie_firefox_3(project_url, strCookieReturnURL, return_url);
        goto END;
    }

END:
    if (!login.empty() && !password_hash.empty()) {
        retval = true;
    }

    return retval;
}

