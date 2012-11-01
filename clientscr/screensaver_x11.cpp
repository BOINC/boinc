// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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
//
//
// This is a XScreenSaver compatible BOINC screensaver for Unix/X11.
//
// To use this screensaver, please add the following to the 'programs'
// preference in your .xscreensaver file:
//
// GL:  boincscr -root  \n\
//
// If your BOINC directory differs from /var/lib/boinc-client, you can use
// the -boinc_dir command line argument.
//
// When run, this screensaver connects to the BOINC client via RPC, asks for
// graphics providing tasks and starts a random graphics application. The window
// created by the graphics appliacation is then searched for using X11 window
// properties, such as "WM_COMMAND". Not every graphics application seems to
// support this, but this method has been successfully tested with Einstein@Home
// and climateprediction.net. When the graphics application window
// has been found, it will be embedded into the XScreenSaver-provided
// fullscreen-window, the root window, the preview window or a newly created
// window, depending on the environment, using the XEMBED method.

// These headers must be included before other
// headers to avoid compiler errors.
#include "gui_rpc_client.h"
#include "boinc_api.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include <pthread.h>
#include <unistd.h>

extern "C" {
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
}

/// A screensaver window class.
/** Creates a window in the size of the given parent.
    When not in windowed mode, it is overwrite-redirected.
    It shows the text "screensaver loading" when redrwan.
    A client window may be xembedded into it, which will also be resized.
*/
class scr_window {
private:
  /// X server connection
  xcb_connection_t *con;

  /// X screen
  xcb_screen_t *scr;
  
  /// window id
  xcb_window_t win; 

  /// text graphics context id
  xcb_gcontext_t txt_gc;

  /// text font
  xcb_font_t font;

  /// parent window size;
  uint16_t width;
  uint16_t height;

  /// client window
  xcb_window_t client_win;

  /// text to be drawn
  std::string text;

  /// Small helper function to convert std::string to xcb_char2b_t*
  /** Remember to delete[] the returned string.
   */
  xcb_char2b_t *char2b(std::string str) {
    xcb_char2b_t *s = new xcb_char2b_t[str.size()];
    if(!s) return NULL;
    for(int c = 0; c < str.size(); c++) {
        s[c].byte1 = '\0';
        s[c].byte2 = str[c];
    }
    return s;
}

public:
  /// Constructs the screensaver window.
  /** \param connection connection to a X sever
      \param screen screen, where the window will be created
      \param parent parent window (0 for screen->root)
      \param windowed means no fullscreen
  */
  scr_window(xcb_connection_t *connection, xcb_screen_t *screen,
             xcb_window_t parent, bool windowed)
    : con(connection), scr(screen), client_win(0)
  {
    uint32_t mask;
    uint32_t values[3];
    xcb_void_cookie_t cookie;
    xcb_generic_error_t *error = NULL;
    if(!parent) parent = scr->root;

    // use parent window size when not in windowed mode
    if(!windowed) {
        xcb_get_geometry_cookie_t geo_cookie = xcb_get_geometry(con, parent);
        xcb_get_geometry_reply_t *reply =
          xcb_get_geometry_reply(con, geo_cookie, &error);
        if(error) {
            std::cerr << "Could not get parent window geometry." << std::endl;
            exit(1);
        }
        width = reply->width;
        height = reply->height;
        free(reply);
    } else {
        // use some defaults in windowed mode
        width = 640;
        height = 480;
    }

    if(windowed) {
        // create a black maybe override-redirected window
        // and register for expose and resize events.
        mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
        values[0] = scr->black_pixel;
        values[1] = !windowed; // only if in fullscreen mode, otherwise normal window
        values[2] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
        win = xcb_generate_id(con);
        cookie = xcb_create_window_checked(
            con, XCB_COPY_FROM_PARENT, win,
            parent, 0, 0, width, height, 0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            scr->root_visual, mask, values
        );
        error = xcb_request_check(con, cookie);
        if(error) {
            std::cerr << "Could not create window." << std::endl;
            exit(1);
        }

        // map the window on the screen
        xcb_map_window(con, win);
        xcb_flush(con);
    } else {
        // directly use the parent window
        win = parent;

        // cahnge window attributes like above
        mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
        values[0] = scr->black_pixel;
        values[1] = XCB_EVENT_MASK_EXPOSURE;
        xcb_void_cookie_t cookie =
          xcb_change_window_attributes(con, win, mask, values);

        xcb_generic_error_t *error = xcb_request_check(con, cookie);
        if(error) {
            std::cerr << "Could not configure window." << std::endl;
            exit(1);
        }
    } 

    // open a font. "fixed" should hopefully be available everywhere
    font = xcb_generate_id(con);
    std::string font_name = "fixed";
    cookie = xcb_open_font_checked(con, font, font_name.size(),
                                   font_name.c_str());
    error = xcb_request_check(con, cookie);
    if(error) {
        std::cerr << "Could not open font " << font_name << "." << std::endl;
        exit(1);
    }

    // allocate white text graphics context with above font
    txt_gc = xcb_generate_id(con);
    mask = XCB_GC_FOREGROUND | XCB_GC_FONT;
    values[0] = scr->white_pixel;
    values[1] = font;
    cookie = xcb_create_gc_checked(con, txt_gc, win, mask, values);
    error = xcb_request_check(con, cookie);
    if(error) {
        std::cerr << "Could not create graphics context." << std::endl;
        exit(1);
    }
  }

  /// Destructor
  ~scr_window() {
    // clean up
    xcb_unmap_window(con, win);
    xcb_destroy_window(con, win);
  }

  /// Returns the window id.
  xcb_window_t get_window_id() {
    return win;
  }

  /// Sets the text to be drawn
  void set_text(std::string txt) {
    text = txt;
    redraw();
  }

  /// Redraws the window.
  /** Draws a black background with white text.
      Should be calld when an expose event is received.
  */
  void redraw() {
    // convert the text to be displayed.
    xcb_char2b_t *str = char2b(text);

    // get the dimensions of the text
    xcb_query_text_extents_cookie_t cookie = 
      xcb_query_text_extents(con, font, text.size(), str);
    delete[] str;

    xcb_generic_error_t *error;
    xcb_query_text_extents_reply_t *reply =
      xcb_query_text_extents_reply(con, cookie, &error);
    if(error)
      {
        std::cerr << "Could not query text extents." << std::endl;
        exit(1);
      }

    // draw the text in the middle of the window.
    xcb_image_text_8(con, text.size(), win, txt_gc,
                     width/2 - reply->overall_width/2, height/2, text.c_str());

    free(reply);
    xcb_flush(con);
  }

  /// Notifies the window on resizes and resizes the client, if any.
  /** Should be called when a configure notify event is received.
   */
  void resize(uint16_t w, uint16_t h) {
    width = w;
    height = h;
    
    // resize client window, if any.
    if(client_win) {
        // moving the client back to (0, 0) is required when maximizing
        uint32_t values[4] = { 0, 0, width, height };
        uint32_t mask = XCB_CONFIG_WINDOW_X |XCB_CONFIG_WINDOW_Y |
          XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
        xcb_void_cookie_t cookie =
          xcb_configure_window_checked(con, client_win, mask, values);

        xcb_generic_error_t *error = xcb_request_check(con, cookie);
        if(error)
          {
            std::cerr << "Could not resize client." << std::endl;
            exit(1);
        }
    }
}

  /// Xembeds a X window
  void xembed(xcb_window_t client) {
    client_win = client;
    uint32_t values[4];
    uint32_t mask;
    xcb_void_cookie_t cookie;
    xcb_generic_error_t *error;

    // order of the following operations is important here!

    // move window below other windows
    values[0] = XCB_STACK_MODE_BELOW;
    mask = XCB_CONFIG_WINDOW_STACK_MODE;
    cookie = xcb_configure_window_checked(con, client_win, mask, values);

    error = xcb_request_check(con, cookie);
    if(error) {
        std::cerr << "Could not change client attributes." << std::endl;
        exit(1);
    }

    // reparent client
    cookie = xcb_reparent_window_checked(con, client_win, win, 0, 0);

    error = xcb_request_check(con, cookie);
    if(error) {
        std::cerr << "Could not reparent client." << std::endl;
        exit(1);
    }
    
    // move and resize client window
    values[0] = 0;
    values[1] = 0;
    values[2] = width;
    values[3] = height;
    mask = XCB_CONFIG_WINDOW_X |XCB_CONFIG_WINDOW_Y |
      XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    cookie = xcb_configure_window_checked(con, client_win, mask, values);

    error = xcb_request_check(con, cookie);
    if(error) {
        std::cerr << "Could not resize client." << std::endl;
        exit(1);
    }

    // make client overwrite-redirected
    values[0] = true;
    mask = XCB_CW_OVERRIDE_REDIRECT;
    cookie = xcb_change_window_attributes(con, client_win, mask, values);

    error = xcb_request_check(con, cookie);
    if(error) {
        std::cerr << "Could not change client attributes." << std::endl;
        exit(1);
    }
  }
};

xcb_connection_t *con;
scr_window *window;

/// X event loop
void *event_loop(void*) {
  xcb_generic_event_t *event;

  // wait for X events and process them
  while((event = xcb_wait_for_event(con))) {
      switch(event->response_type & ~0x80) {
        case XCB_EXPOSE:
          {
            xcb_expose_event_t *expose_event
              = reinterpret_cast<xcb_expose_event_t*>(event);
            
            // ignore the expose event, if there are more waiting.
            if(!expose_event->count && window && window->get_window_id() ==
               expose_event->window) window->redraw();
            break;
          }
        case XCB_CONFIGURE_NOTIFY:
          {
            xcb_configure_notify_event_t *configure_notify_event
              = reinterpret_cast<xcb_configure_notify_event_t*>(event);
            
            if(window && window->get_window_id() ==
               configure_notify_event->window)
              window->resize(configure_notify_event->width,
                             configure_notify_event->height);
            break;
          }
        default:
          // ignore
          break;
        }
      free(event);
   }
   pthread_exit(0);
}

/// Program entry point.
int main(int argc, char *argv[]) {
  unsigned long int window_id = 0;
  bool windowed = true;
  std::string boinc_wd = "/var/lib/boinc-client";

  // parse command line
  for(int c = 0; c < argc; c++) {
      std::string option = argv[c];
      if(option == "-window-id" && argv[c+1])
        sscanf(argv[++c], "%lx", &window_id);
      else if(option == "-root")
        windowed = false;
      else if (option == "-window")
        windowed = true;
      else if (option == "-boinc_dir")
        if(argv[++c])
          boinc_wd = argv[c];
    }

  // if no -window-id command line argument is given,
  // look for the XSCREENSAVER_WINDOW environment variable
  if(!window_id) {
      char *xssw = getenv("XSCREENSAVER_WINDOW");
      if(xssw && *xssw) {
          unsigned long int id = 0;
          char c;
          if (sscanf(xssw, "0x%lx %c", &id, &c) == 1 ||
              sscanf(xssw, "%lu %c", &id, &c) == 1)
            window_id = id;
      }
  }

  // connect to the X server using $DISPLAY
  int screen_num = 0;
  con = xcb_connect(NULL, &screen_num);

  if(!con) {
      std::cerr << "Cannot connect to your X server." << std::endl
                << "Please check if it's running and whether your DISPLAY "
                << "environment variable is set correctly." << std::endl;
      return 1;
   }

  // get default screen
  xcb_screen_t *screen;
  for(xcb_screen_iterator_t it = xcb_setup_roots_iterator(xcb_get_setup(con));
      it.rem; screen_num--, xcb_screen_next(&it))
    if(!screen_num) screen = it.data;

  // create screensaver window
  window = new scr_window(con, screen, window_id, windowed);
  window->set_text("screensaver loading");

  // start the X event loop
  pthread_t thread;
  if(pthread_create(&thread, NULL, event_loop, NULL)) {
      std::cerr << "Could not create a thread." << std::endl;
      exit(1);
  }

  // try to connect
  RPC_CLIENT *rpc = new RPC_CLIENT;
  if(rpc->init(NULL)) {
      window->set_text("boinc not running");
      pthread_join(thread, NULL);
      return 0;
  }

  // get results that support graphics
  RESULTS results;
  while(true) {
      int suspend_reason = 0;
      rpc->get_screensaver_tasks(suspend_reason, results);
      if(results.results.empty()) sleep(10);
      else break;
  }

  srandom(time(NULL));
  std::string graphics_cmd = "graphics_app";
  // the loop skips projects that do not yet
  // support the graphics_app soft link.
  while(graphics_cmd == "graphics_app") {
      // select a random result
      int n = random() % results.results.size();
      RESULT *result = results.results[n];
      
      // change to slot dir
      std::stringstream stream;
      stream << boinc_wd << "/slots/" << result->slot << "/";
      std::string slot_dir = stream.str();
      if(chdir(slot_dir.c_str())) {
          perror("chdir");
          exit(1);
      }

      // resolve graphics_app soft link
      boinc_resolve_filename_s(graphics_cmd.c_str(), graphics_cmd);
  }

  // fork and...
  pid_t pid = fork();
  if(pid == -1) {
      perror("fork");
      exit(1);
  }

  // ...spawn graphics app
  if(!pid) // child
    if(execl(graphics_cmd.c_str(), graphics_cmd.c_str(), NULL)) {
        perror("exec");
        exit(1);
    }

  // look for our graphics app
  // do this 10 times, every 1/2 seconds, then give up.
  //
  xcb_window_t client = 0;
  for(int n = 0; n < 10; n++) {
      // get list of x clients
      xcb_intern_atom_cookie_t cookie0=xcb_intern_atom(
            con, 0, strlen("_NET_CLIENT_LIST"), "_NET_CLIENT_LIST"
      );
      xcb_intern_atom_reply_t *reply0=xcb_intern_atom_reply(con, cookie0, NULL);

      xcb_get_property_cookie_t cookie =
        xcb_get_property(con, 0, screen->root, reply0->atom, XCB_ATOM_WINDOW, 0,
                         std::numeric_limits<uint32_t>::max());

      xcb_generic_error_t  *error;
      xcb_get_property_reply_t *reply =
        xcb_get_property_reply(con, cookie, &error);
      if(error) {
          std::cerr << "Could not get client list." << std::endl;
          exit(1);
      }

      xcb_window_t *clients =
        static_cast<xcb_window_t*>(xcb_get_property_value(reply));

      // check if one of them is our graphics app
      for(int c = 0; c < reply->length; c++) {
          xcb_get_property_reply_t *reply2;

          // check WM_COMMAND
          cookie = xcb_get_property(con, 0, clients[c], XCB_ATOM_WM_COMMAND, XCB_ATOM_STRING,
                                    0, std::numeric_limits<uint32_t>::max());
          reply2 = xcb_get_property_reply(con, cookie, &error);
          if(!error) // ignore errors {
              char *command = static_cast<char*>(xcb_get_property_value(reply2));
      
              if(command && graphics_cmd == command) {
                  client = clients[c];
                  break;
              }

              free(reply2);
          }

          // check WM_CLASS
          cookie = xcb_get_property(con, 0, clients[c], XCB_ATOM_WM_CLASS, XCB_ATOM_STRING,
                                    0, std::numeric_limits<uint32_t>::max());
          reply2 = xcb_get_property_reply(con, cookie, &error);
          if(!error) // ignore errors {
              char *clas = static_cast<char*>(xcb_get_property_value(reply2));

              size_t pos = graphics_cmd.find_last_of('/');
              std::string executable;
              if(pos == std::string::npos) executable = graphics_cmd;
              else executable = graphics_cmd.substr(pos + 1);

              if(clas && executable == clas) {
                  client = clients[c];
                  break;
                }
              
              free(reply2);
          }

          // More checks are possible, but a single method for all graphics
          // applications would be preferred, such as WM_CLASS = "BOINC".
      }

      free(reply);

      if(client) break;

      usleep(500000);
  }

  // if the client window was found, xembed it
  if(client)
    window->xembed(client);

  pthread_join(thread, NULL);

  delete window;
  xcb_disconnect(con);
  return 0;
}
