#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <X11/Shell.h>

// listen for mouse motion in all windows under the given one
//
void listen_for_mouse(Display* display, Window window) {
    Window parent, *children;
    unsigned int nchildren;
    int retval;

    retval = XQueryTree(display, window, &window, &parent, &children, &nchildren);
    if (retval == FALSE) {
        fprintf(stderr, "XQueryTree() failed: %d\n", retval);
        return;
    }

    if (nchildren == 0) return;
    XSelectInput(display, window, PointerMotionMask);
    for (int i=0; i<nchildren; i++) {
        XSelectInput(display, children[i], PointerMotionMask);
        listen_for_mouse(display, children[i]);
    }
    XFree((char *)children);
}

// test program for the above.
//
#if 1
int main(int argc, char **argv) {
    XEvent event;
    int count = 0;
    Display* display;

    const char* hostname = argv[1]?argv[1]:":0";
    display = XOpenDisplay(hostname);
    if (!display) {
        fprintf(stderr, "XOpenDisplay(%s) failed\n", hostname);
        exit(1);
    }
    fprintf(stderr, "Sleeping for 10 sec\n");
    sleep(10);
    fprintf(stderr, "Checking for mouse movement\n");
    listen_for_mouse(display, DefaultRootWindow(display));
    XNextEvent(display, &event);
    if (event.type == MotionNotify) {
        fprintf(stderr, "mouse moved, exiting\n");
        exit(0);
    }
}
#endif
