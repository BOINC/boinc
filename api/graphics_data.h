// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

// The class GRAPHICS_DOUBLE_BUFFER provides a handy mechanism
// for synchronizing the generation of graphics information
// (done by the "science thread")
// with the graphics rendering (done by the "GUI thread",
// which periodically calls the application-supplied render() function)
//
// We assume that graphics info is generated incrementally
// and displayed incrementally.
// The scheme handles mismatches between generation and rendering rates:
// - if generation is faster than display, data cycles will be skipped
// - if display is faster than generation, the display will "linger"
//   on a cycle until the next cycle is available in its entirety.
//
// Here's what the application programmer does:
// - subclass GRAPHICS_BUFFER, adding your application-specific data
// - declare a GRAPHICS_DOUBLE_BUFFER object (call it gdb),
//   initializing it with pointers to two GRAPHICS_BUFFER structures.
// - when the science thread wants to add an increment of data,
//   it calls gdb->get_generate_buffer(first)
//   ("first" indicates whether this is the first data increment of a cycle).
//   If this returns nonzero, it adds data to the buffer.
//   If this addition causes it to be full, it calls gdb->generate_done();
// - render() calls gdb->get_render_buffer().
//   If this returns nonzero, it displays some or all the data.
//   If this completes the display of the data, it calls gdb->render_done().

// Here's how it works:
// There are two buffers, each with a "state" (see below).
// In a given state, only one thread can access the data or change the state.
//
#define GB_STATE_IDLE       0
#define GB_STATE_GENERATING 1
#define GB_STATE_GENERATED  2
#define GB_STATE_RENDERING  3
#define GB_STATE_RENDERED   4

// subclass this for your applications
//
class GRAPHICS_BUFFER {
public:
    virtual void clear();
    int state;
};

class GRAPHICS_DOUBLE_BUFFER {
    GRAPHICS_BUFFER* b1, *b2;
    GRAPHICS_BUFFER* get_buffer(int state);
public:
    void init(GRAPHICS_BUFFER*, GRAPHICS_BUFFER*);

    // the following called by app's render()
    //
    GRAPHICS_BUFFER* get_render_buffer();
    void render_done(GRAPHICS_BUFFER*);

    // the following called by app's science thread
    //
    GRAPHICS_BUFFER* get_generate_buffer(bool first);
    void generate_done(GRAPHICS_BUFFER*);
};
