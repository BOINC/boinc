#include <stdio.h>
#include <string.h>

#include "parse.h"
#include "error_numbers.h"

#include "config.h"

int CONFIG::parse(FILE* in) {
    char buf[256];

    memset(this, 0, sizeof(CONFIG));
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</config>")) return 0;
        else if (parse_str(buf, "<db_name>", db_name, sizeof(db_name))) continue;
        else if (parse_str(buf, "<db_passwd>", db_passwd, sizeof(db_passwd))) continue;
        else if (parse_int(buf, "<shmem_key>", shmem_key)) continue;
        else if (parse_str(buf, "<key_dir>", key_dir, sizeof(key_dir))) continue;
        else if (parse_str(buf, "<download_url>", download_url, sizeof(download_url))) continue;
        else if (parse_str(buf, "<upload_url>", upload_url, sizeof(upload_url))) continue;
        else if (parse_str(buf, "<upload_dir>", upload_dir, sizeof(upload_dir))) continue;
        else if (parse_str(buf, "<user_name>", user_name, sizeof(user_name))) continue;
    }
    return ERR_XML_PARSE;
}

int CONFIG::parse_file() {
    FILE* f;

    f = fopen("config.xml", "r");
    if (!f) return ERR_FOPEN;
    return parse(f);
}
