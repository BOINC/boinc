void write_graphics_file(FILE* f, GRAPHICS_INFO& gi) {
    fprintf(f,
        "<graphics_xsize>%d</graphics_xsize>\n"
        "<graphics_ysize>%d</graphics_ysize>\n"
        "<graphics_refresh_period>%f</graphics_refresh_period>\n",
        gi.graphics.xsize,
        gi.graphics.ysize,
        gi.graphics.refresh_period,
    );
}

int parse_graphics_file(FILE* f, GRAPHICS_INFO& gi) {
    char buf[256];
    while (fgets(buf, 256, f)) {
        if (match_tag(buf, "<graphics_info>")) return 0;
        else if (parse_int(buf, "<graphics_xsize>", gi.graphics.xsize)) continue;
        else if (parse_int(buf, "<graphics_ysize>", gi.graphics.ysize)) continue;
        else if (parse_double(buf, "<graphics_refresh_period>", gi.graphics.refresh_period)) continue;
        else fprintf(stderr, "parse_core_file: unrecognized %s", buf);
    }
    return -1;
}

