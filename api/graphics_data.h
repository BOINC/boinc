// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// The class GRAPHICS_DOUBLE_BUFFER provides a mechanism
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
#ifndef GRAPHICS_DATA_H
#define GRAPHICS_DATA_H

#define GB_STATE_IDLE       0
#define GB_STATE_GENERATING 1
#define GB_STATE_GENERATED  2
#define GB_STATE_RENDERING  3
#define GB_STATE_RENDERED   4

// subclass this for your applications
//
class GRAPHICS_BUFFER {
public:
    virtual ~GRAPHICS_BUFFER() {}
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

#endif
