// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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
//
// NOTE:
//  - all commands take optional -db_name and -db_password args
//  - all commands look for config.xml and, if found, get
//    the db_name, db_password, up/download_dir, up/download_url
//    from there (can override them w/ cmdline args)
//
// add project
//      -project_short_name x
//      -project_long_name x
//      add project
// add app -app_name x
//      add application
//      create DB record
// add platform -platform_name x -user_friendly_name y
//      create DB record
// add app_version
//      -app_name x -platform_name y -version a
//      [ -download_dir d ]
//      [ -download_url e ]
//      -exec_dir b
//      [ -exec_files file1 file2 ... ]
//      [ -signed_exec_files file1 sign1 file2 sign2 ... ]
//      create DB record (w/ XML descriptor; generate signatures if needed)
//      copy executable files from exec_dir to download_dir directory
// add core_version
//      -platform_name x -version n
//      [ -message 'foo' ] [ -message_priority 'foo']
//      [ -download_dir d -download_url e ]
//      -exec_dir b
//      -exec_files file1
// add user -email_addr x -name y -authenticator a
//      [ -global_prefs_file y ]

#include <cstring>
#include <cstdlib>
#include <ctime>

#include "boinc_db.h"
#include "sched_config.h"
#include "backend_lib.h"
#include "md5_file.h"
#include "crypt.h"

//APP app;
//PLATFORM platform;
//APP_VERSION app_version;
//USER user;
//PROJECT project;

int version, retval, nexec_files;
double nbytes;
bool signed_exec_files;
char buf[256], md5_cksum[64];
char *db_name=0, *db_passwd=0, *app_name=0, *platform_name=0;
char *project_short_name=0, *project_long_name=0;
char* user_friendly_name=0;
char* exec_dir=0, *exec_files[10], *signature_files[10];
char *email_addr=0, *user_name=0, *authenticator=0;
char *global_prefs_file=0, *download_dir, *download_url;
char code_sign_keyfile[256];
char *message=0, *message_priority=0;

void add_project() {
    int retval;
    DB_PROJECT project;

    if (!project_short_name || !project_long_name) {
        fprintf( stderr, "Project name (long or short) not specified.\n" );
        exit(1);
    }
    project.clear();
    strcpy(project.short_name, project_short_name);
    strcpy(project.long_name, project_long_name);
    retval = project.insert();
    if (retval) {
        boinc_db.print_error("project.insert()");
    }
}

void add_app() {
    int retval;
    DB_APP app;

    if (!app_name) {
        fprintf( stderr, "Application name not specified.\n" );
        exit(1);
    }
    app.clear();
    strcpy(app.name, app_name);
    app.create_time = time(0);
    app.min_version = version;
    retval = app.insert();
    if (retval) {
        boinc_db.print_error("app.insert()");
    }
}

void add_platform() {
    int retval;
    DB_PLATFORM platform;

    if (!user_friendly_name) {
        fprintf( stderr, "User friendly name not specified.\n" );
        exit(1);
    }
    if (!platform_name) {
        fprintf( stderr, "Platform name not specified.\n" );
        exit(1);
    }
    platform.clear();
    strcpy(platform.name, platform_name);
    strcpy(platform.user_friendly_name, user_friendly_name);
    platform.create_time = time(0);
    retval = platform.insert();
    if (retval) {
        boinc_db.print_error("platform.insert()");
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

// copy executable file to the download dir, generate XML
//
static int process_executable_file(
    char* filename, char* signature_text, char* xml_doc
) {
    char longbuf[MEDIUM_BLOB_SIZE];
    char path[256];

    sprintf(path, "%s/%s", exec_dir, filename);
    sprintf(
        buf,
        "cp %s %s/%s",
        path, download_dir, filename
    );
    retval = system(buf);
    if (retval) {
        printf("failed: %s\n", buf);
        return retval;
    }

    retval = md5_file(path, md5_cksum, nbytes);
    if (retval) return retval;

    // generate the XML doc directly.
    // TODO: use a template, as in create_work (??)
    //
    sprintf(longbuf,
        "<file_info>\n"
        "    <name>%s</name>\n"
        "    <url>%s/%s</url>\n"
        "    <executable/>\n",
        filename,
        download_url, filename
    );
    strcat(xml_doc, longbuf);
    if (signature_text) {
        sprintf(longbuf,
            "    <file_signature>\n%s"
            "    </file_signature>\n",
            signature_text
        );
    } else {
        sprintf(longbuf,
            "    <md5_cksum>%s</md5_cksum>\n",
            md5_cksum
        );
    }
    strcat(xml_doc, longbuf);
    sprintf(longbuf,
        "    <nbytes>%f</nbytes>\n"
        "</file_info>\n",
        nbytes
    );
    strcat(xml_doc, longbuf);
    return 0;
}

void add_core_version() {
    DB_CORE_VERSION core_version;
    DB_PLATFORM platform;

    core_version.clear();
    sprintf(buf, "where name='%s'", platform_name);
    retval = platform.lookup(buf);
    if (retval) {
        fprintf(stderr, "add_core_version(): can't find platform %s\n", platform_name);
        boinc_db.print_error("platform.lookup()");
        return;
    }
    core_version.platformid = platform.id;
    core_version.version_num = version;
    if (message) strcpy(core_version.message, message);
    if (message_priority) strcpy(core_version.message, message_priority);
    if (nexec_files != 1) {
        fprintf(stderr, "add_core_version(): multiple files not allowed\n");
        return;
    }
    strcpy(core_version.xml_doc, "");
    process_executable_file(exec_files[0], NULL, core_version.xml_doc);
    core_version.create_time = time(0);
    retval = core_version.insert();
    if (retval) {
        boinc_db.print_error("core_version.insert()");
    }
}

void add_app_version() {
    char path[256];
    char longbuf[MEDIUM_BLOB_SIZE];
    char signature_text[1024];
    int i;
    DB_APP app;
    DB_APP_VERSION app_version;
    DB_PLATFORM platform;

    app_version.clear();

    if (!app_name) {
        fprintf( stderr, "Application name not specified.\n" );
        exit(1);
    }
    sprintf(buf, "where name='%s'", app_name);
    retval = app.lookup(buf);
    if (retval) {
        fprintf(stderr, "add_app_version(): can't find app %s\n", app_name);
        boinc_db.print_error("app.lookup()");
        return;
    }
    app_version.appid = app.id;
    sprintf(buf, "where name='%s'", platform_name);
    retval = platform.lookup(buf);
    if (retval) {
        fprintf(stderr, "add_app_version(): can't find platform %s\n", platform_name);
        boinc_db.print_error("platform.lookup()");
        return;
    }
    app_version.platformid = platform.id;
    app_version.version_num = version;

    strcpy(app_version.xml_doc, "");

    // copy executables to download directory and sign them
    //
    for (i=0; i<nexec_files; i++) {
        if (signed_exec_files) {
            read_filename(signature_files[i], signature_text);
        } else {
            sprintf(path, "%s/%s", exec_dir, exec_files[i]);
            sign_executable(path, signature_text);
        }
        retval = process_executable_file(
            exec_files[i], signature_text, app_version.xml_doc
        );
        if (retval) {
            fprintf(stderr, "process_executable_file(): %d\n", retval);
            exit(1);
        }
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
    retval = app_version.insert();
    if (retval) {
        boinc_db.print_error("app_version.insert()");
        return;
    }
}

void add_user() {
    DB_USER user;

    user.clear();
    user.create_time = time(0);
    strcpy(user.email_addr, email_addr);
    strcpy(user.name, user_name);
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
    retval = user.insert();
    if (retval) {
        boinc_db.print_error("user.insert()");
        return;
    }
}

int main(int argc, char** argv) {
    int i, retval;
    SCHED_CONFIG config;

    printf("%s", "add: this tool is deprecated; use add.py\n");

    retval = config.parse_file();
    if (retval) {
        printf("no config file found\n");
    } else {
        db_name = config.db_name;
        db_passwd = config.db_passwd;
        download_url = config.download_url;
        download_dir = config.download_dir;
        sprintf(code_sign_keyfile, "%s/code_sign_private", config.key_dir);
    }

    for (i=2; i<argc; i++) {
    next_arg:
        if (!strcmp(argv[i], "-db_name")) {
            db_name = argv[++i];
        } else if (!strcmp(argv[i], "-db_passwd")) {
            db_passwd = argv[++i];
        } else if (!strcmp(argv[i], "-project_long_name")) {
            project_long_name = argv[++i];
        } else if (!strcmp(argv[i], "-project_short_name")) {
            project_short_name = argv[++i];
        } else if (!strcmp(argv[i], "-app_name")) {
            app_name = argv[++i];
        } else if (!strcmp(argv[i], "-platform_name")) {
            platform_name = argv[++i];
        } else if (!strcmp(argv[i], "-user_friendly_name")) {
            user_friendly_name = argv[++i];
        } else if (!strcmp(argv[i], "-exec_dir")) {
            exec_dir = argv[++i];
        } else if (!strcmp(argv[i], "-exec_files")) {
            signed_exec_files = false;
            i++;
            nexec_files = 0;
            while (i < argc) {
                if (!strncmp(argv[i],"-",1)) goto next_arg;
                exec_files[nexec_files++] = argv[i++];
            }
            break;
        } else if (!strcmp(argv[i], "-signed_exec_files")) {
            signed_exec_files = true;
            i++;
            nexec_files = 0;
            while (i < argc) {
                if (!strncmp(argv[i],"-",1)) goto next_arg;
                exec_files[nexec_files] = argv[i++];
                signature_files[nexec_files] = argv[i++];
                nexec_files++;
            }
            break;
        } else if (!strcmp(argv[i], "-exec_dir")) {
            exec_dir = argv[++i];
        } else if (!strcmp(argv[i], "-email_addr")) {
            email_addr = argv[++i];
        } else if (!strcmp(argv[i], "-user_name")) {
            user_name = argv[++i];
        } else if (!strcmp(argv[i], "-authenticator")) {
            authenticator = argv[++i];
        } else if (!strcmp(argv[i], "-global_prefs_file")) {
            global_prefs_file = argv[++i];
        } else if (!strcmp(argv[i], "-download_url")) {
            download_url = argv[++i];
        } else if (!strcmp(argv[i], "-download_dir")) {
            download_dir = argv[++i];
        } else if (!strcmp(argv[i], "-version")) {
            version = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-message")) {
            message = argv[++i];
        } else if (!strcmp(argv[i], "-message_priority")) {
            message_priority = argv[++i];
        } else if (!strcmp(argv[i], "-code_sign_keyfile")) {
            strcpy(code_sign_keyfile, argv[++i]);
        }
    }
    retval = boinc_db.open(db_name, "", db_passwd);
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
    } else if (!strcmp(argv[1], "core_version")) {
        add_core_version();
    } else if (!strcmp(argv[1], "app_version")) {
        add_app_version();
    } else if (!strcmp(argv[1], "user")) {
        add_user();
    } else {
        printf("Unrecognized command\n");
    }
    boinc_db.close();
    exit(0);
}
