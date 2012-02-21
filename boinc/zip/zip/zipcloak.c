/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
   This code was originally written in Europe and can be freely distributed
   from any country except the U.S.A. If this code is imported into the U.S.A,
   it cannot be re-exported from the U.S.A to another country. (This
   restriction might seem curious but this is what US law requires.)
 */
#define __ZIPCLOAK_C

#ifndef UTIL
#define UTIL
#endif
#include "zip.h"
#define DEFCPYRT        /* main module: enable copyright string defines! */
#include "revision.h"
//#include "z_crypt.h"
#include "z_ttyio.h"
#include <signal.h>
#ifndef NO_STDLIB_H
#  include <stdlib.h>
#endif

#if CRYPT       /* defined (as TRUE or FALSE) in crypt.h */

int main OF((int argc, char **argv));

local void handler OF((int sig));
local void license OF((void));
local void help OF((void));
local void version_info OF((void));

/* Temporary zip file name and file pointer */
local char *tempzip;
local FILE *tempzf;

/* Pointer to CRC-32 table (used for decryption/encryption) */
#if (!defined(USE_ZLIB) || defined(USE_OWN_CRCTAB))
ZCONST ulg near *crc_32_tab;
#else
ZCONST uLongf *crc_32_tab;
#endif

/***********************************************************************
 * Issue a message for the error, clean up files and memory, and exit.
 */
void ziperr(code, msg)
    int code;               /* error code from the ZE_ class */
    char *msg;              /* message about how it happened */
{
    if (PERR(code)) perror("zipcloak error");
    fprintf(stderr, "zipcloak error: %s (%s)\n", errors[code-1], msg);
    if (tempzf != NULL) fclose(tempzf);
    if (tempzip != NULL) {
        destroy(tempzip);
        free((zvoid *)tempzip);
    }
    if (zipfile != NULL) free((zvoid *)zipfile);
    EXIT(code);
}

/***********************************************************************
 * Print a warning message to stderr and return.
 */
void zipwarn(msg1, msg2)
    char *msg1, *msg2;        /* message strings juxtaposed in output */
{
    fprintf(stderr, "zipcloak warning: %s%s\n", msg1, msg2);
}


/***********************************************************************
 * Upon getting a user interrupt, turn echo back on for tty and abort
 * cleanly using ziperr().
 */
local void handler(sig)
    int sig;                  /* signal number (ignored) */
{
#if (!defined(MSDOS) && !defined(__human68k__) && !defined(RISCOS))
    echon();
    putc('\n', stderr);
#endif
    ziperr(ZE_ABORT +sig-sig, "aborting");
    /* dummy usage of sig to avoid compiler warnings */
}


static ZCONST char *public[] = {
"The encryption code of this program is not copyrighted and is put in the",
"public domain. It was originally written in Europe and can be freely",
"distributed from any country except the U.S.A. If this program is imported",
"into the U.S.A, it cannot be re-exported from the U.S.A to another country.",
"The copyright notice of the zip program applies to the rest of the code."
};

/***********************************************************************
 * Print license information to stdout.
 */
local void license()
{
    extent i;             /* counter for copyright array */

    for (i = 0; i < sizeof(public)/sizeof(char *); i++) {
        puts(public[i]);
    }
    for (i = 0; i < sizeof(swlicense)/sizeof(char *); i++) {
        puts(swlicense[i]);
    }
}


static ZCONST char *help_info[] = {
"",
"ZipCloak %s (%s)",
#ifdef VM_CMS
"Usage:  zipcloak [-d] [-b fm] zipfile",
#else
"Usage:  zipcloak [-d] [-b path] zipfile",
#endif
"  the default action is to encrypt all unencrypted entries in the zip file",
"  -d   decrypt--decrypt encrypted entries (copy if given wrong password)",
#ifdef VM_CMS
"  -b   use \"fm\" as the filemode for the temporary zip file",
#else
"  -b   use \"path\" for the temporary zip file",
#endif
"  -h   show this help    -v   show version info    -L   show software license"
  };

/***********************************************************************
 * Print help (along with license info) to stdout.
 */
local void help()
{
    extent i;             /* counter for help array */

    for (i = 0; i < sizeof(public)/sizeof(char *); i++) {
        puts(public[i]);
    }
    for (i = 0; i < sizeof(help_info)/sizeof(char *); i++) {
        printf(help_info[i], VERSION, REVDATE);
        putchar('\n');
    }
}


local void version_info()
/* Print verbose info about program version and compile time options
   to stdout. */
{
  extent i;             /* counter in text arrays */

  /* Options info array */
  static ZCONST char *comp_opts[] = {
#ifdef DEBUG
    "DEBUG",
#endif
#if CRYPT && defined(PASSWD_FROM_STDIN)
    "PASSWD_FROM_STDIN",
#endif /* CRYPT && PASSWD_FROM_STDIN */
    NULL
  };

  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++)
  {
    printf(copyright[i], "zipcloak");
    putchar('\n');
  }

  for (i = 0; i < sizeof(versinfolines)/sizeof(char *); i++)
  {
    printf(versinfolines[i], "ZipCloak", VERSION, REVDATE);
    putchar('\n');
  }

  version_local();

  puts("ZipCloak special compilation options:");
  for (i = 0; (int)i < (int)(sizeof(comp_opts)/sizeof(char *) - 1); i++)
  {
    printf("\t%s\n",comp_opts[i]);
  }
  printf("\t[encryption, version %d.%d%s of %s]\n",
            CR_MAJORVER, CR_MINORVER, CR_BETA_VER, CR_VERSION_DATE);
}


/***********************************************************************
 * Encrypt or decrypt all of the entries in a zip file.  See the command
 * help in help() above.
 */

int main(argc, argv)
    int argc;                   /* number of tokens in command line */
    char **argv;                /* command line tokens */
{
    int attr;                   /* attributes of zip file */
    ulg start_offset;           /* start of central directory */
    int decrypt;                /* decryption flag */
    int temp_path;              /* 1 if next argument is path for temp files */
    char passwd[IZ_PWLEN+1];    /* password for encryption or decryption */
    char verify[IZ_PWLEN+1];    /* password for encryption or decryption */
    char *q;                    /* steps through option arguments */
    int r;                      /* arg counter */
    int res;                    /* result code */
    ulg length;                 /* length of central directory */
    FILE *inzip, *outzip;       /* input and output zip files */
    struct zlist far *z;        /* steps through zfiles linked list */

#ifdef THEOS
    setlocale(LC_CTYPE, "I");
#endif

    /* If no args, show help */
    if (argc == 1) {
        help();
        EXIT(0);
    }

    init_upper();               /* build case map table */

    crc_32_tab = get_crc_table_boinc();
                                /* initialize crc table for crypt */

    /* Go through args */
    zipfile = tempzip = NULL;
    tempzf = NULL;
    temp_path = decrypt = 0;
    for (r = 1; r < argc; r++) {
        if (*argv[r] == '-') {
            if (!argv[r][1]) ziperr(ZE_PARMS, "zip file cannot be stdin");
            for (q = argv[r]+1; *q; q++) {
                switch (*q) {
                case 'b':   /* Specify path for temporary file */
                    if (temp_path) {
                        ziperr(ZE_PARMS, "use -b before zip file name");
                    }
                    temp_path = 1;          /* Next non-option is path */
                    break;
                case 'd':
                    decrypt = 1;  break;
                case 'h':   /* Show help */
                    help();
                    EXIT(0);
                case 'l': case 'L':  /* Show copyright and disclaimer */
                    license();
                    EXIT(0);
                case 'v':   /* Show version info */
                    version_info();
                    EXIT(0);
                default:
                    ziperr(ZE_PARMS, "unknown option");
                } /* switch */
            } /* for */

        } else if (temp_path == 0) {
            if (zipfile != NULL) {
                ziperr(ZE_PARMS, "can only specify one zip file");

            } else if ((zipfile = ziptyp(argv[r])) == NULL) {
                ziperr(ZE_MEM, "was processing arguments");
            }
        } else {
            tempath = argv[r];
            temp_path = 0;
        } /* if */
    } /* for */

    if (zipfile == NULL) ziperr(ZE_PARMS, "need to specify zip file");

    /* Read zip file */
    if ((res = readzipfile()) != ZE_OK) ziperr(res, zipfile);
    if (zfiles == NULL) ziperr(ZE_NAME, zipfile);

    /* Check for something to do */
    for (z = zfiles; z != NULL; z = z->nxt) {
        if (decrypt ? z->flg & 1 : !(z->flg & 1)) break;
    }
    if (z == NULL) {
        ziperr(ZE_NONE, decrypt ? "no encrypted files"
                       : "all files encrypted already");
    }

    /* Before we get carried away, make sure zip file is writeable */
    if ((inzip = fopen(zipfile, "a")) == NULL) ziperr(ZE_CREAT, zipfile);
    fclose(inzip);
    attr = getfileattr(zipfile);

    /* Open output zip file for writing */
    if ((tempzf = outzip = fopen(tempzip = tempname(zipfile), FOPW)) == NULL) {
        ziperr(ZE_TEMP, tempzip);
    }

    /* Get password */
    if (getp("Enter password: ", passwd, IZ_PWLEN+1) == NULL)
        ziperr(ZE_PARMS,
               "stderr is not a tty (you may never see this message!)");

    if (decrypt == 0) {
        if (getp("Verify password: ", verify, IZ_PWLEN+1) == NULL)
               ziperr(ZE_PARMS,
                      "stderr is not a tty (you may never see this message!)");

        if (strcmp(passwd, verify))
               ziperr(ZE_PARMS, "password verification failed");

        if (*passwd == '\0')
               ziperr(ZE_PARMS, "zero length password not allowed");
    }

    /* Open input zip file again, copy preamble if any */
    if ((inzip = fopen(zipfile, FOPR)) == NULL) ziperr(ZE_NAME, zipfile);

    if (zipbeg && (res = fcopy(inzip, outzip, zipbeg)) != ZE_OK) {
        ziperr(res, res == ZE_TEMP ? tempzip : zipfile);
    }
    tempzn = zipbeg;

    /* Go through local entries, copying, encrypting, or decrypting */
    for (z = zfiles; z != NULL; z = z->nxt) {
        if (decrypt && (z->flg & 1)) {
            printf("decrypting: %s", z->zname);
            fflush(stdout);
            if ((res = zipbare(z, inzip, outzip, passwd)) != ZE_OK) {
                if (res != ZE_MISS) ziperr(res, "was decrypting an entry");
                printf(" (wrong password--just copying)");
            }
            putchar('\n');

        } else if ((!decrypt) && !(z->flg & 1)) {
            printf("encrypting: %s\n", z->zname);
            fflush(stdout);
            if ((res = zipcloak(z, inzip, outzip, passwd)) != ZE_OK) {
                ziperr(res, "was encrypting an entry");
            }
        } else {
            printf("   copying: %s\n", z->zname);
            fflush(stdout);
            if ((res = zipcopy(z, inzip, outzip)) != ZE_OK) {
                ziperr(res, "was copying an entry");
            }
        } /* if */
    } /* for */
    fclose(inzip);

    /* Write central directory and end of central directory */

    /* get start of central */
    if ((start_offset = (ulg)ftell(outzip)) == (ulg)-1L)
        ziperr(ZE_TEMP, tempzip);

    for (z = zfiles; z != NULL; z = z->nxt) {
        if ((res = putcentral(z, outzip)) != ZE_OK) ziperr(res, tempzip);
    }

    /* get end of central */
    if ((length = (ulg)ftell(outzip)) == (ulg)-1L)
        ziperr(ZE_TEMP, tempzip);

    length -= start_offset;               /* compute length of central */
    if ((res = putend((int)zcount, length, start_offset, zcomlen,
                       zcomment, outzip)) != ZE_OK) {
        ziperr(res, tempzip);
    }
    tempzf = NULL;
    if (fclose(outzip)) ziperr(ZE_TEMP, tempzip);
    if ((res = replace(zipfile, tempzip)) != ZE_OK) {
        zipwarn("new zip file left as: ", tempzip);
        free((zvoid *)tempzip);
        tempzip = NULL;
        ziperr(res, "was replacing the original zip file");
    }
    free((zvoid *)tempzip);
    tempzip = NULL;
    setfileattr(zipfile, attr);
#ifdef RISCOS
    /* Set the filetype of the zipfile to &DDC */
    setfiletype(zipfile, 0xDDC);
#endif
    free((zvoid *)zipfile);
    zipfile = NULL;

    /* Done! */
    RETURN(0);
}
#else /* !CRYPT */

int main OF((void));

void zipwarn(msg1, msg2)
char  *msg1, *msg2;
{
    /* Tell picky compilers to shut up about unused variables */
    msg1 = msg1; msg2 = msg2;
}

void ziperr(c, h)
int  c;
char *h;
{
    /* Tell picky compilers to shut up about unused variables */
    c = c; h = h;
}

int main()
{
    fprintf(stderr, "\
This version of ZipCloak does not support encryption.  Get zcrypt27.zip (or\n\
a later version) and recompile.  The Info-ZIP file `WHERE' lists sites.\n");
    RETURN(1);
}

#endif /* ?CRYPT */

const char *BOINC_RCSID_20afbe80ed = "$Id: zipcloak.c 7481 2005-08-25 21:33:28Z davea $";
