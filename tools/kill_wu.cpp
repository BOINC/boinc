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

// a program for killing a WU in the BOINC
// system this is needed for integration with GRID.
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
        cout << "killWU v." << VERSION << endl;
        cout << "usage:" << endl;
        cout << "   killWU [-h] [-c config_dir] wuname\n";
        cout << "\t-h\tprints this help\n";
        cout << "\t-c config_dir\tread the database configuration from this file\n";
    }
    exit(status);
}

int main(int argc, char** argv){
    int retval,final;
    string wuname;
    char buf[256];
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
    wuname = argv[argc-1];
    final =0;
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
    } else {
        //the workunit state is set as deleted
        sprintf(buf,"update workunit set error_mask=error_mask|16 where id='%d'",wu.id);
        boinc_db.do_query(buf);
        // and the results are set as server state over and outcome state
        // result not needed if the result is unsent
        // otherwise it is left to finish normally
        //
        sprintf(buf,"update result set server_state=5,outcome=5 where server_state=2 and workunitid='%d'",wu.id);
        boinc_db.do_query(buf);
        cout << "Workunit with name: " << wuname << " killed."endl;
    }

    boinc_db.close();
    return final;
}
