// utility program for projects that use homogeneous redundancy.
// Converts old-style info (in result table)
// to new style (using workseq_next field of workunit)

#include "config.h"
#include "boinc_db.h"

const int unspec = 0;
const int nocpu = 1;
const int Intel = 2;
const int AMD = 3;
const int Macintosh = 4;

const int noos = 128;
const int Linux = 256;
const int Windows = 384;
const int Darwin = 512;
const int SunOS = 640;

int OS(DB_HOST& host){
    if ( strstr(host.os_name, "Linux") != NULL ) return Linux;
    else if( strstr(host.os_name, "Windows") != NULL ) return Windows;
    else if( strstr(host.os_name, "Darwin") != NULL ) return Darwin;
    else if( strstr(host.os_name, "SunOS") != NULL ) return SunOS;
    else return noos;
};

int CPU(DB_HOST& host){
    if ( strstr(host.p_vendor, "Intel") != NULL ) return Intel;
    else if( strstr(host.p_vendor, "AMD") != NULL ) return AMD;
    else if( strstr(host.p_vendor, "Macintosh") != NULL ) return Macintosh;
    else return nocpu;
};

int main() {
  if ( boinc_db.open("predictor", "boinc", NULL, NULL) ) {
    printf("Open failed\n");
    return 0;
  }

  DB_WORKUNIT workunit;
  char buf[256];

  while (!workunit.enumerate()) {

    printf("workunit %d wsn %d\n", workunit.id, workunit.workseq_next);
    DB_RESULT result;
    sprintf(buf, "where workunitid=%d", workunit.id);
    if ( !result.enumerate(buf) ) {
      DB_HOST host;
      sprintf(buf, "where id=%d", result.hostid);
      if ( !host.enumerate(buf) ) {
        workunit.workseq_next = OS(host) + CPU(host);
        if ( workunit.update() ) printf("Update failed!\n");
      }
    }

  }

};
