#include "boinc_api.h"
#include "graphics_impl.h"
#include "graphics_api.h"

int boinc_init_graphics(void (*worker)()) {
    return boinc_init_graphics_impl(worker, boinc_init_options_general);
}

int boinc_init_options_graphics(BOINC_OPTIONS& opt, void (*worker)()) {
    return boinc_init_options_graphics_impl(opt, worker, boinc_init_options_general);
}
