#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define DRAW_OFFSCREEN      0x01
#define DRAW_ONSCREEN       0x02

// Status flags
#define ERROR               -1
#define WAITING_FOR_APP     0
#define APP_READY           1
#define APP_REQUEST         2
#define BUFFER_DIRTY        3
#define BUFFER_CLEAN        4

#ifndef _GFX_INTERFACE
#define _GFX_INTERFACE

struct GFX_INTERFACE {
        int width, height;      // in pixels
        int depth;              // in bits (8. 16, 24, etc)
        double fps;             // max frames per second to draw (can be < 1)
        int draw_offscreen;     // should we draw offscreen/onscreen/anything
        int shared_mem_key;     // key for graphics shared memory region
        int offscreen_dirty;    // is the offscreen buffer ready for blitting?

        int status;             // status of graphics rendering (see flags above)
        char *graphicsData;
        int shared_mem_allocated;

        int write_prefs(FILE*);
        int parse(FILE*);
        int open_parse_prefs();
};

#endif _GFX_INTERFACE
