#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <dc_client.h>
#include "common_defs.h"

static time_t ckpt_start;
static time_t ckpt_freq = 10; /* Checkpoint frequency in seconds */

int DC_init(void)
{
  ckpt_start = time(NULL);
  return DC_OK;
}

char *DC_resolveFileName(DC_FileType type, const char *logicalFileName)
{
  return (char *)logicalFileName;
}

int DC_sendResult(const char *logicalFileName, const char *path, DC_FileMode fileMode)
{
  // not implemented yet!!

  return DC_OK;
}

int DC_sendMessage(const char *message)
{
  // not implemented yet!!

  return DC_OK;
}

DC_Event *DC_checkEvent(void)
{
  // not impl.

  return NULL;
}

void DC_destroyEvent(DC_Event *event)
{
  // not impl.
}

#if 0
int DC_timeToCheckpoint(void)
{
  time_t current;
  current = time(NULL);
  if (current - ckpt_start > ckpt_freq)
    return 1;
  return 0;
}
#endif

void DC_checkpointMade(const char *fileName)
{
  ckpt_start = time(NULL);
}

void DC_fractionDone(double fraction)
{

}

void DC_finish(int exitcode)
{
  exit(exitcode);
}
