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

//  make_app_icon_h.C
// Utility to convert *.icns file into app_icon.h file for use 
// with api/setMacIcon() in science applications.

#include <stdio.h>

int main(int argc, char** argv) {
    int retval = 0;
    FILE *inFile, *outFile;
    int count, c;

    if (argc < 3) {
        puts ("usage: make_app_icon_h source_path dest_path\n");
        return 0;
    }
    
    inFile = fopen(argv[1], "rb");
    if (inFile == NULL) {
        printf ("Couldn't open input file %s\n", argv[1]);
        return 0;
    }
    
    outFile = fopen(argv[2], "w");
    if (inFile == NULL) {
        printf ("Couldn't create output file %s\n", argv[2]);
        fclose(inFile);
        return 0;
    }
    
    fputs("char MacAppIconData[] = {\n\t", outFile);
    count = 16;
    c = getc(inFile);
    if (c == EOF) {
        printf ("No data in input file %s\n", argv[1]);
        fclose(inFile);
        fclose(outFile);
        return 0;
    }
    
    fprintf(outFile, "0X%02X", c);
    
    while ((c = getc(inFile)) != EOF) {
        if (--count)
            fputs(",", outFile);
        else {
            fputs(",\n\t", outFile);
            count = 16;
        }
        
        fprintf(outFile, "0X%02X", c);
    }
    
    fputs("\n};\n", outFile);
    fclose(inFile);
    fclose(outFile);
    
    return retval;
}
    
