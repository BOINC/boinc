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

// crash and burn

#include <cstdio>
#include <cctype>

int main() {
  char * hello = (char *) 100;
  int c, n=0;
  fprintf(stderr, "APP: upper_case starting\n");
  printf("%s",hello);
    while (1) {
        c = getchar();
        if (c == EOF) break;
        c = toupper(c);
        putchar(c);
        n++;
    }
    fprintf(stderr, "APP: upper_case ending, wrote %d chars\n", n);
}

#ifdef __GNUC__
static volatile const char  __attribute__((unused)) *BOINCrcsid="$Id$";
#else
static volatile const char *BOINCrcsid="$Id$";
#endif
