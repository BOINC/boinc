#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gfx_interface.h"
#include "parse.h"

int GFX_INTERFACE::write_prefs(FILE *fout) {
    fprintf( fout, 
        "<gfx_prefs>\n"
        "    <gfx_width>%d</gfx_width>\n"
        "    <gfx_height>%d</gfx_height>\n"
        "    <gfx_depth>%d</gfx_depth>\n"
        "    <gfx_fps>%f</gfx_fps>\n"
        "    <gfx_shmem_key>%d</gfx_shmem_key>\n"
        "</gfx_prefs>",
        width, height, depth, fps, shared_mem_key
    );
    return 0;
}

int GFX_INTERFACE::parse(FILE* fin) {
    char buf[256];

    fgets(buf, 256, fin);
    if (!match_tag(buf, "<gfx_prefs>")) {
        fprintf(stderr, "GFX_INTERFACE::parse(): bad first tag %s\n", buf);
        return -1;
    }

    while(fgets(buf,256,fin)) {
        if (match_tag(buf, "</gfx_prefs>")) return 0;
        else if (parse_int(buf, "<gfx_width>", width)) continue;
        else if (parse_int(buf, "<gfx_height>", height)) continue;
        else if (parse_int(buf, "<gfx_depth>", depth)) continue;
        else if (parse_double(buf, "<gfx_fps>", fps)) continue;
        else if (parse_int(buf, "<gfx_shmem_key>", shared_mem_key)) continue;
        else fprintf(stderr, "GFX_INTERFACE::parse(): unrecognized: %s\n", buf);
    }
    return -1;
}

int GFX_INTERFACE::open_parse_prefs( void ) {
    FILE *prefs;
    int err;

    prefs = fopen( "prefs.xml", "r" );
    if( prefs == NULL )
        return -1;

    rewind( prefs );
    err = parse( prefs );
    fclose( prefs );

    return err;
}
