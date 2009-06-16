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

#include "config.h"
#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#else
#include <cstdio>
#endif
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <cassert>
#include <unistd.h>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>


#include "boinc_db.h"
#include "crypt.h"
#include "error_numbers.h"
#include "md5_file.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "common_defs.h"
#include "filesys.h"
#include "sched_util.h"
#include "util.h"

#include "backend_lib.h"


using std::string;

static struct random_init {
    random_init() {
    srand48(getpid() + time(0));
                        }
} random_init;

int read_file(FILE* f, char* buf, int len) {
    int n = fread(buf, 1, len, f);
    buf[n] = 0;
    return 0;
}

int read_filename(const char* path, char* buf, int len) {
    int retval;
#ifndef _USING_FCGI_
    FILE* f = fopen(path, "r");
#else
    FCGI_FILE *f=FCGI::fopen(path, "r");
#endif
    if (!f) return -1;
    retval = read_file(f, buf, len);
    fclose(f);
    return retval;
}

// see checkin notes Dec 30 2004
//
static bool got_md5_info(
    const char *path,
    char *md5data,
    double *nbytes
) {
    bool retval=false;
    // look for file named FILENAME.md5 containing md5sum and length.
    // If found, and newer mod time than file, read md5 sum and file
    // length from it.

    char md5name[512];
    struct stat md5stat, filestat;
    char endline='\0';

    sprintf(md5name, "%s.md5", path);
    
    // get mod times for file
    if (stat(path, &filestat))
        return retval;
  
    // get mod time for md5 cache file
    if (stat(md5name, &md5stat))
        return retval;
    
    // if cached md5 newer, then open it
#ifndef _USING_FCGI_
    FILE *fp=fopen(md5name, "r");
#else
    FCGI_FILE *fp=FCGI::fopen(md5name, "r");
#endif
    if (!fp)
        return retval;
  
    // read two quantities: md5 sum and length.  If we can't read
    // these, or there is MORE stuff in the file' it's not an md5
    // cache file
    if (3 == fscanf(fp, "%s %lf%c", md5data, nbytes, &endline) &&
        endline=='\n' &&
        EOF==fgetc(fp)
       )
        retval=true; 
    fclose(fp);

    // if this is one of our cached md5 files, but it's OLDER than the
    // data file which it supposedly corresponds to, delete it.
    if (retval && md5stat.st_mtime<filestat.st_mtime) {
        unlink(md5name);
        retval=false;
    }
    return retval;
}

// see checkin notes Dec 30 2004
//
static void write_md5_info(
    const char *path,
    const char *md5,
    double nbytes
) {
    // Write file FILENAME.md5 containing md5sum and length
    //
    char md5name[512];
    struct stat statbuf;
    int retval;
    
    // if file already exists with this name, don't touch it.
    //
    sprintf(md5name, "%s.md5", path);
    if (!stat(md5name, &statbuf)) {
        return;
    }

#ifndef _USING_FCGI_
    FILE *fp=fopen(md5name, "w");
#else
    FCGI_FILE *fp=FCGI::fopen(md5name, "w");
#endif

    // if can't open the file, give up
    //
    if (!fp) {
        return;
    }
  
    retval = fprintf(fp,"%s %.15e\n", md5, nbytes);
    fclose(fp);

    // if we didn't write properly to the file, delete it.
    //
    if (retval<0) {
        unlink(md5name);
    }
  
    return;
}

// fill in the workunit's XML document (wu.xml_doc)
// by scanning the WU template, macro-substituting the input files,
// and putting in the command line element and additional XML
//
static int process_wu_template(
    WORKUNIT& wu,
    char* tmplate,
    const char** infiles,
    int ninfiles,
    SCHED_CONFIG& config_loc,
    const char* command_line,
    const char* additional_xml
) {
    char* p;
    char buf[BLOB_SIZE], md5[33], path[256], url[256], top_download_path[256];
    string out, cmdline;
    int retval, file_number;
    double nbytes;
    char open_name[256];
    bool found=false;
    int nfiles_parsed = 0;

    out = "";
    for (p=strtok(tmplate, "\n"); p; p=strtok(0, "\n")) {
        if (match_tag(p, "<file_info>")) {
            bool generated_locally = false;
            file_number = -1;
            out += "<file_info>\n";
            while (1) {
                p = strtok(0, "\n");
                if (!p) break;
                if (parse_int(p, "<number>", file_number)) {
                    continue;
                } else if (parse_bool(p, "generated_locally", generated_locally)) {
                    continue;
                } else if (match_tag(p, "</file_info>")) {
                    if (file_number < 0) {
                        fprintf(stderr, "No file number found\n");
                        return ERR_XML_PARSE;
                    }
                    if (file_number >= ninfiles) {
                        fprintf(stderr,
                            "Too few input files given; need at least %d\n",
                            file_number+1
                        );
                        return ERR_XML_PARSE;
                    }
                    nfiles_parsed++;
                    if (generated_locally) {
                        sprintf(buf,
                            "    <name>%s</name>\n"
                            "    <generated_locally/>\n"
                            "</file_info>\n",
                            infiles[file_number]
                        );
                    } else {
                        dir_hier_path(
                            infiles[file_number], config_loc.download_dir,
                            config_loc.uldl_dir_fanout, path, true
                        );

                        // if file isn't found in hierarchy,
                        // look for it at top level and copy
                        //
                        if (!boinc_file_exists(path)) {
                            sprintf(top_download_path,
                                "%s/%s",config_loc.download_dir,
                                infiles[file_number]
                            );
                            boinc_copy(top_download_path, path);
                        }

                        if (!config_loc.cache_md5_info || !got_md5_info(path, md5, &nbytes)) {
                            retval = md5_file(path, md5, nbytes);
                            if (retval) {
                                fprintf(stderr, "process_wu_template: md5_file %d\n", retval);
                                return retval;
                            }
                            else if (config_loc.cache_md5_info) {
                                write_md5_info(path, md5, nbytes);
                            }
                        }

                        dir_hier_url(
                            infiles[file_number], config_loc.download_url,
                            config_loc.uldl_dir_fanout, url
                        );
                        sprintf(buf,
                            "    <name>%s</name>\n"
                            "    <url>%s</url>\n"
                            "    <md5_cksum>%s</md5_cksum>\n"
                            "    <nbytes>%.0f</nbytes>\n"
                            "</file_info>\n",
                            infiles[file_number],
                            url,
                            md5,
                            nbytes
                        );
                    }
                    out += buf;
                    break;
                } else {
                    out += p;
                    out += "\n";
                }
            }
        } else if (match_tag(p, "<workunit>")) {
            found = true;
            out += "<workunit>\n";
            if (command_line) {
                out += "<command_line>\n";
                out += command_line;
                out += "\n</command_line>\n";
            }
        } else if (match_tag(p, "</workunit>")) {
            if (additional_xml && strlen(additional_xml)) {
                out += additional_xml;
                out += "\n";
            }
            out += "</workunit>\n";
        } else if (match_tag(p, "<file_ref>")) {
            out += "<file_ref>\n";
            bool found_file_number = false, found_open_name = false;
            while (1) {
                p = strtok(0, "\n");
                if (!p) break;
                if (parse_int(p, "<file_number>", file_number)) {
                    sprintf(buf, "    <file_name>%s</file_name>\n",
                        infiles[file_number]
                    );
                    out += buf;
                    found_file_number = true;
                    continue;
                } else if (parse_str(p, "<open_name>", open_name, sizeof(open_name))) {
                    sprintf(buf, "    <open_name>%s</open_name>\n", open_name);
                    out += buf;
                    found_open_name = true;
                    continue;
                } else if (match_tag(p, "</file_ref>")) {
                    if (!found_file_number) {
                        fprintf(stderr, "No file number found\n");
                        return ERR_XML_PARSE;
                    }
                    if (!found_open_name) {
                        fprintf(stderr, "No open name found\n");
                        return ERR_XML_PARSE;
                    }
                    out += "</file_ref>\n";
                    break;
                } else {
                    sprintf(buf, "%s\n", p);
                    out += buf;
                }
            }
        } else if (parse_str(p, "<command_line>", cmdline)) {
            if (command_line) {
                fprintf(stderr, "Can't specify command line twice");
                return ERR_XML_PARSE;
            }
            out += "<command_line>\n";
            out += cmdline;
            out += "\n</command_line>\n";
        } else if (parse_double(p, "<rsc_fpops_est>", wu.rsc_fpops_est)) {
            continue;
        } else if (parse_double(p, "<rsc_fpops_bound>", wu.rsc_fpops_bound)) {
            continue;
        } else if (parse_double(p, "<rsc_memory_bound>", wu.rsc_memory_bound)) {
            continue;
        } else if (parse_double(p, "<rsc_bandwidth_bound>", wu.rsc_bandwidth_bound)) {
            continue;
        } else if (parse_double(p, "<rsc_disk_bound>", wu.rsc_disk_bound)) {
            continue;
        } else if (parse_int(p, "<batch>", wu.batch)) {
            continue;
        } else if (parse_int(p, "<delay_bound>", wu.delay_bound)) {
            continue;
        } else if (parse_int(p, "<min_quorum>", wu.min_quorum)) {
            continue;
        } else if (parse_int(p, "<target_nresults>", wu.target_nresults)) {
            continue;
        } else if (parse_int(p, "<max_error_results>", wu.max_error_results)) {
            continue;
        } else if (parse_int(p, "<max_total_results>", wu.max_total_results)) {
            continue;
        } else if (parse_int(p, "<max_success_results>", wu.max_success_results)) {
            continue;
        } else {
            out += p;
            out += "\n";
        }
    }
    if (!found) {
        fprintf(stderr, "process_wu_template: bad WU template - no <workunit>\n");
        return -1;
    }
    if (nfiles_parsed != ninfiles) {
        fprintf(stderr,
            "process_wu_template: %d input files listed, but template has %d\n",
            ninfiles, nfiles_parsed
        );
        return -1;
    }
    if (out.size() > sizeof(wu.xml_doc)-1) {
        fprintf(stderr,
            "create_work: WU XML field is too long (%d bytes; max is %d)\n",
            out.size(), sizeof(wu.xml_doc)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    strcpy(wu.xml_doc, out.c_str());
    return 0;
}

// initialize an about-to-be-created result, given its WU
//
static void initialize_result(DB_RESULT& result, WORKUNIT& wu) {
    result.id = 0;
    result.create_time = time(0);
    result.workunitid = wu.id;
    result.server_state = RESULT_SERVER_STATE_UNSENT;
    result.hostid = 0;
    result.report_deadline = 0;
    result.sent_time = 0;
    result.received_time = 0;
    result.client_state = 0;
    result.cpu_time = 0;
    strcpy(result.xml_doc_out, "");
    strcpy(result.stderr_out, "");
    result.outcome = RESULT_OUTCOME_INIT;
    result.file_delete_state = ASSIMILATE_INIT;
    result.validate_state = VALIDATE_STATE_INIT;
    result.claimed_credit = 0;
    result.granted_credit = 0;
    result.appid = wu.appid;
    result.priority = wu.priority;
    result.batch = wu.batch;
}

int create_result_ti(
    TRANSITIONER_ITEM& ti,
    char* result_template_filename,
    char* result_name_suffix,
    R_RSA_PRIVATE_KEY& key,
    SCHED_CONFIG& config_loc,
    char* query_string,
        // if nonzero, write value list here; else do insert
    int priority_increase
) {
    WORKUNIT wu;

    // copy relevant fields from TRANSITIONER_ITEM to WORKUNIT
    //
    strcpy(wu.name, ti.name);
    wu.id = ti.id;
    wu.appid = ti.appid;
    wu.priority = ti.priority;
    wu.batch = ti.batch;
    return create_result(
        wu,
        result_template_filename,
        result_name_suffix,
        key,
        config_loc,
        query_string,
        priority_increase
    );
}

// Create a new result for the given WU.
// This is called ONLY from the transitioner
//
int create_result(
    WORKUNIT& wu,
    char* result_template_filename,
    char* result_name_suffix,
    R_RSA_PRIVATE_KEY& key,
    SCHED_CONFIG& config_loc,
    char* query_string,
        // if nonzero, write value list here; else do insert
    int priority_increase
) {
    DB_RESULT result;
    char base_outfile_name[256];
    char result_template[BLOB_SIZE];
    int retval;

    result.clear();
    initialize_result(result, wu);
    result.priority = result.priority + priority_increase;
    sprintf(result.name, "%s_%s", wu.name, result_name_suffix);
    sprintf(base_outfile_name, "%s_", result.name);
    retval = read_filename(
        result_template_filename, result_template, sizeof(result_template)
    );
    if (retval) {
        fprintf(stderr,
            "Failed to read result template file '%s': %d\n",
            result_template_filename, retval
        );
        return retval;
    }

    retval = process_result_template(
        result_template, key, base_outfile_name, config_loc
    );
    if (retval) {
        fprintf(stderr, "process_result_template() error: %d\n", retval);
    }
    if (strlen(result_template) > sizeof(result.xml_doc_in)-1) {
        fprintf(stderr,
            "result XML doc is too long: %d bytes, max is %d\n",
            strlen(result_template), sizeof(result.xml_doc_in)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    strlcpy(result.xml_doc_in, result_template, sizeof(result.xml_doc_in));

    result.random = lrand48();

    if (query_string) {
        result.db_print_values(query_string);
    } else {
        retval = result.insert();
        if (retval) {
            fprintf(stderr, "result.insert(): %d\n", retval);
            return retval;
        }
    }

    // if using locality scheduling, advertise data file
    // associated with this newly-created result
    //
    if (config_loc.locality_scheduling) {
        const char *datafilename;
        char *last=strstr(result.name, "__");
        if (result.name<last && last<(result.name+255)) {
            datafilename = config.project_path("locality_scheduling/working_set_removal/%s", result.name);
            unlink(datafilename);
            datafilename = config.project_path("locality_scheduling/work_available/%s", result.name);
            boinc_touch_file(datafilename);
        } 
    }
    return 0;
}

// make sure a WU's input files are actually there
//
int check_files(char** infiles, int ninfiles, SCHED_CONFIG& config_loc) {
    int i;
    char path[256];

    for (i=0; i<ninfiles; i++) {
        dir_hier_path(
            infiles[i], config_loc.download_dir, config_loc.uldl_dir_fanout, path
        );
		if (!boinc_file_exists(path)) {
			return 1;
		}

    }
    return 0;
}

int create_work(
    DB_WORKUNIT& wu,
    const char* _wu_template,
    const char* result_template_filename,
    const char* result_template_filepath,
    const char** infiles,
    int ninfiles,
    SCHED_CONFIG& config_loc,
    const char* command_line,
    const char* additional_xml
) {
    int retval;
    char _result_template[BLOB_SIZE];
    char wu_template[BLOB_SIZE];

#if 0
    retval = check_files(infiles, ninfiles, config_loc);
    if (retval) {
        fprintf(stderr, "Missing input file: %s\n", infiles[0]);
        return -1;
    }
#endif

    strcpy(wu_template, _wu_template);
    wu.create_time = time(0);
    retval = process_wu_template(
        wu, wu_template, infiles, ninfiles, config_loc, command_line, additional_xml
    );
    if (retval) {
        fprintf(stderr, "process_wu_template: %d\n", retval);
        return retval;
    }

    retval = read_filename(
        result_template_filepath, _result_template, sizeof(_result_template)
    );
    if (retval) {
        fprintf(stderr, "create_work: can't read result template file %s\n", result_template_filepath);
        return retval;
    }

    if (strlen(result_template_filename) > sizeof(wu.result_template_file)-1) {
        fprintf(stderr, "result template filename is too big: %d bytes, max is %d\n",
            strlen(result_template_filename), sizeof(wu.result_template_file)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    strlcpy(wu.result_template_file, result_template_filename, sizeof(wu.result_template_file));

    if (wu.rsc_fpops_est == 0) {
        fprintf(stderr, "no rsc_fpops_est given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.rsc_fpops_bound == 0) {
        fprintf(stderr, "no rsc_fpops_bound given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.rsc_disk_bound == 0) {
        fprintf(stderr, "no rsc_disk_bound given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.target_nresults == 0) {
        fprintf(stderr, "no target_nresults given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_error_results == 0) {
        fprintf(stderr, "no max_error_results given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_total_results == 0) {
        fprintf(stderr, "no max_total_results given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (wu.max_success_results == 0) {
        fprintf(stderr, "no max_success_results given; can't create job\n");
        return ERR_NO_OPTION;
    }
    if (strstr(wu.name, ASSIGNED_WU_STR)) {
        wu.transition_time = INT_MAX;
    } else {
        wu.transition_time = time(0);
    }
    if (wu.id) {
        retval = wu.update();
        if (retval) {
            fprintf(stderr, "create_work: workunit.update() %d\n", retval);
            return retval;
        }
    } else {
        retval = wu.insert();
        if (retval) {
            fprintf(stderr, "create_work: workunit.insert() %d\n", retval);
            return retval;
        }
        wu.id = boinc_db.insert_id();
    }

    return 0;
}

const char *BOINC_RCSID_b5f8b10eb5 = "$Id$";
