#include "boinc_api.h"
#include "graphics_impl.h"
#include "graphics_api.h"

static BOINC_MAIN_STATE boinc_main_state;

static void init_main_state() {
    boinc_main_state.boinc_init_options_general_hook = boinc_init_options_general;
    boinc_main_state.boinc_is_standalone_hook = boinc_is_standalone;
    boinc_main_state.boinc_get_init_data_hook = boinc_get_init_data;
    boinc_main_state.set_worker_timer_hook = set_worker_timer;
    boinc_main_state.app_client_shm = app_client_shm;
}

int boinc_init_graphics(void (*worker)()) {
    init_main_state();
    return boinc_init_graphics_impl(worker, &boinc_main_state);
}

int boinc_init_options_graphics(BOINC_OPTIONS& opt, void (*worker)()) {
    init_main_state();
    return boinc_init_options_graphics_impl(opt, worker, &boinc_main_state);
}

#ifdef __GNUC__
static volatile const char  __attribute__((unused)) *BOINCrcsid="$Id$";
#else
static volatile const char *BOINCrcsid="$Id$";
#endif
