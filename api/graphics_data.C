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
