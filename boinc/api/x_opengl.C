#include "x_opengl.h"
#include <stdio.h>

#ifdef HAVE_GL
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "graphics_api.h"

void process_input(Display *dpy);
void refresh( void );
int DrawGLScene(GLvoid);
GLvoid buildFont(GLvoid);

extern int ok_to_draw;
int win_open;
GLuint main_font;
Display *dpy;
Window  win;

static Bool WaitForNotify(Display *d,XEvent *e,char *arg) {
       return(e->type == MapNotify) && (e->xmap.window == (Window)arg);
}

void *p_graphics_loop( void *duff ) {
    XVisualInfo *vi;
    Colormap    cmap;
    XSetWindowAttributes swa;
    GLXContext  cx;
    XEvent  event;
    GRAPHICS_INFO *gfx;
    int attributeList[] = {GLX_RGBA, GLX_DOUBLEBUFFER,
            GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16, None};

    gfx = (GRAPHICS_INFO *)duff;

    /* get a connection */
    dpy = XOpenDisplay(0);

    if (dpy == NULL) {
        printf("couldn't open display\n");
        return 0;
    }

    /* get an appropriate visual */
    vi = glXChooseVisual(dpy,DefaultScreen(dpy),attributeList);
    if (vi == NULL) {
        printf("couldn't find the visual \n");
        return 0;
    }

    /* create a GLX context */
    if (!(cx = glXCreateContext(dpy,vi,0,GL_FALSE))) {
        fprintf(stderr, "glXGetContext failed!\n");
        return 0;
    }

    /* create color map */
    if (!(cmap = XCreateColormap(dpy,RootWindow(dpy,vi->screen),
           vi->visual,AllocNone)))
    {
        fprintf(stderr, "XCreateColormap failed!\n");
        return 0;
    }

    /* create window */
    swa.colormap = cmap;
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask;
    win = XCreateWindow(dpy,RootWindow(dpy,vi->screen),0,0,gfx->xsize,gfx->ysize,0,vi->depth, InputOutput,vi->visual,CWBorderPixel|CWColormap|CWEventMask, &swa);
    XMapWindow(dpy,win);
    XIfEvent(dpy,&event,WaitForNotify,(char*)win);

    if (!(glXMakeCurrent(dpy,win,cx)))
    {
        fprintf(stderr, "glXMakeCurrent failed (window)!\n");
        return 0;
    }
    
    InitGL();

    win_open = true;
    buildFont();
    while(1) {
        process_input(dpy);
    }

    return NULL;
}

// How do we prevent broken pipes and broken connection error messages?
// 
void process_input(Display *dpy) {
    XEvent event;

    if( XPending(dpy) ) {
        do {

	XNextEvent(dpy, &event);
	switch(event.type) {
    case DestroyNotify:
        win_open = false;
        break;
	default:
	    break;
	}
    } while (XPending(dpy));
    }
    if (ok_to_draw && win_open) {
        DrawGLScene();
        glXSwapBuffers(dpy,win);
        ok_to_draw = 0;
    }
}

GLvoid buildFont(GLvoid)
{
    XFontStruct *font;
    
    main_font = glGenLists(256);      /* storage for 256 characters */
    /* load a font with a specific name in "Host Portable Character Encoding" */
    /*font = XLoadQueryFont(dpy,
        "-*-helvetica-bold-r-normal--24-*-*-*-p-*-iso8859-1");*/
    /* this really *should* be available on every X Window System...*/
    font = XLoadQueryFont(dpy, "fixed");
    if (font == NULL)
    {
        printf("Problems loading fonts :-(\n");
	    return; // exit(1);
    }
    /* build 256 display lists out of our font */
    glXUseXFont(font->fid, 0, 256, main_font);
    /* free our XFontStruct since we have our display lists */
    XFreeFont(dpy, font);
}

#endif

