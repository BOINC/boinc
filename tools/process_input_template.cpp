// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// process_input_template():
// fill in the workunit's XML document (wu.xml_doc)
// by scanning the input template, macro-substituting the input files,
// and putting in the command line element and additional XML
//
// Called (only) in create_work.cpp

#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "filesys.h"
#include "md5_file.h"
#include "str_replace.h"

#include "sched_config.h"
#include "sched_util.h"

#include "process_input_template.h"

using std::string;
using std::vector;

// look for file named FILENAME.md5 containing md5sum and length.
// If found, and newer mod time than file,
// read md5 sum and file length from it, and return true.
// Else return false
//
// See checkin notes Dec 30 2004
//
static bool got_md5_info(
    const char *path,
    char *md5data,
    double *nbytes
) {
    char md5name[512];
    struct stat md5stat, filestat;
    char endline='\0';

    sprintf(md5name, "%s.md5", path);

    // get mod times for file
    //
    if (stat(path, &filestat)) {
        return false;
    }

    // get mod time for md5 cache file
    //
    // coverity[toctou]
    if (stat(md5name, &md5stat)) {
        return false;
    }

    // if cached md5 newer, then open it
    FILE *fp=boinc::fopen(md5name, "r");
    if (!fp) {
        return false;
    }

    // read two quantities: md5 sum and length.
    // If we can't read these, or there is MORE stuff in the file,
    // it's not an md5 cache file
    //
    int n = boinc::fscanf(fp, "%s %lf%c", md5data, nbytes, &endline);
    int c = boinc::fgetc(fp);
    boinc::fclose(fp);
    if ((n != 3) || (endline !='\n') || (c != EOF)) {
        boinc::fprintf(stderr, "bad MD5 cache file %s; remove it and stage file again\n", md5name);
        return false;
    }

    // if this is one of our cached md5 files, but it's OLDER than the
    // data file which it supposedly corresponds to, delete it.
    //
    if (md5stat.st_mtime<filestat.st_mtime) {
        unlink(md5name);
        return false;
    }
    return true;
}

// Write file FILENAME.md5 containing md5sum and length
// see checkin notes Dec 30 2004
//
static void write_md5_info(
    const char *path,
    const char *md5,
    double nbytes
) {
    char md5name[512];
    struct stat statbuf;
    int retval;

    // if file already exists with this name, don't touch it.
    //
    // coverity[toctou]
    sprintf(md5name, "%s.md5", path);
    if (!stat(md5name, &statbuf)) {
        return;
    }

    FILE *fp=boinc::fopen(md5name, "w");

    // if can't open the file, give up
    //
    if (!fp) {
        return;
    }

    retval = boinc::fprintf(fp,"%s %.15e\n", md5, nbytes);
    boinc::fclose(fp);

    // if we didn't write properly to the file, delete it.
    //
    if (retval<0) {
        unlink(md5name);
    }

    return;
}

// generate a <file_info> element for workunit XML doc,
// based on a <file_info> in an input template and list of input files
//
// in:
//  xp: parser for input template, pointing after <file_info>
//  var_infiles: list of file descs passed to create_work
//      (the input template may also contain 'constant' input files
//      that aren't passed to create_work)
//
// in/out:
//  nfiles_parsed: increment if not constant file
//
// out:
//  out: append the <file_info> element for the WU XML doc
//  infiles: vector (appended to) of all input files
//
// Actions:
//  generate .md5 file if needed
//
static int process_file_info(
    XML_PARSER& xp, SCHED_CONFIG& config_loc,
    vector<INFILE_DESC> &var_infiles,
    string& out,
    vector<INFILE_DESC> &infiles,
    int& n_var_infiles
) {
    vector<string> urls;
    bool gzip = false;
    int retval, itemp;
    double nbytes, nbytesdef = -1, gzipped_nbytes;
    string md5str, urlstr, tmpstr;
    char physical_name[256];
    char buf[BLOB_SIZE], path[MAXPATHLEN], top_download_path[MAXPATHLEN];
    char gzip_path[MAXPATHLEN];
    char md5[33], url[256], gzipped_url[256], buf2[256];
    INFILE_DESC var_infile, infile;

    strcpy(physical_name, "");

    out += "<file_info>\n";
    while (!xp.get_tag()) {
        if (xp.parse_bool("gzip", gzip)) {
            continue;
        } else if (xp.parse_str("physical_name", physical_name, sizeof(physical_name))) {
            continue;
        } else if (xp.parse_string("url", urlstr)) {
            urls.push_back(urlstr);
            continue;
        } else if (xp.parse_int("number", itemp)) {
            // ignore
            continue;
        } else if (xp.parse_string("md5_cksum", md5str)) {
            continue;
        } else if (xp.parse_double("nbytes", nbytesdef)) {
            continue;
        } else if (xp.parse_double("gzipped_nbytes", gzipped_nbytes)) {
            continue;
        } else if (xp.match_tag("/file_info")) {
            // there are four cases:
            // - normal file
            // - constant file
            //   <physical_name> is given
            // - remote file, specified as create_work() arg
            //   URL, size etc. are given in INFILE_DESC
            // - remote file, specified in template
            //   URL, size etc. are given in template
            //
            if (!strlen(physical_name)) {
                if (n_var_infiles >= (int)var_infiles.size()) {
                    boinc::fprintf(stderr,
                        "Too few var input files given; need at least %d\n",
                        n_var_infiles+1
                    );
                    return ERR_XML_PARSE;
                }
                var_infile = var_infiles[n_var_infiles];
            }

            if (strlen(physical_name)) {
                // constant file case
                //
                dir_hier_path(
                    physical_name, config_loc.download_dir,
                    config_loc.uldl_dir_fanout, path, true
                );
                if (!got_md5_info(path, md5, &nbytes)) {
                    boinc::fprintf(stderr, "missing MD5 info file for %s\n",
                        physical_name
                    );
                    return ERR_FILE_MISSING;
                }

                dir_hier_url(
                    physical_name, config_loc.download_url,
                    config_loc.uldl_dir_fanout, url
                );

                sprintf(buf,
                    "    <name>%s</name>\n"
                    "    <url>%s</url>\n"
                    "    <md5_cksum>%s</md5_cksum>\n"
                    "    <nbytes>%.0f</nbytes>\n",
                    physical_name,
                    url,
                    md5,
                    nbytes
                );
                strcpy(infile.name, physical_name);
                strcpy(infile.md5, md5);
                infile.nbytes = nbytes;
            } else if (nbytesdef > 0) {
                // remote file specified in template
                //
                if (md5str == "" || urlstr == "") {
                    boinc::fprintf(stderr, "All file properties must be defined "
                        "if at least one is defined (url, md5_cksum, nbytes)\n"
                    );
                    return ERR_XML_PARSE;
                }
                if (gzip && !gzipped_nbytes) {
                    boinc::fprintf(stderr, "Must specify gzipped_nbytes\n");
                    return ERR_XML_PARSE;
                }
                urlstr = "";
                for (unsigned int i=0; i<urls.size(); i++) {
                    urlstr += "    <url>" + urls.at(i) + string(var_infile.name) + "</url>\n";
                    if (gzip) {
                        urlstr += "    <gzipped_url>" + urls.at(i) + string(var_infile.name) + ".gz</gzipped_url>\n";
                    }
                }
                sprintf(buf,
                    "    <name>%s</name>\n"
                    "%s"
                    "    <md5_cksum>%s</md5_cksum>\n"
                    "    <nbytes>%.0f</nbytes>\n",
                    var_infile.name,
                    urlstr.c_str(),
                    md5str.c_str(),
                    nbytesdef
                );
                if (gzip) {
                    sprintf(buf2, "    <gzipped_nbytes>%.0f</gzipped_nbytes>\n",
                        gzipped_nbytes
                    );
                    strcat(buf, buf2);
                }
                strcpy(infile.name, var_infile.name);
                strcpy(infile.md5, md5str.c_str());
                infile.nbytes = nbytesdef;
            } else if (var_infile.is_remote) {
                // remote file specified in create_work() arg
                //
                sprintf(buf2, "jf_%s", var_infile.md5);
                sprintf(buf,
                    "    <name>%s</name>\n"
                    "    <url>%s</url>\n"
                    "    <md5_cksum>%s</md5_cksum>\n"
                    "    <nbytes>%.0f</nbytes>\n",
                    buf2,
                    var_infile.url,
                    var_infile.md5,
                    var_infile.nbytes
                );
                strcpy(infile.name, buf2);
                strcpy(infile.md5, var_infile.md5);
                infile.nbytes = var_infile.nbytes;
            } else {
                // normal case. we need to find file size and MD5;
                // stage the file if needed
                //
                dir_hier_path(
                    var_infile.name, config_loc.download_dir,
                    config_loc.uldl_dir_fanout, path, true
                );

                // if file isn't found in hierarchy,
                // look for it at top level and copy
                //
                if (!boinc_file_exists(path)) {
                    sprintf(top_download_path,
                        "%s/%s",config_loc.download_dir, var_infile.name
                    );
                    boinc_copy(top_download_path, path);
                    printf("copy %s to %s\n", top_download_path, path);
                }

                if (!config_loc.cache_md5_info || !got_md5_info(path, md5, &nbytes)) {
                    retval = md5_file(path, md5, nbytes);
                    if (retval) {
                        boinc::fprintf(stderr,
                            "process_input_template: md5_file %s\n",
                            boincerror(retval)
                        );
                        return retval;
                    } else if (config_loc.cache_md5_info) {
                        write_md5_info(path, md5, nbytes);
                    }
                }

                dir_hier_url(
                    var_infile.name, config_loc.download_url,
                    config_loc.uldl_dir_fanout, url
                );

                if (gzip) {
                    sprintf(gzip_path, "%s.gz", path);
                    retval = file_size(gzip_path, gzipped_nbytes);
                    if (retval) {
                        boinc::fprintf(stderr,
                            "process_input_template: missing gzip file %s\n",
                            gzip_path
                        );
                        return ERR_FILE_MISSING;
                    }
                    sprintf(gzipped_url,
                        "    <gzipped_url>%s.gz</gzipped_url>\n"
                        "    <gzipped_nbytes>%.0f</gzipped_nbytes>\n",
                        url, gzipped_nbytes
                    );
                } else {
                    strcpy(gzipped_url, "");
                }

                sprintf(buf,
                    "    <name>%s</name>\n"
                    "    <url>%s</url>\n"
                    "%s"
                    "    <md5_cksum>%s</md5_cksum>\n"
                    "    <nbytes>%.0f</nbytes>\n",
                    var_infile.name,
                    url,
                    gzipped_url,
                    md5,
                    nbytes
                );
                strcpy(infile.name, var_infile.name);
                strcpy(infile.md5, md5);
                infile.nbytes = nbytes;
            }
            infiles.push_back(infile);
            if (!strlen(physical_name)) {
                n_var_infiles++;
            }
            out += buf;
            out += "</file_info>\n";
            break;
        } else {
            // copy any other elements from input template to XML doc
            //
            retval = xp.copy_element(tmpstr);
            if (retval) return retval;
            out += tmpstr;
            out += "\n";
        }
    }
    return 0;
}

// Process the <workunit> section of an input template.
// Copy resource usage and replication params to 'wu'.
// Append XML (the workunit's xml_doc) to 'out'.
// Add the given command line and additional XML.
//
static int process_workunit(
    XML_PARSER& xp, WORKUNIT& wu, string& out,
    vector<INFILE_DESC> &infiles,
    const char* command_line,
    const char* additional_xml
) {
    char buf[256], open_name[256];
    string tmpstr, cmdline;
    int retval, n_file_refs=0, itemp;

    out += "<workunit>\n";
    if (command_line) {
        //fprintf(stderr, "appending command line: %s\n", command_line);
        out += "<command_line>\n";
        out += command_line;
        out += "\n</command_line>\n";
    }
    while (!xp.get_tag()) {
        if (xp.match_tag("/workunit")) {
            if (additional_xml && strlen(additional_xml)) {
                out += additional_xml;
                out += "\n";
            }
            out += "</workunit>";
            break;
        } else if (xp.match_tag("file_ref")) {
            out += "<file_ref>\n";
            bool found_open_name = false, found_copy_file = false;
            if (n_file_refs >= (int)infiles.size()) {
                boinc::fprintf(stderr, "too many <file_ref>s\n");
                return ERR_XML_PARSE;
            }
            INFILE_DESC& id = infiles[n_file_refs];
            sprintf(buf, "    <file_name>%s</file_name>\n", id.name);
            out += buf;

            while (!xp.get_tag()) {
                if (xp.parse_str("open_name", open_name, sizeof(open_name))) {
                    sprintf(buf, "    <open_name>%s</open_name>\n", open_name);
                    out += buf;
                    found_open_name = true;
                    continue;
                } else if (xp.parse_int("file_number", itemp)) {
                } else if (xp.match_tag("copy_file/")) {
                    out += "    <copy_file/>\n";
                    found_copy_file = true;
                    continue;
                } else if (xp.match_tag("/file_ref")) {
                    if (!found_open_name && !found_copy_file) {
                        boinc::fprintf(stderr, "No open name found and copy_file not specified\n");
                        return ERR_XML_PARSE;
                    } else if (!found_open_name && found_copy_file) {
                        sprintf(buf, "    <open_name>%s</open_name>\n", id.name);
                        out += buf;
                    }
                    out += "</file_ref>\n";
                    break;
                } else if (xp.parse_string("file_name", tmpstr)) {
                    boinc::fprintf(stderr, "<file_name> ignored in <file_ref> element.\n");
                    continue;
                } else {
                    retval = xp.copy_element(tmpstr);
                    if (retval) return retval;
                    out += tmpstr;
                    out += "\n";
                }
            }
            n_file_refs++;
        } else if (xp.parse_string("command_line", cmdline)) {
            if (strlen(command_line)) {
                boinc::fprintf(stderr,
                    "Can't specify command line %s; already specified as %s\n",
                    cmdline.c_str(), command_line
                );
                return ERR_XML_PARSE;
            }
            out += "<command_line>\n";
            out += cmdline;
            out += "\n</command_line>\n";
        } else if (xp.parse_double("rsc_fpops_est", wu.rsc_fpops_est)) {
            continue;
        } else if (xp.parse_double("rsc_fpops_bound", wu.rsc_fpops_bound)) {
            continue;
        } else if (xp.parse_double("rsc_memory_bound", wu.rsc_memory_bound)) {
            continue;
        } else if (xp.parse_double("rsc_bandwidth_bound", wu.rsc_bandwidth_bound)) {
            continue;
        } else if (xp.parse_double("rsc_disk_bound", wu.rsc_disk_bound)) {
            continue;
        } else if (xp.parse_int("batch", wu.batch)) {
            continue;
        } else if (xp.parse_int("delay_bound", wu.delay_bound)){
            continue;
        } else if (xp.parse_int("min_quorum", wu.min_quorum)) {
            continue;
        } else if (xp.parse_int("target_nresults", wu.target_nresults)) {
            continue;
        } else if (xp.parse_int("max_error_results", wu.max_error_results)) {
            continue;
        } else if (xp.parse_int("max_total_results", wu.max_total_results)) {
            continue;
        } else if (xp.parse_int("max_success_results", wu.max_success_results)) {
            continue;
        } else if (xp.parse_int("size_class", wu.size_class)) {
            continue;
        } else {
            retval = xp.copy_element(tmpstr);
            if (retval) return retval;
            out += tmpstr;
            out += "\n";
        }
    }

    // fill in possibly missing parameters
    //
    if (wu.target_nresults > wu.max_success_results) {
        wu.max_success_results = wu.target_nresults;
    }
    if (wu.target_nresults > wu.max_total_results) {
        wu.max_total_results = wu.target_nresults;
    }

    if (n_file_refs != (int)infiles.size()) {
        boinc::fprintf(stderr, "#file refs != #file infos\n");
        return ERR_XML_PARSE;
    }
    return 0;
}

// fill in the workunit's XML document (wu.xml_doc)
// by scanning the input template, macro-substituting the input files,
// and putting in the command line element and additional XML
//
int process_input_template(
    WORKUNIT& wu,
    char* tmplate,
    vector<INFILE_DESC> &var_infiles,
        // files passed as args to create_work
    SCHED_CONFIG& config_loc,
    const char* command_line,
    const char* additional_xml
) {
    string out;
    int retval;
    bool found_workunit=false;
    vector<INFILE_DESC> infiles;
        // this gets filled in as we scan <file_info>s
    int n_var_infiles=0;
        // number of non-constant infiles scanned
        // this is an index into var_infiles

    // "out" is the XML we're creating
    //
    out = "";
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_buf_read(tmplate);
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("input_template")) continue;
        if (xp.match_tag("/input_template")) continue;
        if (xp.match_tag("file_info")) {
            retval = process_file_info(
                xp, config_loc, var_infiles,
                out, infiles, n_var_infiles
            );
            if (retval) return retval;
        } else if (xp.match_tag("workunit")) {
            found_workunit = true;
            retval = process_workunit(
                xp, wu, out, infiles, command_line, additional_xml
            );
            if (retval) return retval;
        }
    }
    if (!found_workunit) {
        boinc::fprintf(stderr, "process_input_template: bad WU template - no <workunit>\n");
        return ERR_XML_PARSE;
    }
    if (n_var_infiles != (int)var_infiles.size()) {
        boinc::fprintf(stderr,
            "process_input_template: %d input files given, but template has %d\n",
            (int)var_infiles.size(), n_var_infiles
        );
        return ERR_XML_PARSE;
    }
    if (out.size() > sizeof(wu.xml_doc)-1) {
        boinc::fprintf(stderr,
            "create_work: WU XML field is too long (%d bytes; max is %d)\n",
            (int)out.size(), (int)sizeof(wu.xml_doc)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    //fprintf(stderr, "copying to xml_doc: %s\n", out.c_str());
    safe_strcpy(wu.xml_doc, out.c_str());
    return 0;
}

#ifdef TEST
SCHED_CONFIG config_loc;

int main(int, char**) {
    WORKUNIT wu;
    vector<INFILE_DESC> var_infiles;
    char* tmplate;
    const char* command_line = "sample command line";
    const char* additional_xml;

    config_loc.parse_file("");

    read_file_malloc("template_in", tmplate);
    INFILE_DESC id;
#if 1
    strcpy(id.name, "input");
    var_infiles.push_back(id);
#endif

#if 0
    strcpy(id.name, "input");
    id.is_remote = true;
    strcpy(id.url, "http://blah.foo");
    strcpy(id.md5, "sfslkjsdlfkj");
    id.nbytes = 234;
    var_infiles.push_back(id);
#endif

    int retval = process_input_template(
        wu,
        tmplate,
        var_infiles,
        config_loc,
        command_line,
        additional_xml
    );
    if (retval) {
        printf("process_input_template returned %d\n", retval);
    } else {
        printf("xml_doc:\n%s\n", wu.xml_doc);
    }
}
#endif
