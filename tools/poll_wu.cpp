// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// a program for polling the status of a WU in the BOINC
// system this is needed for integration with GRID.
//
// the states have  to be the of the type the GRAM framework supplies:
// Globus::GRAM::JobState
//
// Pending: 1
// Active: 2
// Done: 3
// Failed: 4
// Suspended: 5
// Unsubmitted: 6
//
//
// Author: Christian Soettrup

#include "config.h"
#include "boinc_db.h"
#include <iostream>
#include <string>
#include "sched_config.h"

#define VERSION 0.3


DB_WORKUNIT wu;
DB_RESULT result;

void usage(int status){
    if (status !=0){
    }else{
        cout << "pollWU v." << VERSION << endl;
        cout << "usage:" << endl;
        cout << "   pollWU [-h] [-c config_dir] wuname\n";
        cout << "\t-h\tprints this help\n";
        cout << "\t-c config_dir\tread the database configuration from this file\n";
    }
    exit(status);
}

int main(int argc, char** argv){
    int retval,final;
    string wuname;
    string database;
    char buf[256];
    MYSQL_ROW row;
    MYSQL_RES* rp;
    string config_dir = "";
    //parse the input
    if (argc<2){
        usage(0);
        return -1;
    }
    for (int i=1;i<argc-1;i++){
        if (!strcmp(argv[i],"-h")){
            usage(0);
            return -1;
        }
        if (!strcmp(argv[i],"-c")){
            config_dir = argv[i++];
            return -1;
        }
    }
    final=0;
    wuname = argv[argc-1];
    retval = config.parse_file(config_dir.c_str());
    if (retval) {
        fprintf(stderr,"can't read config file\n");
        return -1;
    }
    retval = boinc_db.open(config.db_name, config.db_host,config.db_user, config.db_passwd);

    if (retval) {
        cout << "boinc_db.open failed: " << retval<<endl;;
        return -1;
    }
    sprintf(buf,"where name='%s'",wuname.c_str());
    retval = wu.lookup(buf);
    if (retval){
        cout << "no workunit with name: " << wuname << endl;
        final= -1;
    }else{

        if (!(wu.canonical_resultid==0)){ // a canonical result has been chosen
            cout << "Status:\tDONE\n";
            final= 3;
        }else if(wu.error_mask){ // an error mask has been set
            cout << "Status:\tFAILED\n";
            final= 4;
        }else {
            //now we need to check the results belonging to this workunit to figure out its state.

            sprintf(buf,"select * from result where workunitid='%d'",wu.id);
            retval = boinc_db.do_query(buf);
            if (retval){ //there was no results yet
                cout << "Status:\tUNSUBMITTED\n";
                boinc_db.close();
                return 6;
            }
            rp = mysql_store_result(boinc_db.mysql);
            if (!rp) {
                boinc_db.close();
                return -1;
            }
            while ((row = mysql_fetch_row(rp))&&(final!=2)){
                result.db_parse(row);
                if (result.server_state==RESULT_SERVER_STATE_IN_PROGRESS){
                    cout << "Status:\tACTIVE\n";
                    final=2;
                }
            }
            mysql_free_result(rp);
            if (final==0){
                cout << "Status:\tPENDING\n";
                final=1;
            }
        }
    }
    boinc_db.close();
    return final;
}
