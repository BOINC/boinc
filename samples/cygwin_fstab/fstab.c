// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2016 University of California
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

// fstab PROJECT_DIR     Create an etc/fstab file for a cygwin environment
// see: http://boinc.berkeley.edu/trac/wiki/WrapperApp

// cmdline options:
// PROJECT_DIR     an absolute (Windows) path to the project directory
//
// Creates a file "etc\fstab" which allows BOINC XML soft links to properly
// refer to files in the project directory if using an application compiled
// with a cygwin environment on a Windows host

// For example the command
//   fstab "C:\BOINC Data\projects\my.project.dir"
// will produce a file "etc\fstab" containing the line
//   C:/BOINC\040Data/projects/my.project.dir /projects/my.project.dir dummy binary 0 0
// with the CWD regarded as "root" where ".." and "." all point to "/", file locations of the form
//   ../../projects/my.project.dir/file
// can now be properly used


#include <stdio.h>
#include <string.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

char path[PATH_MAX];

int main(int argc, char*argv[]) {
  char* c = argv[1];
  char *n, *proj;
  unsigned int i=0;

  // replace all "\" with "/" in argv[1]
  while (*c) {
    if (*c == '\\') {
      *c = '/';
    }
    c++;
  }

  // point proj to second-last "/"
  while (i<2) {
    c--;
    if (*c == '/')
      i++;
  }
  proj = c;

  // copy argv[1] to path, replacing " " with "\040"
  c = argv[1];
  while ((n = strchr(c,' '))) {
    *n = '\0'; // end the string to copy here
    strcat(path, c);
    strcat(path, "\\040");
    *n = ' '; // restore the original value
    c = n + 1;
  }
  strcat(path, c);

#ifdef _WIN32
  CreateDirectory ("etc", NULL);
  FILE* fp = fopen("etc\\fstab", "w");
#else
  mkdir("etc",0755);;
  FILE* fp = fopen("etc/fstab", "w");
#endif
  if (!fp)
    return(1);
  fprintf(fp, "%s /project dummy binary 0 0\n", path);
  fprintf(fp, "%s %s dummy binary 0 0\n", path, proj);
  fclose(fp);
  return(0);
}

