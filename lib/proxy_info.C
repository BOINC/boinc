#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "parse.h"
#include "proxy_info.h"

int PROXY_INFO::parse(MIOFILE& in) {
    char buf[256];

    memset(this, 0, sizeof(PROXY_INFO));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</proxy_info>")) return 0;
        else if (match_tag(buf, "<use_http_proxy/>")) use_http_proxy = true;
        else if (match_tag(buf, "<use_socks_proxy/>")) use_socks_proxy = true;
        else if (match_tag(buf, "<use_http_auth/>")) use_http_auth = true;
        else if (parse_int(buf, "<socks_version>", socks_version)) continue;
        else if (parse_str(buf, "<socks_server_name>", socks_server_name, sizeof(socks_server_name))) continue;
        else if (parse_int(buf, "<socks_server_port>", socks_server_port)) continue;
        else if (parse_str(buf, "<http_server_name>", http_server_name, sizeof(http_server_name))) continue;
        else if (parse_int(buf, "<http_server_port>", http_server_port)) continue;
        else if (parse_str(buf, "<socks5_user_name>", socks5_user_name, sizeof(socks5_user_name))) continue;
        else if (parse_str(buf, "<socks5_user_passwd>", socks5_user_passwd, sizeof(socks5_user_passwd))) continue;
        else if (parse_str(buf, "<http_user_name>", http_user_name, sizeof(http_user_name))) continue;
        else if (parse_str(buf, "<http_user_passwd>", http_user_passwd, sizeof(http_user_passwd))) continue;
    }
    return 0;
}

int PROXY_INFO::write(MIOFILE& out) {
   out.printf(
       "<proxy_info>\n"
       "%s"
       "%s"
       "%s"
       "    <socks_version>%d</socks_version>\n"
       "    <socks_server_name>%s</socks_server_name>\n"
       "    <socks_server_port>%d</socks_server_port>\n"
       "    <http_server_name>%s</http_server_name>\n"
       "    <http_server_port>%d</http_server_port>\n"
       "    <socks5_user_name>%s</socks5_user_name>\n"
       "    <socks5_user_passwd>%s</socks5_user_passwd>\n"
       "    <http_user_name>%s</http_user_name>\n"
       "    <http_user_passwd>%s</http_user_passwd>\n"
       "</proxy_info>\n",
       use_http_proxy?"    <use_http_proxy/>\n":"",
       use_socks_proxy?"    <use_socks_proxy/>\n":"",
       use_http_auth?"    <use_http_auth/>\n":"",
       socks_version,
       socks_server_name,
       socks_server_port,
       http_server_name,
       http_server_port,
       socks5_user_name,
       socks5_user_passwd,
       http_user_name,
       http_user_passwd
   );
   return 0;
}

void PROXY_INFO::clear() {
    use_http_proxy = false;
    use_socks_proxy = false;
	use_http_auth = false;
    strcpy(socks_server_name, "");
    strcpy(http_server_name, "");
    socks_server_port = 80;
    http_server_port = 80;
    strcpy(socks5_user_name, "");
    strcpy(socks5_user_passwd, "");
    strcpy(http_user_name, "");
    strcpy(http_user_passwd, "");
    socks_version = 0;
}

#ifdef __GNUC__
static volatile const char  __attribute__((unused)) *BOINCrcsid="$Id$";
#else
static volatile const char *BOINCrcsid="$Id$";
#endif
