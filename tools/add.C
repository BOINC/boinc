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

// add.C - add items to the DB
//
// usages:
// add app -app_name x
//      add application
//      create DB record
// add platform -platform_name x
//      create DB record
// add app_version
//      -app_name x -platform_name y -version a -exec_dir b -exec_file c
//      -download_dir d -url_base e
//      create DB record
//      copy exec to data directory
// add user -email_addr x -name y -web_password z -authenticator a
// add prefs -email_addr x -prefs_file y

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "db.h"
#include "backend_lib.h"
#include "md5_file.h"

int main(int argc, char** argv) {
    APP app;
    PLATFORM platform;
    APP_VERSION app_version;
    USER user;
    PREFS prefs;
    int i, version, retval;
    double nbytes;
    char buf[256], md5_cksum[64];
    char *app_name=0, *platform_name=0, *exec_dir=0, *exec_file=0;
    char *email_addr=0, *user_name=0, *web_password=0, *authenticator=0;
    char *prefs_file=0, *download_dir, *url_base;
    char *message=0, *message_priority=0;

    db_open("boinc");
    for (i=2; i<argc; i++) {
        if (!strcmp(argv[i], "-app_name")) {
            i++;
            app_name = argv[i];
        } else if (!strcmp(argv[i], "-platform_name")) {
            i++;
            platform_name = argv[i];
        } else if (!strcmp(argv[i], "-exec_dir")) {
            i++;
            exec_dir = argv[i];
        } else if (!strcmp(argv[i], "-exec_file")) {
            i++;
            exec_file= argv[i];
        } else if (!strcmp(argv[i], "-exec_dir")) {
            i++;
            exec_dir = argv[i];
        } else if (!strcmp(argv[i], "-email_addr")) {
            i++;
            email_addr = argv[i];
        } else if (!strcmp(argv[i], "-user_name")) {
            i++;
            user_name = argv[i];
        } else if (!strcmp(argv[i], "-web_password")) {
            i++;
            web_password = argv[i];
        } else if (!strcmp(argv[i], "-authenticator")) {
            i++;
            authenticator = argv[i];
        } else if (!strcmp(argv[i], "-prefs_file")) {
            i++;
            prefs_file = argv[i];
        } else if (!strcmp(argv[i], "-url_base")) {
            i++;
            url_base = argv[i];
        } else if (!strcmp(argv[i], "-download_dir")) {
            i++;
            download_dir = argv[i];
        } else if (!strcmp(argv[i], "-version")) {
            i++;
            version = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-message")) {
            i++;
            message = argv[i];
        } else if (!strcmp(argv[i], "-message_priority")) {
            i++;
            message_priority = argv[i];
        }
    }
    if (!strcmp(argv[1], "app")) {
        memset(&app, 0, sizeof(app));
        strcpy(app.name, app_name);
        app.create_time = time(0);
        app.alpha_vers = version;
        app.beta_vers = version;
        app.prod_vers = version;
        retval = db_app_new(app);
        if (retval) {
            db_print_error("db_app_new");
        }
    } else if (!strcmp(argv[1], "platform")) {
        memset(&platform, 0, sizeof(platform));
        strcpy(platform.name, platform_name);
        platform.create_time = time(0);
        retval = db_platform_new(platform);
        if (retval) {
            db_print_error("db_platform_new");
        }
    } else if (!strcmp(argv[1], "app_version")) {
        memset(&app_version, 0, sizeof(app_version));

        strcpy(app.name, app_name);
        retval = db_app_lookup_name(app);
        if (retval) {
            fprintf(stderr, "can't find app %s\n", app_name);
            db_print_error("db_app_lookup_name");
            goto done;
        }
        app_version.appid = app.id;
        strcpy(platform.name, platform_name);
        retval = db_platform_lookup_name(platform);
        if (retval) {
            fprintf(stderr, "can't find platform %s\n", platform_name);
            db_print_error("db_platform_lookup_name");
            goto done;
        }
        app_version.platformid = platform.id;
        app_version.version_num = version;
        if (message) strcpy(app_version.message, message);
        if (message_priority) strcpy(app_version.message, message_priority);

        // copy executable to download directory
        //
        sprintf(
            buf,
            "cp %s/%s %s/%s",
            exec_dir, exec_file, download_dir, exec_file
        );
        retval = system(buf);
        if (retval) {
            printf("failed: %s\n", buf);
            goto done;
        }

        // compute checksum of executable
        //
        sprintf(buf, "%s/%s", exec_dir, exec_file);
        md5_file(buf, md5_cksum, nbytes);

        // generate the XML doc directly.
        // TODO: use a template, as in create_work
        //
        sprintf(app_version.xml_doc,
            "<file_info>\n"
            "    <name>%s</name>\n"
            "    <url>%s/%s</url>\n"
            "    <executable/>\n"
            "    <md5_cksum>%s</md5_cksum>\n"
            "    <nbytes>%f</nbytes>\n"
            "</file_info>\n"
            "<app_version>\n"
            "    <app_name>%s</app_name>\n"
            "    <version_num>%d</version_num>\n"
            "    <file_ref>\n"
            "        <file_name>%s</file_name>\n"
            "        <main_program/>\n"
            "    </file_ref>\n"
            "</app_version>\n",
            exec_file,
            url_base, exec_file,
            md5_cksum,
            nbytes,
            app_name,
            version,
            exec_file
        );

        app_version.create_time = time(0);
        retval = db_app_version_new(app_version);
        if (retval) {
            db_print_error("db_app_version_new");
            goto done;
        }
    } else if (!strcmp(argv[1], "user")) {
        memset(&user, 0, sizeof(user));
        user.create_time = time(0);
        strcpy(user.email_addr, email_addr);
        strcpy(user.name, user_name);
        strcpy(user.web_password, web_password);
        strcpy(user.authenticator, authenticator);
        strcpy(user.country, "United States");
        strcpy(user.postal_code, "94703");
        retval = db_user_new(user);
        if (retval) {
            db_print_error("db_user_new");
            goto done;
        }
    } else if (!strcmp(argv[1], "prefs")) {
        memset(&prefs, 0, sizeof(prefs));
        strcpy(user.email_addr, email_addr);
        retval = db_user_lookup_email_addr(user);
        if (retval) {
            db_print_error("db_user_lookup_email_addr");
            goto done;
        }
        prefs.create_time = time(0);
        prefs.modified_time = time(0);
        prefs.userid = user.id;
        retval = read_file(prefs_file, prefs.xml_doc);
        if (retval) {
            printf("read_file: %s", prefs_file);
            goto done;
        }
        strcpy(prefs.name, prefs_file);
        retval = db_prefs_new(prefs);
        if (retval) {
            db_print_error("db_prefs_new");
            goto done;
        }
        user.default_prefsid = db_insert_id();
        retval = db_user_update(user);
        if (retval) {
            db_print_error("db_user_update");
            goto done;
        }
    } else {
        printf("Unrecognized command\n");
    }
done:
    db_close();
    exit(0);
}
