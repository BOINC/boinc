#include <stdio.h>

#define MODE_NO_GRAPHICS        -1
#define MODE_WINDOW             0
#define MODE_FULLSCREEN         1

struct GRAPHICS_INFO {
    int xsize;
    int ysize;
    int graphics_mode;
    double refresh_period;
};

typedef struct GRAPHICS_INFO GRAPHICS_INFO;

struct APP_OUT_GRAPHICS {
};

int write_graphics_file(FILE* f, GRAPHICS_INFO* gi);
int parse_graphics_file(FILE* f, GRAPHICS_INFO* gi);
int boinc_init_opengl();
int boinc_finish_opengl();

#ifdef BOINC_APP_GRAPHICS
GLvoid glPrint(GLuint font, const char *fmt, ...);
bool app_render(int xs, int ys, double time_of_day);
#endif
