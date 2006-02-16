#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <dc_client.h>

static time_t ckpt_start;
static time_t ckpt_freq = 10; /* Checkpoint frequency in seconds */

int DC_init(void)
{
  ckpt_start = time(NULL);
  return DC_OK;
}

int DC_resolveFileName(DC_Filetype type, const char *requestedFileName, char *actualFileName, int maxlength)
{
  snprintf(actualFileName, maxlength, requestedFileName);
  return DC_OK;
}

int DC_sendResult(char **files, int nfiles)
{
  // not implemented yet!!

  return DC_OK;
}

int DC_timeToCheckpoint(void)
{
  time_t current;
  current = time(NULL);
  if (current - ckpt_start > ckpt_freq)
    return 1;
  return 0;
}

int DC_checkpointMade(void)
{
  ckpt_start = time(NULL);
  return DC_OK;
}

int DC_continueWork(void)
{
  // not implemented yet!!

  return 1; // yes, continue work.
}

int DC_fractionDone(double fraction)
{
  return DC_OK;
}

void DC_finish(int exitcode)
{
  exit(exitcode);
}
