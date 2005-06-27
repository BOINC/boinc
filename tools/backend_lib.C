// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <cstdlib>
#include <cstring>
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
#include "util.h"
#include "filesys.h"
#include "sched_util.h"

#include "backend_lib.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#else
#define FCGI_ToFILE(x) (x)
#endif

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
    FILE* f = fopen(path, "r");
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
  
    FILE *fp;
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
    if (!(fp=fopen(md5name, "r")))
        return retval;
  
    // read two quantities: md5 sum and length.  If we can't read
    // these, or there is MORE stuff in the file' it's not an md5
    // cache file
    if (3 == fscanf(FCGI_ToFILE(fp), "%s %lf%c", md5data, nbytes, &endline) &&
        endline=='\n' &&
        EOF==fgetc(FCGI_ToFILE(fp))
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
    FILE *fp;
    char md5name[512];
    struct stat statbuf;
    int retval;
    
    // if file already exists with this name, don't touch it.
    sprintf(md5name, "%s.md5", path);
    if (!stat(md5name, &statbuf))
        return;
 
    // if can't open the file, give up
    if (!(fp=fopen(md5name, "w")))
        return;
  
    retval=fprintf(fp,"%s %.15e\n", md5, nbytes);
    fclose(fp);

    // if we didn't write properly to the file, delete it.
    if (retval<0)
        unlink(md5name);
  
    return;
}


// process WU template
//
static int process_wu_template(
    WORKUNIT& wu,
    char* tmplate,
    const char** infiles,
    int ninfiles,
    SCHED_CONFIG& config
) {
    char* p;
    char buf[LARGE_BLOB_SIZE], md5[33], path[256], url[256], top_download_path[256];
    std::string out;
    int retval, file_number;
    double nbytes;
    char open_name[256];
    bool found=false;

    out = "";
    for (p=strtok(tmplate, "\n"); p; p=strtok(0, "\n")) {
        if (match_tag(p, "<file_info>")) {
            file_number = -1;
            out += "<file_info>\n";
            while (1) {
                p = strtok(0, "\n");
                if (!p) break;
                if (parse_int(p, "<number>", file_number)) {
                    continue;
                } else if (match_tag(p, "</file_info>")) {
                    if (file_number < 0) {
                        fprintf(stderr, "No file number found\n");
                        return ERR_XML_PARSE;
                    }
                    if (file_number >= ninfiles) {
                        fprintf(stderr, "Too few input files given; need at least %d\n", file_number+1);
                        return ERR_XML_PARSE;
                    }
                    dir_hier_path(
                        infiles[file_number], config.download_dir,
                        config.uldl_dir_fanout, true,
						path, true
                    );

                    // if file isn't found in hierarchy,
                    // look for it at top level and copy
                    //
                    if (!boinc_file_exists(path)) {
                        sprintf(top_download_path,
                            "%s/%s",config.download_dir,
                            infiles[file_number]
                        );
                        boinc_copy(top_download_path, path);
                    }

                    if (!config.cache_md5_info || !got_md5_info(path, md5, &nbytes)) {

                        retval = md5_file(path, md5, nbytes);
                        if (retval) {
                            fprintf(stderr, "process_wu_template: md5_file %d\n", retval);
                            return retval;
                        }
                        else if (config.cache_md5_info) {
                            write_md5_info(path, md5, nbytes);
                        }
                    }

                    dir_hier_url(
                        infiles[file_number], config.download_url,
                        config.uldl_dir_fanout, true,
						url
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
        } else if (match_tag(p, "</workunit>")) {
            out += "</workunit>\n";
        } else if (match_tag(p, "<file_ref>")) {
            file_number = -1;
            while (1) {
                p = strtok(0, "\n");
                if (!p) break;
                if (parse_int(p, "<file_number>", file_number)) {
                    continue;
                } else if (parse_str(p, "<open_name>", open_name, sizeof(open_name))) {
                    continue;
                } else if (match_tag(p, "</file_ref>")) {
                    if (file_number < 0) {
                        fprintf(stderr, "No file number found\n");
                        return ERR_XML_PARSE;
                    }
                    sprintf(buf,
                        "<file_ref>\n"
                        "    <file_name>%s</file_name>\n"
                        "    <open_name>%s</open_name>\n"
                        "</file_ref>\n",
                        infiles[file_number],
                        open_name
                    );
                    out += buf;
                    break;
                }
            }
        } else if (parse_double(p, "<rsc_fpops_est>", wu.rsc_fpops_est)) {
            continue;
        } else if (parse_double(p, "<rsc_fpops_bound>", wu.rsc_fpops_bound)) {
            continue;
        } else if (parse_double(p, "<rsc_memory_bound>", wu.rsc_memory_bound)) {
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
        fprintf(stderr, "create_work: bad WU template - no <workunit>\n");
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

// Set the time-varying fields of a result to their initial state.
// This is used to create clones of existing results,
// so set only the time-varying fields
//
void initialize_result(DB_RESULT& result, TRANSITIONER_ITEM& wu) {
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
}

// Create a new result for the given WU.
// This is called ONLY from the transitioner
//
int create_result(
    TRANSITIONER_ITEM& wu,
    char* result_template_filename,
    char* result_name_suffix,
    R_RSA_PRIVATE_KEY& key,
    SCHED_CONFIG& config,
    char* query_string
        // if nonzero, write value list here; else do insert
) {
    DB_RESULT result;
    char base_outfile_name[256];
    char result_template[LARGE_BLOB_SIZE];
    int retval;

    result.clear();
    initialize_result(result, wu);
    sprintf(result.name, "%s_%s", wu.name, result_name_suffix);
    sprintf(base_outfile_name, "%s_", result.name);

    retval = read_filename(result_template_filename, result_template, sizeof(result_template));
    if (retval) {
        fprintf(stderr,
            "Failed to read result template file '%s': %d\n",
            result_template_filename, retval
        );
        return retval;
    }

    retval = process_result_template(
        result_template,
        key,
        base_outfile_name,
        config
    );
    if (strlen(result_template) > sizeof(result.xml_doc_in)-1) {
        fprintf(stderr,
            "result XML doc is too long: %d bytes, max is %d\n",
            strlen(result_template), sizeof(result.xml_doc_in)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    safe_strncpy(result.xml_doc_in, result_template, sizeof(result.xml_doc_in));

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
    if (config.locality_scheduling) {
        char datafilename[512];
        char *last=strstr(result.name, "__");
        if (result.name<last && last<(result.name+255)) {
            sprintf(datafilename, "../locality_scheduling/working_set_removal/");
            strncat(datafilename, result.name, last-result.name);
            unlink(datafilename);
            sprintf(datafilename, "../locality_scheduling/work_available/");
            strncat(datafilename, result.name, last-result.name);
            boinc_touch_file(datafilename);
        } 
    }
    return 0;
}

// make sure a WU's input files are actually there
//
int check_files(char** infiles, int ninfiles, SCHED_CONFIG& config) {
    int i;
    char path[256];

    for (i=0; i<ninfiles; i++) {
        dir_hier_path(
            infiles[i], config.download_dir, config.uldl_dir_fanout, true, path
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
    SCHED_CONFIG& config
) {
    int retval;
    char _result_template[LARGE_BLOB_SIZE];
    char wu_template[LARGE_BLOB_SIZE];

#if 0
    retval = check_files(infiles, ninfiles, config);
    if (retval) {
        fprintf(stderr, "Missing input file: %s\n", infiles[0]);
        return -1;
    }
#endif

    strcpy(wu_template, _wu_template);
    wu.create_time = time(0);
    retval = process_wu_template(
        wu, wu_template, infiles, ninfiles, config
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
    safe_strncpy(wu.result_template_file, result_template_filename, sizeof(wu.result_template_file));

    wu.transition_time = time(0);
    retval = wu.insert();
    if (retval) {
        fprintf(stderr, "create_work: workunit.insert() %d\n", retval);
        return retval;
    }
    wu.id = boinc_db.insert_id();

    return 0;
}

const char *BOINC_RCSID_b5f8b10eb5 = "$Id$";
