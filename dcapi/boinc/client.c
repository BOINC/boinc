#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* BOINC includes */
#ifdef _WIN32
#include <boinc_win.h>
#include <util.h>
#endif
#include <boinc_api.h>
#include <filesys.h>
#include <diagnostics.h>
#include <graphics_api.h>

#include <dc_client.h>

int DC_init(void)
{
  int retval;

  retval = boinc_init_diagnostics(BOINC_DIAG_REDIRECTSTDERR);
  if (retval) return retval;
  retval = boinc_init();
  if (retval) return retval;
  
  return DC_OK;
}

int DC_resolveFileName(DC_Filetype type, const char *requestedFileName, char *actualFileName, int maxlength)
{
  int retval;

  retval = boinc_resolve_filename(requestedFileName, actualFileName, maxlength);
  if (retval) return retval;

  return DC_OK;
}

int DC_sendResult(char **files, int nfiles)
{
  // not implemented yet!!

  return DC_OK;
}

int DC_timeToCheckpoint(void)
{
  return boinc_time_to_checkpoint();
}

int DC_checkpointMade(void)
{
  return boinc_checkpoint_completed();
}

int DC_continueWork(void)
{
  // not implemented yet!!

  return 1; // yes, continue work.
}

int DC_fractionDone(double fraction)
{
  return boinc_fraction_done(fraction);
}

void DC_finish(int exitcode)
{
  boinc_finish(exitcode);
}

void app_graphics_init(void) {}
void app_graphics_resize(int width, int height){}
void app_graphics_render(int xs, int ys, double time_of_day) {}
void app_graphics_reread_prefs(void) {}
void boinc_app_mouse_move(int x, int y, bool left, bool middle, bool right ){}
void boinc_app_mouse_button(int x, int y, int which, bool is_down){}
void boinc_app_key_press(int x, int y){}
void boinc_app_key_release(int x, int y){}
