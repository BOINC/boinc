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
// add project -project_name x
//      add project
// add app -app_name x
//      add application
//      create DB record
// add platform -platform_name x
//      create DB record
// add app_version
//      -app_name x -platform_name y -version a
//      -download_dir d -download_url e
//      -exec_dir b 
//      [ -exec_files file1 file2 ... ]
//      [ -signed_exec_files file1 sign1 file2 sign2 ... ]
//      create DB record
//      copy exec to data directory
// add user -email_addr x -name y -web_password z -authenticator a
//      [ -global_prefs_file y ]

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "db.h"
#include "backend_lib.h"
#include "md5_file.h"
#include "crypt.h"

APP app;
PLATFORM platform;
APP_VERSION app_version;
USER user;
PROJECT project;

int version, retval, nexec_files;
double nbytes;
bool signed_exec_files;
char buf[256], md5_cksum[64];
char *db_name=0, *db_passwd=0, *app_name=0, *platform_name=0, *project_name=0;
char* exec_dir=0, *exec_files[10], *signature_files[10];
char *email_addr=0, *user_name=0, *web_password=0, *authenticator=0;
char *global_prefs_file=0, *download_dir, *download_url;
char* code_sign_keyfile=0;
char *message=0, *message_priority=0;

void add_project() {
    int retval;

    if (!project_name) {
        fprintf( stderr, "Project name not specified.\n" );
        exit(1);
    }
    memset(&project, 0, sizeof(project));
    strcpy(project.name, project_name);
    retval = db_project_new(project);
    if (retval) {
        boinc_db_print_error("db_project_new");
    }
}

void add_app() {
    int retval;

    if (!app_name) {
        fprintf( stderr, "Application name not specified.\n" );
        exit(1);
    }
    memset(&app, 0, sizeof(app));
    strcpy(app.name, app_name);
    app.create_time = time(0);
    app.min_version = version;
    retval = db_app_new(app);
    if (retval) {
        boinc_db_print_error("db_app_new");
    }
}

void add_platform() {
    int retval;

    if (!platform_name) {
        fprintf( stderr, "Platform name not specified.\n" );
        exit(1);
    }
    memset(&platform, 0, sizeof(platform));
    strcpy(platform.name, platform_name);
    platform.create_time = time(0);
    retval = db_platform_new(platform);
    if (retval) {
        boinc_db_print_error("db_platform_new");
    }
}

int sign_executable(char* path, char* signature_text) {
    DATA_BLOCK signature;
    unsigned char signature_buf[SIGNATURE_SIZE_BINARY];
    R_RSA_PRIVATE_KEY code_sign_key;
    retval = read_key_file(code_sign_keyfile, code_sign_key);
    if (retval) {
        fprintf(stderr, "add: can't read key\n");
        exit(1);
    }
    signature.data = signature_buf;
    sign_file(path, code_sign_key, signature);
    sprint_hex_data(signature_text, signature);
    return 0;
}

void add_app_version() {
    char path[256];
    char signature_text[1024], longbuf[MAX_BLOB_SIZE];
    int i;

    memset(&app_version, 0, sizeof(app_version));

    if (!app_name) {
        fprintf( stderr, "Application name not specified.\n" );
        exit(1);
    }
    strcpy(app.name, app_name);
    retval = db_app_lookup_name(app);
    if (retval) {
        fprintf(stderr, "add_app_version(): can't find app %s\n", app_name);
        boinc_db_print_error("db_app_lookup_name");
        return;
    }
    app_version.appid = app.id;
    strcpy(platform.name, platform_name);
    retval = db_platform_lookup_name(platform);
    if (retval) {
        fprintf(stderr, "add_app_version(): can't find platform %s\n", platform_name);
        boinc_db_print_error("db_platform_lookup_name");
        return;
    }
    app_version.platformid = platform.id;
    app_version.version_num = version;
    if (message) strcpy(app_version.message, message);
    if (message_priority) strcpy(app_version.message, message_priority);

    strcpy(app_version.xml_doc, "");

    // copy executables to download directory and sign them
    //
    for (i=0; i<nexec_files; i++) {
        sprintf(
            buf,
            "cp %s/%s %s/%s",
            exec_dir, exec_files[i], download_dir, exec_files[i]
        );
        retval = system(buf);
        if (retval) {
            printf("failed: %s\n", buf);
            return;
        }

        if (signed_exec_files) {
            read_filename(signature_files[i], signature_text);
        } else {
            sprintf(path, "%s/%s", exec_dir, exec_files[i]);
            sign_executable(path, signature_text);
        }

        md5_file(path, md5_cksum, nbytes);

        // generate the XML doc directly.
        // TODO: use a template, as in create_work (??)
        //
        sprintf(longbuf,
            "<file_info>\n"
            "    <name>%s</name>\n"
            "    <url>%s/%s</url>\n"
            "    <executable/>\n"
            "    <file_signature>\n%s"
            "    </file_signature>\n"
            "    <nbytes>%f</nbytes>\n"
            "</file_info>\n",
            exec_files[i],
            download_url, exec_files[i],
            signature_text,
            nbytes
        );
        strcat(app_version.xml_doc, longbuf);
    }

    sprintf(longbuf,
        "<app_version>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n",
        app_name,
        version
    );
    strcat(app_version.xml_doc, longbuf);
    for (i=0; i<nexec_files; i++) {
        sprintf(longbuf,
            "    <file_ref>\n"
            "        <file_name>%s</file_name>\n"
            "%s"
            "    </file_ref>\n",
            exec_files[i],
            i?"":"        <main_program/>\n"
        );
        strcat(app_version.xml_doc, longbuf);
    }
    strcat(app_version.xml_doc, "</app_version>\n");

    app_version.create_time = time(0);
    retval = db_app_version_new(app_version);
    if (retval) {
        boinc_db_print_error("db_app_version_new");
        return;
    }
}

void add_user() {
    memset(&user, 0, sizeof(user));
    user.create_time = time(0);
    strcpy(user.email_addr, email_addr);
    strcpy(user.name, user_name);
    strcpy(user.web_password, web_password);
    strcpy(user.authenticator, authenticator);
    strcpy(user.country, "United States");
    strcpy(user.postal_code, "94703");
    if (global_prefs_file) {
        retval = read_filename(global_prefs_file, user.global_prefs);
        if (retval) {
            printf("read_file: %s", global_prefs_file);
            return;
        }
    }
    retval = db_user_new(user);
    if (retval) {
        boinc_db_print_error("db_user_new");
        return;
    }
}

int main(int argc, char** argv) {
    int i, retval;

    for (i=2; i<argc; i++) {
        if (!strcmp(argv[i], "-db_name")) {
            db_name = argv[++i];
        } else if (!strcmp(argv[i], "-db_passwd")) {
            db_passwd = argv[++i];
        } else if (!strcmp(argv[i], "-project_name")) {
            i++;
            project_name = argv[i];
        } else if (!strcmp(argv[i], "-app_name")) {
            i++;
            app_name = argv[i];
        } else if (!strcmp(argv[i], "-platform_name")) {
            i++;
            platform_name = argv[i];
        } else if (!strcmp(argv[i], "-exec_dir")) {
            i++;
            exec_dir = argv[i];
        } else if (!strcmp(argv[i], "-exec_files")) {
            signed_exec_files = false;
            i++;
            nexec_files = 0;
            while (i < argc) {
                exec_files[nexec_files++] = argv[i++];
            }
            break;
        } else if (!strcmp(argv[i], "-signed_exec_files")) {
            signed_exec_files = true;
            i++;
            nexec_files = 0;
            while (i < argc) {
                exec_files[nexec_files] = argv[i++];
                signature_files[nexec_files] = argv[i++];
                nexec_files++;
            }
            break;
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
        } else if (!strcmp(argv[i], "-global_prefs_file")) {
            i++;
            global_prefs_file = argv[i];
        } else if (!strcmp(argv[i], "-download_url")) {
            i++;
            download_url = argv[i];
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
        } else if (!strcmp(argv[i], "-code_sign_keyfile")) {
            i++;
            code_sign_keyfile = argv[i];
        }
    }
    retval = boinc_db_open(db_name, db_passwd);
    if (retval) {
    	fprintf(stderr, "can't open DB %s\n", db_name);
        exit(1);
    }
    if (!strcmp(argv[1], "project")) {
	add_project();
    } else if (!strcmp(argv[1], "app")) {
        add_app();
    } else if (!strcmp(argv[1], "platform")) {
        add_platform();
    } else if (!strcmp(argv[1], "app_version")) {
        add_app_version();
    } else if (!strcmp(argv[1], "user")) {
        add_user();
    } else {
        printf("Unrecognized command\n");
    }
    boinc_db_close();
    exit(0);
}
