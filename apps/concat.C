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

void file_append(FILE* in, FILE* out) {
    char buf[1024];
    int n;

    while (1) {
        n = fread(buf, 1, 1024, in);
        if (n == 0) break;
        fwrite(buf, 1, n, out);
    }
}

int main(int argc, char** argv) {
    FILE* in, *out;
    int i;

    fprintf(stderr, "APP: concat: starting, argc %d\n", argc);
    for (i=0; i<argc; i++) {
        fprintf(stderr, "APP: concat: argv[%d] is %s\n", i, argv[i]);
    }
    out = fopen(argv[argc-1], "w");
    if (!out) {
        fprintf(stderr, "APP: concat: can't open out file %s\n", argv[argc-1]);
        exit(1);
    }
    for (i=1; i<argc-1; i++) {
        in = fopen(argv[i], "r");
        if (!in) {
            fprintf(stderr, "APP: concat: can't open in file %s\n", argv[i]);
            exit(1);
        }
        file_append(in, out);
        fclose(in);
    }
    fclose(out);
    fprintf(stderr, "APP: concat: done\n");
}
