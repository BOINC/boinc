/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
 *  zipsplit.c by Mark Adler.
 */
#define __ZIPSPLIT_C

#ifndef UTIL
#define UTIL
#endif
#include "zip.h"
#define DEFCPYRT        /* main module: enable copyright string defines! */
#include "revision.h"
#include <signal.h>

#define DEFSIZ 36000L   /* Default split size (change in help() too) */
#ifdef MSDOS
#  define NL 2          /* Number of bytes written for a \n */
#else /* !MSDOS */
#  define NL 1          /* Number of bytes written for a \n */
#endif /* ?MSDOS */
#ifdef RISCOS
#  define INDEX "zipspl/idx"      /* Name of index file */
#  define TEMPL_FMT "%%0%dld"
#  define TEMPL_SIZ 13
#  define ZPATH_SEP '.'
#else
#ifdef QDOS
#  define ZPATH_SEP '_'
#  define INDEX "zipsplit_idx"    /* Name of index file */
#  define TEMPL_FMT "%%0%dld_zip"
#  define TEMPL_SIZ 17
#  define exit(p1) QDOSexit()
#else
#ifdef VM_CMS
#  define INDEX "zipsplit.idx"    /* Name of index file */
#  define TEMPL_FMT "%%0%dld.zip"
#  define TEMPL_SIZ 21
#  define ZPATH_SEP '.'
#else
#  define INDEX "zipsplit.idx"    /* Name of index file */
#  define TEMPL_FMT "%%0%dld.zip"
#  define TEMPL_SIZ 17
#  define ZPATH_SEP '.'
#endif /* VM_CMS */
#endif /* QDOS */
#endif /* RISCOS */

#ifdef MACOS
#define ziperr(c, h)    zipspliterr(c, h)
#define zipwarn(a, b)   zipsplitwarn(a, b)
void zipsplitwarn(char *c,char *h);
void zipspliterr(int c,char *h);
#endif /* MACOS */

/* Local functions */
local zvoid *talloc OF((extent));
local void tfree OF((zvoid *));
local void tfreeall OF((void));
local void handler OF((int));
local void license OF((void));
local void help OF((void));
local void version_info OF((void));
local extent simple OF((ulg *, extent, ulg, ulg));
local int descmp OF((ZCONST zvoid *, ZCONST zvoid *));
local extent greedy OF((ulg *, extent, ulg, ulg));
local int retry OF((void));
int main OF((int, char **));


/* Output zip files */
local char template[TEMPL_SIZ]; /* name template for output files */
local int zipsmade = 0;         /* number of zip files made */
local int indexmade = 0;        /* true if index file made */
local char *path = NULL;        /* space for full name */
local char *name;               /* where name goes in path[] */


/* The talloc() and tree() routines extend malloc() and free() to keep
   track of all allocated memory.  Then the tfreeall() routine uses this
   information to free all allocated memory before exiting. */

#define TMAX 6          /* set intelligently by examining the code */
zvoid *talls[TMAX];     /* malloc'ed pointers to track */
int talln = 0;          /* number of entries in talls[] */


local zvoid *talloc(s)
extent s;
/* does a malloc() and saves the pointer to free later (does not check
   for an overflow of the talls[] list) */
{
  zvoid *p;

  if ((p = (zvoid *)malloc(s)) != NULL)
    talls[talln++] = p;
  return p;
}


local void tfree(p)
zvoid *p;
/* does a free() and also removes the pointer from the talloc() list */
{
  int i;

  free(p);
  i = talln;
  while (i--)
    if (talls[i] == p)
      break;
  if (i >= 0)
  {
    while (++i < talln)
      talls[i - 1] = talls[i];
    talln--;
  }
}


local void tfreeall()
/* free everything talloc'ed and not tfree'd */
{
  while (talln)
    free(talls[--talln]);
}


void ziperr(c, h)
int c;                  /* error code from the ZE_ class */
char *h;                /* message about how it happened */
/* Issue a message for the error, clean up files and memory, and exit. */
{
  if (PERR(c))
    perror("zipsplit error");
  fprintf(stderr, "zipsplit error: %s (%s)\n", errors[c-1], h);
  if (indexmade)
  {
    strcpy(name, INDEX);
    destroy(path);
  }
  for (; zipsmade; zipsmade--)
  {
    sprintf(name, template, zipsmade);
    destroy(path);
  }
  tfreeall();
  if (zipfile != NULL)
    free((zvoid *)zipfile);
  EXIT(c);
}


local void handler(s)
int s;                  /* signal number (ignored) */
/* Upon getting a user interrupt, abort cleanly using ziperr(). */
{
#ifndef MSDOS
  putc('\n', stderr);
#endif /* !MSDOS */
  ziperr(ZE_ABORT, "aborting");
  s++;                                  /* keep some compilers happy */
}


void zipwarn(a, b)
char *a, *b;            /* message strings juxtaposed in output */
/* Print a warning message to stderr and return. */
{
  fprintf(stderr, "zipsplit warning: %s%s\n", a, b);
}


local void license()
/* Print license information to stdout. */
{
  extent i;             /* counter for copyright array */

  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++) {
    printf(copyright[i], "zipsplit");
    putchar('\n');
  }
  for (i = 0; i < sizeof(swlicense)/sizeof(char *); i++)
    puts(swlicense[i]);
}


local void help()
/* Print help (along with license info) to stdout. */
{
  extent i;             /* counter for help array */

  /* help array */
  static ZCONST char *text[] = {
"",
"ZipSplit %s (%s)",
#ifdef VM_CMS
"Usage:  zipsplit [-tips] [-n size] [-r room] [-b fm] zipfile",
#else
"Usage:  zipsplit [-tips] [-n size] [-r room] [-b path] zipfile",
#endif
"  -t   report how many files it will take, but don't make them",
#ifdef RISCOS
"  -i   make index (" INDEX ") and count its size against first zip file",
#else
"  -i   make index (zipsplit.idx) and count its size against first zip file",
#endif
"  -n   make zip files no larger than \"size\" (default = 36000)",
"  -r   leave room for \"room\" bytes on the first disk (default = 0)",
#ifdef VM_CMS
"  -b   use \"fm\" as the filemode for the output zip files",
#else
"  -b   use \"path\" for the output zip files",
#endif
"  -p   pause between output zip files",
"  -s   do a sequential split even if it takes more zip files",
"  -h   show this help    -v   show version info    -L   show software license"
  };

  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++) {
    printf(copyright[i], "zipsplit");
    putchar('\n');
  }
  for (i = 0; i < sizeof(text)/sizeof(char *); i++)
  {
    printf(text[i], VERSION, REVDATE);
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
    NULL
  };

  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++)
  {
    printf(copyright[i], "zipsplit");
    putchar('\n');
  }

  for (i = 0; i < sizeof(versinfolines)/sizeof(char *); i++)
  {
    printf(versinfolines[i], "ZipSplit", VERSION, REVDATE);
    putchar('\n');
  }

  version_local();

  puts("ZipSplit special compilation options:");
  for (i = 0; (int)i < (int)(sizeof(comp_opts)/sizeof(char *) - 1); i++)
  {
    printf("\t%s\n",comp_opts[i]);
  }
  if (i == 0)
      puts("\t[none]");
}


local extent simple(a, n, c, d)
ulg *a;         /* items to put in bins, return value: destination bins */
extent n;       /* number of items */
ulg c;          /* capacity of each bin */
ulg d;          /* amount to deduct from first bin */
/* Return the number of bins of capacity c that are needed to contain the
   integers in a[0..n-1] placed sequentially into the bins.  The value d
   is deducted initially from the first bin (space for index).  The entries
   in a[] are replaced by the destination bins. */
{
  extent k;     /* current bin number */
  ulg t;        /* space used in current bin */

  t = k = 0;
  while (n--)
  {
    if (*a + t > c - (k == 0 ? d : 0))
    {
      k++;
      t = 0;
    }
    t += *a;
    *(ulg huge *)a++ = k;
  }
  return k + 1;
}


local int descmp(a, b)
ZCONST zvoid *a, *b;          /* pointers to pointers to ulg's to compare */
/* Used by qsort() in greedy() to do a descending sort. */
{
  return **(ulg **)a < **(ulg **)b ? 1 : (**(ulg **)a > **(ulg **)b ? -1 : 0);
}


local extent greedy(a, n, c, d)
ulg *a;         /* items to put in bins, return value: destination bins */
extent n;       /* number of items */
ulg c;          /* capacity of each bin */
ulg d;          /* amount to deduct from first bin */
/* Return the number of bins of capacity c that are needed to contain the
   items with sizes a[0..n-1] placed non-sequentially into the bins.  The
   value d is deducted initially from the first bin (space for index).
   The entries in a[] are replaced by the destination bins. */
{
  ulg *b;       /* space left in each bin (malloc'ed for each m) */
  ulg *e;       /* copy of argument a[] (malloc'ed) */
  extent i;     /* steps through items */
  extent j;     /* steps through bins */
  extent k;     /* best bin to put current item in */
  extent m;     /* current number of bins */
  ulg **s;      /* pointers to e[], sorted descending (malloc'ed) */
  ulg t;        /* space left in best bin (index k) */

  /* Algorithm:
     1. Copy a[] to e[] and sort pointers to e[0..n-1] (in s[]), in
        descending order.
     2. Compute total of s[] and set m to the smallest number of bins of
        capacity c that can hold the total.
     3. Allocate m bins.
     4. For each item in s[], starting with the largest, put it in the
        bin with the smallest current capacity greater than or equal to the
        item's size.  If no bin has enough room, increment m and go to step 4.
     5. Else, all items ended up in a bin--return m.
  */

  /* Copy a[] to e[], put pointers to e[] in s[], and sort s[].  Also compute
     the initial number of bins (minus 1). */
  if ((e = (ulg *)malloc(n * sizeof(ulg))) == NULL ||
      (s = (ulg **)malloc(n * sizeof(ulg *))) == NULL)
  {
    if (e != NULL)
      free((zvoid *)e);
    ziperr(ZE_MEM, "was trying a smart split");
    return 0;                           /* only to make compiler happy */
  }
  memcpy((char *)e, (char *)a, n * sizeof(ulg));
  for (t = i = 0; i < n; i++)
    t += *(s[i] = e + i);
  m = (extent)((t + c - 1) / c) - 1;    /* pre-decrement for loop */
  qsort((char *)s, n, sizeof(ulg *), descmp);

  /* Stuff bins until successful */
  do {
    /* Increment the number of bins, allocate and initialize bins */
    if ((b = (ulg *)malloc(++m * sizeof(ulg))) == NULL)
    {
      free((zvoid *)s);
      free((zvoid *)e);
      ziperr(ZE_MEM, "was trying a smart split");
    }
    b[0] = c - d;                       /* leave space in first bin */
    for (j = 1; j < m; j++)
      b[j] = c;

    /* Fill the bins greedily */
    for (i = 0; i < n; i++)
    {
      /* Find smallest bin that will hold item i (size s[i]) */
      t = c + 1;
      for (k = j = 0; j < m; j++)
        if (*s[i] <= b[j] && b[j] < t)
          t = b[k = j];

      /* If no bins big enough for *s[i], try next m */
      if (t == c + 1)
        break;

      /* Diminish that bin and save where it goes */
      b[k] -= *s[i];
      a[(int)((ulg huge *)(s[i]) - (ulg huge *)e)] = k;
    }

    /* Clean up */
    free((zvoid *)b);

    /* Do until all items put in a bin */
  } while (i < n);

  /* Done--clean up and return the number of bins needed */
  free((zvoid *)s);
  free((zvoid *)e);
  return m;
}


local int retry()
{
  char m[10];
  fputs("Error writing to disk--redo entire disk? ", stderr);
  fgets(m, 10, stdin);
  return *m == 'y' || *m == 'Y';
}


#ifndef USE_ZIPSPLITMAIN
int main(argc, argv)
#else
int zipsplitmain(argc, argv)
#endif

int argc;               /* number of tokens in command line */
char **argv;            /* command line tokens */
/* Split a zip file into several zip files less than a specified size.  See
   the command help in help() above. */
{
  ulg *a;               /* malloc'ed list of sizes, dest bins */
  extent *b;            /* heads of bin linked lists (malloc'ed) */
  ulg c;                /* bin capacity, start of central directory */
  int d;                /* if true, just report the number of disks */
  FILE *e;              /* input zip file */
  FILE *f;              /* output index and zip files */
  extent g;             /* number of bins from greedy(), entry to write */
  int h;                /* how to split--true means simple split, counter */
  ulg i = 0;            /* size of index file plus room to leave */
  extent j;             /* steps through zip entries, bins */
  int k;                /* next argument type */
  extent *n = NULL;     /* next item in bin list (heads in b) */
  ulg *p;               /* malloc'ed list of sizes, dest bins for greedy() */
  char *q;              /* steps through option characters */
  int r;                /* temporary variable, counter */
  extent s;             /* number of bins needed */
  ulg t;                /* total of sizes, end of central directory */
  int u;                /* flag to wait for user on output files */
  struct zlist far **w; /* malloc'ed table for zfiles linked list */
  int x;                /* if true, make an index file */
  struct zlist far *z;  /* steps through zfiles linked list */
#ifdef AMIGA
  char tailchar;         /* temporary variable used in name generation below */
#endif

#ifdef THEOS
  setlocale(LC_CTYPE, "I");
#endif

  /* If no args, show help */
  if (argc == 1)
  {
    help();
    EXIT(0);
  }

  init_upper();           /* build case map table */

  /* Go through args */
  signal(SIGINT, handler);
#ifdef SIGTERM                 /* Amiga has no SIGTERM */
  signal(SIGTERM, handler);
#endif
  k = h = x = d = u = 0;
  c = DEFSIZ;
  for (r = 1; r < argc; r++)
    if (*argv[r] == '-')
    {
      if (argv[r][1])
        for (q = argv[r]+1; *q; q++)
          switch (*q)
          {
            case 'b':   /* Specify path for output files */
              if (k)
                ziperr(ZE_PARMS, "options are separate and precede zip file");
              else
                k = 1;          /* Next non-option is path */
              break;
            case 'h':   /* Show help */
              help();  EXIT(0);
            case 'i':   /* Make an index file */
              x = 1;
              break;
            case 'l': case 'L':  /* Show copyright and disclaimer */
              license();  EXIT(0);
            case 'n':   /* Specify maximum size of resulting zip files */
              if (k)
                ziperr(ZE_PARMS, "options are separate and precede zip file");
              else
                k = 2;          /* Next non-option is size */
              break;
            case 'p':
              u = 1;
              break;
            case 'r':
              if (k)
                ziperr(ZE_PARMS, "options are separate and precede zip file");
              else
                k = 3;          /* Next non-option is room to leave */
              break;
            case 's':
              h = 1;    /* Only try simple */
              break;
            case 't':   /* Just report number of disks */
              d = 1;
              break;
            case 'v':   /* Show version info */
              version_info();  EXIT(0);
            default:
              ziperr(ZE_PARMS, "Use option -h for help.");
          }
      else
        ziperr(ZE_PARMS, "zip file cannot be stdin");
    }
    else
      switch (k)
      {
        case 0:
          if (zipfile == NULL)
          {
            if ((zipfile = ziptyp(argv[r])) == NULL)
              ziperr(ZE_MEM, "was processing arguments");
          }
          else
            ziperr(ZE_PARMS, "can only specify one zip file");
          break;
        case 1:
          tempath = argv[r];
          k = 0;
          break;
        case 2:
          if ((c = (ulg)atol(argv[r])) < 100)   /* 100 is smallest zip file */
            ziperr(ZE_PARMS, "invalid size given. Use option -h for help.");
          k = 0;
          break;
        default:        /* k must be 3 */
          i = (ulg)atol(argv[r]);
          k = 0;
          break;
      }
  if (zipfile == NULL)
    ziperr(ZE_PARMS, "need to specify zip file");


  /* Read zip file */
  if ((r = readzipfile()) != ZE_OK)
    ziperr(r, zipfile);
  if (zfiles == NULL)
    ziperr(ZE_NAME, zipfile);

  /* Make a list of sizes and check against capacity.  Also compute the
     size of the index file. */
  c -= ENDHEAD + 4;                     /* subtract overhead/zipfile */
  if ((a = (ulg *)talloc(zcount * sizeof(ulg))) == NULL ||
      (w = (struct zlist far **)talloc(zcount * sizeof(struct zlist far *))) ==
       NULL)
  {
    ziperr(ZE_MEM, "was computing split");
    return 1;
  }
  t = 0;
  for (j = 0, z = zfiles; j < zcount; j++, z = z->nxt)
  {
    w[j] = z;
    if (x)
      i += z->nam + 6 + NL;
    t += a[j] = 8 + LOCHEAD + CENHEAD +
           2 * (ulg)z->nam + 2 * (ulg)z->ext + z->com + z->siz;
    if (a[j] > c)
      ziperr(ZE_BIG, z->zname);
  }

  /* Decide on split to use, report number of files */
  if (h)
    s = simple(a, zcount, c, i);
  else
  {
    if ((p = (ulg *)talloc(zcount * sizeof(ulg))) == NULL)
      ziperr(ZE_MEM, "was computing split");
    memcpy((char *)p, (char *)a, zcount * sizeof(ulg));
    s = simple(a, zcount, c, i);
    g = greedy(p, zcount, c, i);
    if (s <= g)
      tfree((zvoid *)p);
    else
    {
      tfree((zvoid *)a);
      a = p;
      s = g;
    }
  }
  printf("%ld zip files w%s be made (%ld%% efficiency)\n",
         (ulg)s, d ? "ould" : "ill", ((200 * ((t + c - 1)/c)) / s + 1) >> 1);
  if (d)
  {
    tfreeall();
    free((zvoid *)zipfile);
    zipfile = NULL;
    EXIT(0);
  }

  /* Set up path for output files */
  /* Point "name" past the path, where the filename should go */
  if ((path = (char *)talloc(tempath == NULL ? 13 : strlen(tempath) + 14)) ==
      NULL)
    ziperr(ZE_MEM, "was making output file names");
  if (tempath == NULL)
     name = path;
  else
  {
#ifndef VM_CMS
    /* Copy the output path to the target */
    strcpy(path, tempath);
#endif
#ifdef AMIGA
    tailchar = path[strlen(path) - 1];  /* last character */
    if (path[0] && (tailchar != '/') && (tailchar != ':'))
      strcat(path, "/");
    name = path + strlen(path);
#else
#  ifdef RISCOS
    if (path[0] && path[strlen(path) - 1] != '.')
      strcat(path, ".");
    name = path + strlen(path);
#  else /* !RISCOS */
#   ifndef QDOS
    if (path[0] && path[strlen(path) - 1] != '/')
      strcat(path, "/");
#   else
    if (path[0] && path[strlen(path) - 1] != '_')
      strcat(path, "_");
#   endif
    name = path + strlen(path);
#  endif
#endif /* ?AMIGA */
  }

  /* Make linked lists of results */
  if ((b = (extent *)talloc(s * sizeof(extent))) == NULL ||
      (n = (extent *)talloc(zcount * sizeof(extent))) == NULL)
    ziperr(ZE_MEM, "was computing split");
  for (j = 0; j < s; j++)
    b[j] = (extent)-1;
  j = zcount;
  while (j--)
  {
    g = (extent)a[j];
    n[j] = b[g];
    b[g] = j;
  }

  /* Make a name template for the zip files that is eight or less characters
     before the .zip, and that will not overwrite the original zip file. */
  for (k = 1, j = s; j >= 10; j /= 10)
    k++;
  if (k > 7)
    ziperr(ZE_PARMS, "way too many zip files must be made");
/*
 * XXX, ugly ....
 */
/* Find the final "path" separator character */
#ifdef QDOS
  q = LastDir(zipfile);
#else
#ifdef VMS
  if ((q = strrchr(zipfile, ']')) != NULL)
#else
#ifdef AMIGA
  if (((q = strrchr(zipfile, '/')) != NULL)
                       || ((q = strrchr(zipfile, ':'))) != NULL)
#else
#ifdef RISCOS
  if ((q = strrchr(zipfile, '.')) != NULL)
#else
#ifdef MVS
  if ((q = strrchr(zipfile, '.')) != NULL)
#else
  if ((q = strrchr(zipfile, '/')) != NULL)
#endif /* MVS */
#endif /* RISCOS */
#endif /* AMIGA */
#endif /* VMS */
    q++;
  else
    q = zipfile;
#endif /* QDOS */

  r = 0;
  while ((g = *q++) != '\0' && g != ZPATH_SEP && r < 8 - k)
    template[r++] = (char)g;
  if (r == 0)
    template[r++] = '_';
  else if (g >= '0' && g <= '9')
    template[r - 1] = (char)(template[r - 1] == '_' ? '-' : '_');
  sprintf(template + r, TEMPL_FMT, k);
#ifdef VM_CMS
  /* For CMS, add the "path" as the filemode at the end */
  if (tempath)
  {
     strcat(template,".");
     strcat(template,tempath);
  }
#endif

  /* Make the zip files from the linked lists of entry numbers */
  if ((e = fopen(zipfile, FOPR)) == NULL)
    ziperr(ZE_NAME, zipfile);
  free((zvoid *)zipfile);
  zipfile = NULL;
  for (j = 0; j < s; j++)
  {
    /* jump here on a disk retry */
   redobin:

    /* prompt if requested */
    if (u)
    {
      char m[10];
      fprintf(stderr, "Insert disk #%ld of %ld and hit return: ",
              (ulg)j + 1, (ulg)s);
      fgets(m, 10, stdin);
    }

    /* write index file on first disk if requested */
    if (j == 0 && x)
    {
      strcpy(name, INDEX);
      printf("creating: %s\n", path);
      indexmade = 1;
      if ((f = fopen(path, "w")) == NULL)
      {
        if (u && retry()) goto redobin;
        ziperr(ZE_CREAT, path);
      }
      for (j = 0; j < zcount; j++)
        fprintf(f, "%5ld %s\n", a[j] + 1, w[j]->zname);
      if ((j = ferror(f)) != 0 || fclose(f))
      {
        if (j)
          fclose(f);
        if (u && retry()) goto redobin;
        ziperr(ZE_WRITE, path);
      }
    }

    /* create output zip file j */
    sprintf(name, template, j + 1L);
    printf("creating: %s\n", path);
    zipsmade = j + 1;
    if ((f = fopen(path, FOPW)) == NULL)
    {
      if (u && retry()) goto redobin;
      ziperr(ZE_CREAT, path);
    }
    tempzn = 0;

    /* write local headers and copy compressed data */
    for (g = b[j]; g != (extent)-1; g = (extent)n[g])
    {
      if (fseek(e, w[g]->off, SEEK_SET))
        ziperr(ferror(e) ? ZE_READ : ZE_EOF, zipfile);
      if ((r = zipcopy(w[g], e, f)) != ZE_OK)
      {
        if (r == ZE_TEMP)
        {
          if (u && retry()) goto redobin;
          ziperr(ZE_WRITE, path);
        }
        else
          ziperr(r, zipfile);
      }
    }

    /* write central headers */
    if ((c = ftell(f)) == (ulg)(-1L))
    {
      if (u && retry()) goto redobin;
      ziperr(ZE_WRITE, path);
    }
    for (g = b[j], k = 0; g != (extent)-1; g = n[g], k++)
      if ((r = putcentral(w[g], f)) != ZE_OK)
      {
        if (u && retry()) goto redobin;
        ziperr(ZE_WRITE, path);
      }

    /* write end-of-central header */
    if ((t = ftell(f)) == (ulg)(-1L) ||
        (r = putend(k, t - c, c, (extent)0, (char *)NULL, f)) != ZE_OK ||
        ferror(f) || fclose(f))
    {
      if (u && retry()) goto redobin;
      ziperr(ZE_WRITE, path);
    }
#ifdef RISCOS
    /* Set the filetype to &DDC */
    setfiletype(path,0xDDC);
#endif
  }
  fclose(e);

  /* Done! */
  if (u)
    fputs("Done.\n", stderr);
  tfreeall();

  RETURN(0);
}
