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

// concat file1 ... filen outfile
//
// concatenate files, write to outfile

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "boinc_api.h"

#define CHECKPOINT_FILE "concat_slow_state"

int do_checkpoint(MFILE& mf, int filenum, int nchars ) {
    int retval;
    char resolved_name[512],res_name2[512];

    boinc_resolve_filename( "temp", resolved_name );
    FILE* f = fopen(resolved_name, "w");
    if (!f) return 1;
    fprintf(f, "%d %d", filenum, nchars);
    fclose(f);

    fprintf(stderr, "APP: concat_slow checkpointing\n");

    retval = mf.flush();
    if (retval) return retval;
    boinc_resolve_filename( CHECKPOINT_FILE, res_name2 );
    retval = boinc_rename(resolved_name, res_name2);
    if (retval) return retval;

    return 0;
}

void file_append(FILE* in, MFILE &out, int skip, int filenum) {
    char buf[1];
    int n,nread,retval;

    fseek( in, skip, SEEK_SET );
    nread = skip;

    while (1) {
        n = fread(buf, 1, 1, in);
        if (n == 0) break;
        out.write(buf, 1, n);
        nread += n;
        fprintf( stderr, "Wrote 1 char.\n" );
        // Burn cycles
        for( n=0;n<5000000;n++ )
            retval += n;
        if( retval == 0 )
            n = 1;

        if( boinc_time_to_checkpoint() ) {
            fprintf( stderr, "Checkpoint.\n" );
            retval = do_checkpoint( out, filenum, nread );
            if( retval ) {
                fprintf( stderr, "APP: concat_slow checkpoint failed %d\n", retval );
                exit(1);
            }
            boinc_checkpoint_completed();
        }
        sleep(1);
    }
}

int main(int argc, char** argv) {
    FILE* in, *state;
    MFILE out;
    char file_name[512];
    int i;
    int file_num,nchars,retval;
    char *mode;

    boinc_init();
    fprintf(stderr, "APP: concat: starting, argc %d\n", argc);
    for (i=0; i<argc; i++) {
        fprintf(stderr, "APP: concat: argv[%d] is %s\n", i, argv[i]);
    }
    boinc_resolve_filename( CHECKPOINT_FILE, file_name );
    state = fopen( file_name, "r" );
    if( state ) {
        fscanf( state, "%d %d", &file_num, &nchars );
        mode = "a";
    } else {
        file_num = 1;
        nchars = 0;
        mode = "w";
    }
    boinc_resolve_filename( argv[argc-1], file_name );
    fprintf( stderr, "res: %s\n", file_name );
    retval = out.open(file_name, mode);
    if (retval) {
        fprintf(stderr, "APP: concat: can't open out file %s\n", argv[argc-1]);
        exit(1);
    }
    for (i=file_num; i<argc-1; i++) {
        boinc_resolve_filename( argv[i], file_name );
        fprintf( stderr, "res: %s\n", file_name );
        in = fopen(file_name, "r");
        if (!in) {
            fprintf(stderr, "APP: concat: can't open in file %s\n", argv[i]);
            exit(1);
        }
        file_append(in, out, nchars, i);
        nchars = 0;
        fclose(in);
    }
    out.close();
    fprintf(stderr, "APP: concat: done\n");
    boinc_finish(0);
    return 0;
}
