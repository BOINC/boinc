// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#include "graphics_data.h"

void GRAPHICS_BUFFER::clear() {
}

void GRAPHICS_DOUBLE_BUFFER::init(GRAPHICS_BUFFER* _b1, GRAPHICS_BUFFER* _b2) {
    b1 = _b1;
    b2 = _b2;
}

GRAPHICS_BUFFER* GRAPHICS_DOUBLE_BUFFER::get_buffer(int state) {
    if (b1->state == state) return b1;
    if (b2->state == state) return b2;
    return 0;
}

GRAPHICS_BUFFER* GRAPHICS_DOUBLE_BUFFER::get_render_buffer() {
    GRAPHICS_BUFFER* b, *other;

    b = get_buffer(GB_STATE_RENDERING);
    if (!b) b = get_buffer(GB_STATE_RENDERED);
    if (!b) b = get_buffer(GB_STATE_GENERATED);
    if (b) {
        if (b->state == GB_STATE_RENDERED) {
            other = get_buffer(GB_STATE_GENERATED);
            if (other) {
                b->clear();
                b->state = GB_STATE_IDLE;
                b = other;
                b->state = GB_STATE_RENDERING;
            }
        } else {
            b->state = GB_STATE_RENDERING;
        }
    }
    return b;
}

void GRAPHICS_DOUBLE_BUFFER::render_done(GRAPHICS_BUFFER* b) {
    b->state = GB_STATE_RENDERED;
}

GRAPHICS_BUFFER* GRAPHICS_DOUBLE_BUFFER::get_generate_buffer(bool first) {
    GRAPHICS_BUFFER* b;

    b = get_buffer(GB_STATE_GENERATING);
    if (!b && first) {
        b = get_buffer(GB_STATE_IDLE);
        if (b) {
            b->state = GB_STATE_GENERATING;
        }
    }
    return b;
}

void GRAPHICS_DOUBLE_BUFFER::generate_done(GRAPHICS_BUFFER* b) {
    b->state = GB_STATE_GENERATED;
}

const char *BOINC_RCSID_ebfbd0f929 = "$Id$";
