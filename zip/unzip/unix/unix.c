/*
  Copyright (c) 1990-2002 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*---------------------------------------------------------------------------

  unix.c

  Unix-specific routines for use with Info-ZIP's UnZip 5.41 and later.

  Contains:  readdir()
             do_wild()           <-- generic enough to put in fileio.c?
             mapattr()
             mapname()
             checkdir()
             mkdir()
             close_outfile()
             set_direc_attribs()
             stamp_file()
             version()

  ---------------------------------------------------------------------------*/


#define UNZIP_INTERNAL
#include "unzip.h"

#ifdef SCO_XENIX
#  define SYSNDIR
#else  /* SCO Unix, AIX, DNIX, TI SysV, Coherent 4.x, ... */
#  if defined(__convexc__) || defined(SYSV) || defined(CRAY) || defined(BSD4_4)
#    define DIRENT
#  endif
#endif
#if defined(_AIX) || defined(__mpexl)
#  define DIRENT
#endif
#ifdef COHERENT
#  if defined(_I386) || (defined(__COHERENT__) && (__COHERENT__ >= 0x420))
#    define DIRENT
#  endif
#endif

/* GRR:  may need to uncomment this: */
#if 0
#if defined(_POSIX_VERSION)
#  define DIRENT
#endif
#endif

#ifdef DIRENT
#  include <dirent.h>
#else
#  ifdef SYSV
#    ifdef SYSNDIR
#      include <sys/ndir.h>
#    else
#      include <ndir.h>
#    endif
#  else /* !SYSV */
#    ifndef NO_SYSDIR
#      include <sys/dir.h>
#    endif
#  endif /* ?SYSV */
#  ifndef dirent
#    define dirent direct
#  endif
#endif /* ?DIRENT */

#ifdef ACORN_FTYPE_NFS
/* Acorn bits for NFS filetyping */
typedef struct {
  uch ID[2];
  uch size[2];
  uch ID_2[4];
  uch loadaddr[4];
  uch execaddr[4];
  uch attr[4];
} RO_extra_block;

#endif /* ACORN_FTYPE_NFS */

/* static int created_dir;      */      /* used in mapname(), checkdir() */
/* static int renamed_fullpath; */      /* ditto */


#ifndef SFX
#ifdef NO_DIR                  /* for AT&T 3B1 */

#define opendir(path) fopen(path,"r")
#define closedir(dir) fclose(dir)
typedef FILE DIR;
typedef struct zdir {
    FILE *dirhandle;
    struct dirent *entry;
} DIR
DIR *opendir OF((ZCONST char *dirspec));
void closedir OF((DIR *dirp));
struct dirent *readdir OF((DIR *dirp));

DIR *opendir(dirspec)
    ZCONST char *dirspec;
{
    DIR *dirp;

    if ((dirp = malloc(sizeof(DIR)) != NULL) {
        if ((dirp->dirhandle = fopen(dirspec, "r")) == NULL) {
            free(dirp);
            dirp = NULL;
        }
    }
    return dirp;
}

void closedir(dirp)
    DIR *dirp;
{
    fclose(dirp->dirhandle);
    free(dirp);
}

/*
 *  Apparently originally by Rich Salz.
 *  Cleaned up and modified by James W. Birdsall.
 */
struct dirent *readdir(dirp)
    DIR *dirp;
{

    if (dirp == NULL)
        return NULL;

    for (;;)
        if (fread(&(dirp->entry), sizeof (struct dirent), 1,
                  dirp->dirhandle) == 0)
            return (struct dirent *)NULL;
        else if ((dirp->entry).d_ino)
            return &(dirp->entry);

} /* end function readdir() */

#endif /* NO_DIR */


/**********************/
/* Function do_wild() */   /* for porting:  dir separator; match(ignore_case) */
/**********************/

char *do_wild(__G__ wildspec)
    __GDEF
    ZCONST char *wildspec;  /* only used first time on a given dir */
{
/* these statics are now declared in SYSTEM_SPECIFIC_GLOBALS in unxcfg.h:
    static DIR *wild_dir = (DIR *)NULL;
    static ZCONST char *wildname;
    static char *dirname, matchname[FILNAMSIZ];
    static int notfirstcall=FALSE, have_dirname, dirnamelen;
*/
    struct dirent *file;

    /* Even when we're just returning wildspec, we *always* do so in
     * matchname[]--calling routine is allowed to append four characters
     * to the returned string, and wildspec may be a pointer to argv[].
     */
    if (!G.notfirstcall) {  /* first call:  must initialize everything */
        G.notfirstcall = TRUE;

        if (!iswild(wildspec)) {
            strcpy(G.matchname, wildspec);
            G.have_dirname = FALSE;
            G.wild_dir = NULL;
            return G.matchname;
        }

        /* break the wildspec into a directory part and a wildcard filename */
        if ((G.wildname = (ZCONST char *)strrchr(wildspec, '/')) == NULL) {
            G.dirname = ".";
            G.dirnamelen = 1;
            G.have_dirname = FALSE;
            G.wildname = wildspec;
        } else {
            ++G.wildname;     /* point at character after '/' */
            G.dirnamelen = G.wildname - wildspec;
            if ((G.dirname = (char *)malloc(G.dirnamelen+1)) == (char *)NULL) {
                Info(slide, 0x201, ((char *)slide,
                  "warning:  cannot allocate wildcard buffers\n"));
                strcpy(G.matchname, wildspec);
                return G.matchname; /* but maybe filespec was not a wildcard */
            }
            strncpy(G.dirname, wildspec, G.dirnamelen);
            G.dirname[G.dirnamelen] = '\0';   /* terminate for strcpy below */
            G.have_dirname = TRUE;
        }

        if ((G.wild_dir = (zvoid *)opendir(G.dirname)) != (zvoid *)NULL) {
            while ((file = readdir((DIR *)G.wild_dir)) !=
                   (struct dirent *)NULL) {
                Trace((stderr, "do_wild:  readdir returns %s\n",
                  FnFilter1(file->d_name)));
                if (file->d_name[0] == '.' && G.wildname[0] != '.')
                    continue; /* Unix:  '*' and '?' do not match leading dot */
                if (match(file->d_name, G.wildname, 0) && /* 0 == case sens. */
                    /* skip "." and ".." directory entries */
                    strcmp(file->d_name, ".") && strcmp(file->d_name, "..")) {
                    Trace((stderr, "do_wild:  match() succeeds\n"));
                    if (G.have_dirname) {
                        strcpy(G.matchname, G.dirname);
                        strcpy(G.matchname+G.dirnamelen, file->d_name);
                    } else
                        strcpy(G.matchname, file->d_name);
                    return G.matchname;
                }
            }
            /* if we get to here directory is exhausted, so close it */
            closedir((DIR *)G.wild_dir);
            G.wild_dir = (zvoid *)NULL;
        }
        Trace((stderr, "do_wild:  opendir(%s) returns NULL\n",
          FnFilter1(G.dirname)));

        /* return the raw wildspec in case that works (e.g., directory not
         * searchable, but filespec was not wild and file is readable) */
        strcpy(G.matchname, wildspec);
        return G.matchname;
    }

    /* last time through, might have failed opendir but returned raw wildspec */
    if ((DIR *)G.wild_dir == (DIR *)NULL) {
        G.notfirstcall = FALSE; /* nothing left--reset for new wildspec */
        if (G.have_dirname)
            free(G.dirname);
        return (char *)NULL;
    }

    /* If we've gotten this far, we've read and matched at least one entry
     * successfully (in a previous call), so dirname has been copied into
     * matchname already.
     */
    while ((file = readdir((DIR *)G.wild_dir)) != (struct dirent *)NULL) {
        Trace((stderr, "do_wild:  readdir returns %s\n",
          FnFilter1(file->d_name)));
        if (file->d_name[0] == '.' && G.wildname[0] != '.')
            continue;   /* Unix:  '*' and '?' do not match leading dot */
        if (match(file->d_name, G.wildname, 0)) { /* 0 == don't ignore case */
            Trace((stderr, "do_wild:  match() succeeds\n"));
            if (G.have_dirname) {
                /* strcpy(G.matchname, G.dirname); */
                strcpy(G.matchname+G.dirnamelen, file->d_name);
            } else
                strcpy(G.matchname, file->d_name);
            return G.matchname;
        }
    }

    closedir((DIR *)G.wild_dir);  /* at least one entry read; nothing left */
    G.wild_dir = (zvoid *)NULL;
    G.notfirstcall = FALSE;       /* reset for new wildspec */
    if (G.have_dirname)
        free(G.dirname);
    return (char *)NULL;

} /* end function do_wild() */

#endif /* !SFX */





/**********************/
/* Function mapattr() */
/**********************/

int mapattr(__G)
    __GDEF
{
    ulg tmp = G.crec.external_file_attributes;

    G.pInfo->file_attr = 0;
    /* initialized to 0 for check in "default" branch below... */

    switch (G.pInfo->hostnum) {
        case AMIGA_:
            tmp = (unsigned)(tmp>>17 & 7);   /* Amiga RWE bits */
            G.pInfo->file_attr = (unsigned)(tmp<<6 | tmp<<3 | tmp);
            break;
        case THEOS_:
            tmp &= 0xF1FFFFFFL;
            if ((tmp & 0xF0000000L) != 0x40000000L)
                tmp &= 0x01FFFFFFL;     /* not a dir, mask all ftype bits */
            else
                tmp &= 0x41FFFFFFL;     /* leave directory bit as set */
            /* fall through! */
        case UNIX_:
        case VMS_:
        case ACORN_:
        case ATARI_:
        case BEOS_:
        case QDOS_:
        case TANDEM_:
            G.pInfo->file_attr = (unsigned)(tmp >> 16);
            if (G.pInfo->file_attr != 0 || !G.extra_field) {
                return 0;
            } else {
                /* Some (non-Info-ZIP) implementations of Zip for Unix and
                 * VMS (and probably others ??) leave 0 in the upper 16-bit
                 * part of the external_file_attributes field. Instead, they
                 * store file permission attributes in some extra field.
                 * As a work-around, we search for the presence of one of
                 * these extra fields and fall back to the MSDOS compatible
                 * part of external_file_attributes if one of the known
                 * e.f. types has been detected.
                 * Later, we might implement extraction of the permission
                 * bits from the VMS extra field. But for now, the work-around
                 * should be sufficient to provide "readable" extracted files.
                 * (For ASI Unix e.f., an experimental remap from the e.f.
                 * mode value IS already provided!)
                 */
                ush ebID;
                unsigned ebLen;
                uch *ef = G.extra_field;
                unsigned ef_len = G.crec.extra_field_length;
                int r = FALSE;

                while (!r && ef_len >= EB_HEADSIZE) {
                    ebID = makeword(ef);
                    ebLen = (unsigned)makeword(ef+EB_LEN);
                    if (ebLen > (ef_len - EB_HEADSIZE))
                        /* discoverd some e.f. inconsistency! */
                        break;
                    switch (ebID) {
                      case EF_ASIUNIX:
                        if (ebLen >= (EB_ASI_MODE+2)) {
                            G.pInfo->file_attr =
                              (unsigned)makeword(ef+(EB_HEADSIZE+EB_ASI_MODE));
                            /* force stop of loop: */
                            ef_len = (ebLen + EB_HEADSIZE);
                            break;
                        }
                        /* else: fall through! */
                      case EF_PKVMS:
                        /* "found nondecypherable e.f. with perm. attr" */
                        r = TRUE;
                      default:
                        break;
                    }
                    ef_len -= (ebLen + EB_HEADSIZE);
                    ef += (ebLen + EB_HEADSIZE);
                }
                if (!r)
                    return 0;
            }
            /* fall through! */
        /* all remaining cases:  expand MSDOS read-only bit into write perms */
        case FS_FAT_:
            /* PKWARE's PKZip for Unix marks entries as FS_FAT_, but stores the
             * Unix attributes in the upper 16 bits of the external attributes
             * field, just like Info-ZIP's Zip for Unix.  We try to use that
             * value, after a check for consistency with the MSDOS attribute
             * bits (see below).
             */
            G.pInfo->file_attr = (unsigned)(tmp >> 16);
            /* fall through! */
        case FS_HPFS_:
        case FS_NTFS_:
        case MAC_:
        case TOPS20_:
        default:
            /* Ensure that DOS subdir bit is set when the entry's name ends
             * in a '/'.  Some third-party Zip programs fail to set the subdir
             * bit for directory entries.
             */
            if ((tmp & 0x10) == 0) {
                extent fnlen = strlen(G.filename);
                if (fnlen > 0 && G.filename[fnlen-1] == '/')
                    tmp |= 0x10;
            }
            /* read-only bit --> write perms; subdir bit --> dir exec bit */
            tmp = !(tmp & 1) << 1  |  (tmp & 0x10) >> 4;
            if ((G.pInfo->file_attr & 0700) == (unsigned)(0400 | tmp<<6))
                /* keep previous G.pInfo->file_attr setting, when its "owner"
                 * part appears to be consistent with DOS attribute flags!
                 */
                return 0;
            G.pInfo->file_attr = (unsigned)(0444 | tmp<<6 | tmp<<3 | tmp);
            break;
    } /* end switch (host-OS-created-by) */

    /* for originating systems with no concept of "group," "other," "system": */
    umask( (int)(tmp=umask(0)) );    /* apply mask to expanded r/w(/x) perms */
    G.pInfo->file_attr &= ~tmp;

    return 0;

} /* end function mapattr() */





/************************/
/*  Function mapname()  */
/************************/

int mapname(__G__ renamed)
    __GDEF
    int renamed;
/*
 * returns:
 *  MPN_OK          - no problem detected
 *  MPN_INF_TRUNC   - caution (truncated filename)
 *  MPN_INF_SKIP    - info "skip entry" (dir doesn't exist)
 *  MPN_ERR_SKIP    - error -> skip entry
 *  MPN_ERR_TOOLONG - error -> path is too long
 *  MPN_NOMEM       - error (memory allocation failed) -> skip entry
 *  [also MPN_VOL_LABEL, MPN_CREATED_DIR]
 */
{
    char pathcomp[FILNAMSIZ];      /* path-component buffer */
    char *pp, *cp=(char *)NULL;    /* character pointers */
    char *lastsemi=(char *)NULL;   /* pointer to last semi-colon in pathcomp */
#ifdef ACORN_FTYPE_NFS
    char *lastcomma=(char *)NULL;  /* pointer to last comma in pathcomp */
    RO_extra_block *ef_spark;      /* pointer Acorn FTYPE ef block */
#endif
    int quote = FALSE;             /* flags */
    int killed_ddot = FALSE;       /* is set when skipping "../" pathcomp */
    int error = MPN_OK;
    register unsigned workch;      /* hold the character being tested */


/*---------------------------------------------------------------------------
    Initialize various pointers and counters and stuff.
  ---------------------------------------------------------------------------*/

    if (G.pInfo->vollabel)
        return MPN_VOL_LABEL;   /* can't set disk volume labels in Unix */

    /* can create path as long as not just freshening, or if user told us */
    G.create_dirs = (!uO.fflag || renamed);

    G.created_dir = FALSE;      /* not yet */

    /* user gave full pathname:  don't prepend rootpath */
    G.renamed_fullpath = (renamed && (*G.filename == '/'));

    if (checkdir(__G__ (char *)NULL, INIT) == MPN_NOMEM)
        return MPN_NOMEM;       /* initialize path buffer, unless no memory */

    *pathcomp = '\0';           /* initialize translation buffer */
    pp = pathcomp;              /* point to translation buffer */
    if (uO.jflag)               /* junking directories */
        cp = (char *)strrchr(G.filename, '/');
    if (cp == (char *)NULL)     /* no '/' or not junking dirs */
        cp = G.filename;        /* point to internal zipfile-member pathname */
    else
        ++cp;                   /* point to start of last component of path */

/*---------------------------------------------------------------------------
    Begin main loop through characters in filename.
  ---------------------------------------------------------------------------*/

    while ((workch = (uch)*cp++) != 0) {

        if (quote) {                 /* if character quoted, */
            *pp++ = (char)workch;    /*  include it literally */
            quote = FALSE;
        } else
            switch (workch) {
            case '/':             /* can assume -j flag not given */
                *pp = '\0';
                if (((error = checkdir(__G__ pathcomp, APPEND_DIR)) & MPN_MASK)
                     > MPN_INF_TRUNC)
                    return error;
                pp = pathcomp;    /* reset conversion buffer for next piece */
                lastsemi = (char *)NULL; /* leave directory semi-colons alone */
                break;

            case '.':
                if (pp == pathcomp) {   /* nothing appended yet... */
                    if (*cp == '/') {   /* don't bother appending "./" to */
                        ++cp;           /*  the path: skip behind the '/' */
                        break;
                    } else if (!uO.ddotflag && *cp == '.' && cp[1] == '/') {
                        /* "../" dir traversal detected */
                        cp += 2;        /*  skip over behind the '/' */
                        killed_ddot = TRUE; /*  set "show message" flag */
                        break;
                    }
                }
                *pp++ = '.';
                break;

            case ';':             /* VMS version (or DEC-20 attrib?) */
                lastsemi = pp;
                *pp++ = ';';      /* keep for now; remove VMS ";##" */
                break;            /*  later, if requested */

#ifdef ACORN_FTYPE_NFS
            case ',':             /* NFS filetype extension */
                lastcomma = pp;
                *pp++ = ',';      /* keep for now; may need to remove */
                break;            /*  later, if requested */
#endif

            case '\026':          /* control-V quote for special chars */
                quote = TRUE;     /* set flag for next character */
                break;

#ifdef MTS
            case ' ':             /* change spaces to underscore under */
                *pp++ = '_';      /*  MTS; leave as spaces under Unix */
                break;
#endif

            default:
                /* allow European characters in filenames: */
                if (isprint(workch) || (128 <= workch && workch <= 254))
                    *pp++ = (char)workch;
            } /* end switch */

    } /* end while loop */

    /* Show warning when stripping insecure "parent dir" path components */
    if (killed_ddot && QCOND2) {
        Info(slide, 0, ((char *)slide,
          "warning:  skipped \"../\" path component(s) in %s\n",
          FnFilter1(G.filename)));
        if (!(error & ~MPN_MASK))
            error = (error & MPN_MASK) | PK_WARN;
    }

/*---------------------------------------------------------------------------
    Report if directory was created (and no file to create:  filename ended
    in '/'), check name to be sure it exists, and combine path and name be-
    fore exiting.
  ---------------------------------------------------------------------------*/

    if (G.filename[strlen(G.filename) - 1] == '/') {
        checkdir(__G__ G.filename, GETPATH);
        if (G.created_dir) {
            if (QCOND2) {
                Info(slide, 0, ((char *)slide, "   creating: %s\n",
                  FnFilter1(G.filename)));
            }
#ifndef NO_CHMOD
            /* set approx. dir perms (make sure can still read/write in dir) */
            if (chmod(G.filename, (0xffff & G.pInfo->file_attr) | 0700))
                perror("chmod (directory attributes) error");
#endif
            /* set dir time (note trailing '/') */
            return (error & ~MPN_MASK) | MPN_CREATED_DIR;
        }
        /* dir existed already; don't look for data to extract */
        return (error & ~MPN_MASK) | MPN_INF_SKIP;
    }

    *pp = '\0';                   /* done with pathcomp:  terminate it */

    /* if not saving them, remove VMS version numbers (appended ";###") */
    if (!uO.V_flag && lastsemi) {
        pp = lastsemi + 1;
        while (isdigit((uch)(*pp)))
            ++pp;
        if (*pp == '\0')          /* only digits between ';' and end:  nuke */
            *lastsemi = '\0';
    }

#ifdef ACORN_FTYPE_NFS
    /* translate Acorn filetype information if asked to do so */
    if (uO.acorn_nfs_ext &&
        (ef_spark = (RO_extra_block *)
                    getRISCOSexfield(G.extra_field, G.lrec.extra_field_length))
        != (RO_extra_block *)NULL)
    {
        /* file *must* have a RISC OS extra field */
        long ft = (long)makelong(ef_spark->loadaddr);
        /*32-bit*/
        if (lastcomma) {
            pp = lastcomma + 1;
            while (isxdigit((uch)(*pp))) ++pp;
            if (pp == lastcomma+4 && *pp == '\0') *lastcomma='\0'; /* nuke */
        }
        if ((ft & 1<<31)==0) ft=0x000FFD00;
        sprintf(pathcomp+strlen(pathcomp), ",%03x", (int)(ft>>8) & 0xFFF);
    }
#endif /* ACORN_FTYPE_NFS */

    if (*pathcomp == '\0') {
        Info(slide, 1, ((char *)slide, "mapname:  conversion of %s failed\n",
          FnFilter1(G.filename)));
        return (error & ~MPN_MASK) | MPN_ERR_SKIP;
    }

    checkdir(__G__ pathcomp, APPEND_NAME);  /* returns 1 if truncated: care? */
    checkdir(__G__ G.filename, GETPATH);

    return error;

} /* end function mapname() */




#if 0  /*========== NOTES ==========*/

  extract-to dir:      a:path/
  buildpath:           path1/path2/ ...   (NULL-terminated)
  pathcomp:                filename

  mapname():
    loop over chars in zipfile member name
      checkdir(path component, COMPONENT | CREATEDIR) --> map as required?
        (d:/tmp/unzip/)                    (disk:[tmp.unzip.)
        (d:/tmp/unzip/jj/)                 (disk:[tmp.unzip.jj.)
        (d:/tmp/unzip/jj/temp/)            (disk:[tmp.unzip.jj.temp.)
    finally add filename itself and check for existence? (could use with rename)
        (d:/tmp/unzip/jj/temp/msg.outdir)  (disk:[tmp.unzip.jj.temp]msg.outdir)
    checkdir(name, GETPATH)     -->  copy path to name and free space

#endif /* 0 */




/***********************/
/* Function checkdir() */
/***********************/

int checkdir(__G__ pathcomp, flag)
    __GDEF
    char *pathcomp;
    int flag;
/*
 * returns:
 *  MPN_OK          - no problem detected
 *  MPN_INF_TRUNC   - (on APPEND_NAME) truncated filename
 *  MPN_INF_SKIP    - path doesn't exist, not allowed to create
 *  MPN_ERR_SKIP    - path doesn't exist, tried to create and failed; or path
 *                    exists and is not a directory, but is supposed to be
 *  MPN_ERR_TOOLONG - path is too long
 *  MPN_NOMEM       - can't allocate memory for filename buffers
 */
{
 /* static int rootlen = 0; */  /* length of rootpath */
 /* static char *rootpath;  */  /* user's "extract-to" directory */
 /* static char *buildpath; */  /* full path (so far) to extracted file */
 /* static char *end;       */  /* pointer to end of buildpath ('\0') */

#   define FN_MASK   7
#   define FUNCTION  (flag & FN_MASK)



/*---------------------------------------------------------------------------
    APPEND_DIR:  append the path component to the path being built and check
    for its existence.  If doesn't exist and we are creating directories, do
    so for this one; else signal success or error as appropriate.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_DIR) {
        int too_long = FALSE;
#ifdef SHORT_NAMES
        char *old_end = end;
#endif

        Trace((stderr, "appending dir segment [%s]\n", FnFilter1(pathcomp)));
        while ((*G.end = *pathcomp++) != '\0')
            ++G.end;
#ifdef SHORT_NAMES   /* path components restricted to 14 chars, typically */
        if ((G.end-old_end) > FILENAME_MAX)  /* GRR:  proper constant? */
            *(G.end = old_end + FILENAME_MAX) = '\0';
#endif

        /* GRR:  could do better check, see if overrunning buffer as we go:
         * check end-buildpath after each append, set warning variable if
         * within 20 of FILNAMSIZ; then if var set, do careful check when
         * appending.  Clear variable when begin new path. */

        /* next check: need to append '/', at least one-char name, '\0' */
        if ((G.end-G.buildpath) > FILNAMSIZ-3)
            too_long = TRUE;                    /* check if extracting dir? */
        if (SSTAT(G.buildpath, &G.statbuf)) {   /* path doesn't exist */
            if (!G.create_dirs) { /* told not to create (freshening) */
                free(G.buildpath);
                return MPN_INF_SKIP;    /* path doesn't exist: nothing to do */
            }
            if (too_long) {
                Info(slide, 1, ((char *)slide,
                  "checkdir error:  path too long: %s\n",
                  FnFilter1(G.buildpath)));
                free(G.buildpath);
                /* no room for filenames:  fatal */
                return MPN_ERR_TOOLONG;
            }
            if (mkdir(G.buildpath, 0777) == -1) {   /* create the directory */
                Info(slide, 1, ((char *)slide,
                  "checkdir error:  cannot create %s\n\
                 unable to process %s.\n",
                  FnFilter2(G.buildpath), FnFilter1(G.filename)));
                free(G.buildpath);
                /* path didn't exist, tried to create, failed */
                return MPN_ERR_SKIP;
            }
            G.created_dir = TRUE;
        } else if (!S_ISDIR(G.statbuf.st_mode)) {
            Info(slide, 1, ((char *)slide,
              "checkdir error:  %s exists but is not directory\n\
                 unable to process %s.\n",
              FnFilter2(G.buildpath), FnFilter1(G.filename)));
            free(G.buildpath);
            /* path existed but wasn't dir */
            return MPN_ERR_SKIP;
        }
        if (too_long) {
            Info(slide, 1, ((char *)slide,
              "checkdir error:  path too long: %s\n", FnFilter1(G.buildpath)));
            free(G.buildpath);
            /* no room for filenames:  fatal */
            return MPN_ERR_TOOLONG;
        }
        *G.end++ = '/';
        *G.end = '\0';
        Trace((stderr, "buildpath now = [%s]\n", FnFilter1(G.buildpath)));
        return MPN_OK;

    } /* end if (FUNCTION == APPEND_DIR) */

/*---------------------------------------------------------------------------
    GETPATH:  copy full path to the string pointed at by pathcomp, and free
    G.buildpath.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == GETPATH) {
        strcpy(pathcomp, G.buildpath);
        Trace((stderr, "getting and freeing path [%s]\n",
          FnFilter1(pathcomp)));
        free(G.buildpath);
        G.buildpath = G.end = (char *)NULL;
        return MPN_OK;
    }

/*---------------------------------------------------------------------------
    APPEND_NAME:  assume the path component is the filename; append it and
    return without checking for existence.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_NAME) {
#ifdef SHORT_NAMES
        char *old_end = end;
#endif

        Trace((stderr, "appending filename [%s]\n", FnFilter1(pathcomp)));
        while ((*G.end = *pathcomp++) != '\0') {
            ++G.end;
#ifdef SHORT_NAMES  /* truncate name at 14 characters, typically */
            if ((G.end-old_end) > FILENAME_MAX)    /* GRR:  proper constant? */
                *(G.end = old_end + FILENAME_MAX) = '\0';
#endif
            if ((G.end-G.buildpath) >= FILNAMSIZ) {
                *--G.end = '\0';
                Info(slide, 0x201, ((char *)slide,
                  "checkdir warning:  path too long; truncating\n\
                   %s\n                -> %s\n",
                  FnFilter1(G.filename), FnFilter2(G.buildpath)));
                return MPN_INF_TRUNC;   /* filename truncated */
            }
        }
        Trace((stderr, "buildpath now = [%s]\n", FnFilter1(G.buildpath)));
        /* could check for existence here, prompt for new name... */
        return MPN_OK;
    }

/*---------------------------------------------------------------------------
    INIT:  allocate and initialize buffer space for the file currently being
    extracted.  If file was renamed with an absolute path, don't prepend the
    extract-to path.
  ---------------------------------------------------------------------------*/

/* GRR:  for VMS and TOPS-20, add up to 13 to strlen */

    if (FUNCTION == INIT) {
        Trace((stderr, "initializing buildpath to "));
#ifdef ACORN_FTYPE_NFS
        if ((G.buildpath = (char *)malloc(strlen(G.filename)+G.rootlen+
                                          (uO.acorn_nfs_ext ? 5 : 1)))
#else
        if ((G.buildpath = (char *)malloc(strlen(G.filename)+G.rootlen+1))
#endif
            == (char *)NULL)
            return MPN_NOMEM;
        if ((G.rootlen > 0) && !G.renamed_fullpath) {
            strcpy(G.buildpath, G.rootpath);
            G.end = G.buildpath + G.rootlen;
        } else {
            *G.buildpath = '\0';
            G.end = G.buildpath;
        }
        Trace((stderr, "[%s]\n", FnFilter1(G.buildpath)));
        return MPN_OK;
    }

/*---------------------------------------------------------------------------
    ROOT:  if appropriate, store the path in rootpath and create it if
    necessary; else assume it's a zipfile member and return.  This path
    segment gets used in extracting all members from every zipfile specified
    on the command line.
  ---------------------------------------------------------------------------*/

#if (!defined(SFX) || defined(SFX_EXDIR))
    if (FUNCTION == ROOT) {
        Trace((stderr, "initializing root path to [%s]\n",
          FnFilter1(pathcomp)));
        if (pathcomp == (char *)NULL) {
            G.rootlen = 0;
            return MPN_OK;
        }
        if (G.rootlen > 0)      /* rootpath was already set, nothing to do */
            return MPN_OK;
        if ((G.rootlen = strlen(pathcomp)) > 0) {
            char *tmproot;

            if ((tmproot = (char *)malloc(G.rootlen+2)) == (char *)NULL) {
                G.rootlen = 0;
                return MPN_NOMEM;
            }
            strcpy(tmproot, pathcomp);
            if (tmproot[G.rootlen-1] == '/') {
                tmproot[--G.rootlen] = '\0';
            }
            if (G.rootlen > 0 && (SSTAT(tmproot, &G.statbuf) ||
                                  !S_ISDIR(G.statbuf.st_mode)))
            {   /* path does not exist */
                if (!G.create_dirs /* || iswild(tmproot) */ ) {
                    free(tmproot);
                    G.rootlen = 0;
                    /* skip (or treat as stored file) */
                    return MPN_INF_SKIP;
                }
                /* create the directory (could add loop here scanning tmproot
                 * to create more than one level, but why really necessary?) */
                if (mkdir(tmproot, 0777) == -1) {
                    Info(slide, 1, ((char *)slide,
                      "checkdir:  cannot create extraction directory: %s\n",
                      FnFilter1(tmproot)));
                    free(tmproot);
                    G.rootlen = 0;
                    /* path didn't exist, tried to create, and failed: */
                    /* file exists, or 2+ subdir levels required */
                    return MPN_ERR_SKIP;
                }
            }
            tmproot[G.rootlen++] = '/';
            tmproot[G.rootlen] = '\0';
            if ((G.rootpath = (char *)realloc(tmproot, G.rootlen+1)) == NULL) {
                free(tmproot);
                G.rootlen = 0;
                return MPN_NOMEM;
            }
            Trace((stderr, "rootpath now = [%s]\n", FnFilter1(G.rootpath)));
        }
        return MPN_OK;
    }
#endif /* !SFX || SFX_EXDIR */

/*---------------------------------------------------------------------------
    END:  free rootpath, immediately prior to program exit.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == END) {
        Trace((stderr, "freeing rootpath\n"));
        if (G.rootlen > 0) {
            free(G.rootpath);
            G.rootlen = 0;
        }
        return MPN_OK;
    }

    return MPN_INVALID; /* should never reach */

} /* end function checkdir() */





#ifdef NO_MKDIR

/********************/
/* Function mkdir() */
/********************/

int mkdir(path, mode)
    ZCONST char *path;
    int mode;   /* ignored */
/*
 * returns:   0 - successful
 *           -1 - failed (errno not set, however)
 */
{
    char command[FILNAMSIZ+40]; /* buffer for system() call */

    /* GRR 930416:  added single quotes around path to avoid bug with
     * creating directories with ampersands in name; not yet tested */
    sprintf(command, "IFS=\" \t\n\" /bin/mkdir '%s' 2>/dev/null", path);
    if (system(command))
        return -1;
    return 0;
}

#endif /* NO_MKDIR */





#ifndef MTS

/****************************/
/* Function close_outfile() */
/****************************/

void close_outfile(__G)    /* GRR: change to return PK-style warning level */
    __GDEF
{
    iztimes zt;
    ush z_uidgid[2];
    unsigned eb_izux_flg;

/*---------------------------------------------------------------------------
    If symbolic links are supported, allocate a storage area, put the uncom-
    pressed "data" in it, and create the link.  Since we know it's a symbolic
    link to start with, we shouldn't have to worry about overflowing unsigned
    ints with unsigned longs.
  ---------------------------------------------------------------------------*/

#ifdef SYMLINKS
    if (G.symlnk) {
        unsigned ucsize = (unsigned)G.lrec.ucsize;
        char *linktarget = (char *)malloc((unsigned)G.lrec.ucsize+1);

        fclose(G.outfile);                      /* close "data" file... */
        G.outfile = fopen(G.filename, FOPR);    /* ...and reopen for reading */
        if (!linktarget || fread(linktarget, 1, ucsize, G.outfile) !=
                           (int)ucsize)
        {
            Info(slide, 0x201, ((char *)slide,
              "warning:  symbolic link (%s) failed\n", FnFilter1(G.filename)));
            if (linktarget)
                free(linktarget);
            fclose(G.outfile);
            return;
        }
        fclose(G.outfile);                  /* close "data" file for good... */
        unlink(G.filename);                 /* ...and delete it */
        linktarget[ucsize] = '\0';
        if (QCOND2)
            Info(slide, 0, ((char *)slide, "-> %s ", FnFilter1(linktarget)));
        if (symlink(linktarget, G.filename))  /* create the real link */
            perror("symlink error");
        free(linktarget);
        return;                             /* can't set time on symlinks */
    }
#endif /* SYMLINKS */

    fclose(G.outfile);
#ifdef QLZIP
    if (G.extra_field) {
        static void qlfix OF((__GPRO__ uch *ef_ptr, unsigned ef_len));

        qlfix(__G__ G.extra_field, G.lrec.extra_field_length);
    }
#endif

/*---------------------------------------------------------------------------
    Convert from MSDOS-format local time and date to Unix-format 32-bit GMT
    time:  adjust base year from 1980 to 1970, do usual conversions from
    yy/mm/dd hh:mm:ss to elapsed seconds, and account for timezone and day-
    light savings time differences.  If we have a Unix extra field, however,
    we're laughing:  both mtime and atime are ours.  On the other hand, we
    then have to check for restoration of UID/GID.
  ---------------------------------------------------------------------------*/

    eb_izux_flg = (G.extra_field ? ef_scan_for_izux(G.extra_field,
                   G.lrec.extra_field_length, 0, G.lrec.last_mod_dos_datetime,
#ifdef IZ_CHECK_TZ
                   (G.tz_is_valid ? &zt : NULL),
#else
                   &zt,
#endif
                   z_uidgid) : 0);
    if (eb_izux_flg & EB_UT_FL_MTIME) {
        TTrace((stderr, "\nclose_outfile:  Unix e.f. modif. time = %ld\n",
          zt.mtime));
    } else {
        zt.mtime = dos_to_unix_time(G.lrec.last_mod_dos_datetime);
    }
    if (eb_izux_flg & EB_UT_FL_ATIME) {
        TTrace((stderr, "close_outfile:  Unix e.f. access time = %ld\n",
          zt.atime));
    } else {
        zt.atime = zt.mtime;
        TTrace((stderr, "\nclose_outfile:  modification/access times = %ld\n",
          zt.mtime));
    }

    /* if -X option was specified and we have UID/GID info, restore it */
    if (uO.X_flag && eb_izux_flg & EB_UX2_VALID) {
        TTrace((stderr, "close_outfile:  restoring Unix UID/GID info\n"));
        if (chown(G.filename, (uid_t)z_uidgid[0], (gid_t)z_uidgid[1]))
        {
            if (uO.qflag)
                Info(slide, 0x201, ((char *)slide,
                  "warning:  cannot set UID %d and/or GID %d for %s\n",
                  z_uidgid[0], z_uidgid[1], FnFilter1(G.filename)));
            else
                Info(slide, 0x201, ((char *)slide,
                  " (warning) cannot set UID %d and/or GID %d",
                  z_uidgid[0], z_uidgid[1]));
        }
    }

    /* set the file's access and modification times */
    if (utime(G.filename, (ztimbuf *)&zt)) {
#ifdef AOS_VS
        if (uO.qflag)
            Info(slide, 0x201, ((char *)slide, "... cannot set time for %s\n",
              FnFilter1(G.filename)));
        else
            Info(slide, 0x201, ((char *)slide, "... cannot set time"));
#else
        if (uO.qflag)
            Info(slide, 0x201, ((char *)slide,
              "warning:  cannot set times for %s\n", FnFilter1(G.filename)));
        else
            Info(slide, 0x201, ((char *)slide,
              " (warning) cannot set times"));
#endif /* ?AOS_VS */
    }

/*---------------------------------------------------------------------------
    Change the file permissions from default ones to those stored in the
    zipfile.
  ---------------------------------------------------------------------------*/

#ifndef NO_CHMOD
    if (chmod(G.filename, 0xffff & G.pInfo->file_attr))
        perror("chmod (file attributes) error");
#endif

} /* end function close_outfile() */

#endif /* !MTS */




#ifdef SET_DIR_ATTRIB
/* messages of code for setting directory attributes */
static ZCONST char Far DirlistUidGidFailed[] =
  "warning:  cannot set UID %d and/or GID %d for %s\n";
static ZCONST char Far DirlistUtimeFailed[] =
  "warning:  cannot set modification, access times for %s\n";
#  ifndef NO_CHMOD
  static ZCONST char Far DirlistChmodFailed[] =
    "warning:  cannot set permissions for %s\n";
#  endif


int set_direc_attribs(__G__ d)
    __GDEF
    dirtime *d;
{
    int errval = PK_OK;

    if (d->have_uidgid &&
        chown(d->fn, (uid_t)d->uidgid[0], (gid_t)d->uidgid[1]))
    {
        Info(slide, 0x201, ((char *)slide,
          LoadFarString(DirlistUidGidFailed),
          d->uidgid[0], d->uidgid[1], FnFilter1(d->fn)));
        if (!errval)
            errval = PK_WARN;
    }
    if (utime(d->fn, &d->u.t2)) {
        Info(slide, 0x201, ((char *)slide,
          LoadFarString(DirlistUtimeFailed), FnFilter1(d->fn)));
        if (!errval)
            errval = PK_WARN;
    }
#ifndef NO_CHMOD
    if (chmod(d->fn, 0xffff & d->perms)) {
        Info(slide, 0x201, ((char *)slide,
          LoadFarString(DirlistChmodFailed), FnFilter1(d->fn)));
        /* perror("chmod (file attributes) error"); */
        if (!errval)
            errval = PK_WARN;
    }
#endif /* !NO_CHMOD */
    return errval;
} /* end function set_directory_attributes() */

#endif /* SET_DIR_ATTRIB */




#ifdef TIMESTAMP

/***************************/
/*  Function stamp_file()  */
/***************************/

int stamp_file(fname, modtime)
    ZCONST char *fname;
    time_t modtime;
{
    ztimbuf tp;

    tp.modtime = tp.actime = modtime;
    return (utime(fname, &tp));

} /* end function stamp_file() */

#endif /* TIMESTAMP */




#ifndef SFX

/************************/
/*  Function version()  */
/************************/

void version(__G)
    __GDEF
{
#if (defined(__GNUC__) && defined(NX_CURRENT_COMPILER_RELEASE))
    char cc_namebuf[40];
    char cc_versbuf[40];
#else
#if (defined(CRAY) && defined(_RELEASE))
    char cc_versbuf[40];
#endif
#endif
#if ((defined(CRAY) || defined(cray)) && defined(_UNICOS))
    char os_namebuf[40];
#else
#if defined(__NetBSD__)
    char os_namebuf[40];
#endif
#endif

    /* Pyramid, NeXT have problems with huge macro expansion, too:  no Info() */
    sprintf((char *)slide, LoadFarString(CompiledWith),

#ifdef __GNUC__
#  ifdef NX_CURRENT_COMPILER_RELEASE
      (sprintf(cc_namebuf, "NeXT DevKit %d.%02d ",
        NX_CURRENT_COMPILER_RELEASE/100, NX_CURRENT_COMPILER_RELEASE%100),
       cc_namebuf),
      (strlen(__VERSION__) > 8)? "(gcc)" :
        (sprintf(cc_versbuf, "(gcc %s)", __VERSION__), cc_versbuf),
#  else
      "gcc ", __VERSION__,
#  endif
#else
#  if defined(CRAY) && defined(_RELEASE)
      "cc ", (sprintf(cc_versbuf, "version %d", _RELEASE), cc_versbuf),
#  else
#  ifdef __VERSION__
#   ifndef IZ_CC_NAME
#    define IZ_CC_NAME "cc "
#   endif
      IZ_CC_NAME, __VERSION__
#  else
#   ifndef IZ_CC_NAME
#    define IZ_CC_NAME "cc"
#   endif
      IZ_CC_NAME, "",
#  endif
#  endif
#endif /* ?__GNUC__ */

#ifndef IZ_OS_NAME
#  define IZ_OS_NAME "Unix"
#endif
      IZ_OS_NAME,

#if defined(sgi) || defined(__sgi)
      " (Silicon Graphics IRIX)",
#else
#ifdef sun
#  ifdef sparc
#    ifdef __SVR4
      " (Sun SPARC/Solaris)",
#    else /* may or may not be SunOS */
      " (Sun SPARC)",
#    endif
#  else
#  if defined(sun386) || defined(i386)
      " (Sun 386i)",
#  else
#  if defined(mc68020) || defined(__mc68020__)
      " (Sun 3)",
#  else /* mc68010 or mc68000:  Sun 2 or earlier */
      " (Sun 2)",
#  endif
#  endif
#  endif
#else
#ifdef __hpux
      " (HP/UX)",
#else
#ifdef __osf__
      " (DEC OSF/1)",
#else
#ifdef _AIX
      " (IBM AIX)",
#else
#ifdef aiws
      " (IBM RT/AIX)",
#else
#if defined(CRAY) || defined(cray)
#  ifdef _UNICOS
      (sprintf(os_namebuf, " (Cray UNICOS release %d)", _UNICOS), os_namebuf),
#  else
      " (Cray UNICOS)",
#  endif
#else
#if defined(uts) || defined(UTS)
      " (Amdahl UTS)",
#else
#ifdef NeXT
#  ifdef mc68000
      " (NeXTStep/black)",
#  else
      " (NeXTStep for Intel)",
#  endif
#else              /* the next dozen or so are somewhat order-dependent */
#ifdef LINUX
#  ifdef __ELF__
      " (Linux ELF)",
#  else
      " (Linux a.out)",
#  endif
#else
#ifdef MINIX
      " (Minix)",
#else
#ifdef M_UNIX
      " (SCO Unix)",
#else
#ifdef M_XENIX
      " (SCO Xenix)",
#else
#ifdef __NetBSD__
#  ifdef NetBSD0_8
      (sprintf(os_namebuf, " (NetBSD 0.8%c)", (char)(NetBSD0_8 - 1 + 'A')),
       os_namebuf),
#  else
#  ifdef NetBSD0_9
      (sprintf(os_namebuf, " (NetBSD 0.9%c)", (char)(NetBSD0_9 - 1 + 'A')),
       os_namebuf),
#  else
#  ifdef NetBSD1_0
      (sprintf(os_namebuf, " (NetBSD 1.0%c)", (char)(NetBSD1_0 - 1 + 'A')),
       os_namebuf),
#  else
      (BSD4_4 == 0.5)? " (NetBSD before 0.9)" : " (NetBSD 1.1 or later)",
#  endif
#  endif
#  endif
#else
#ifdef __FreeBSD__
      (BSD4_4 == 0.5)? " (FreeBSD 1.x)" : " (FreeBSD 2.0 or later)",
#else
#ifdef __bsdi__
      (BSD4_4 == 0.5)? " (BSD/386 1.0)" : " (BSD/386 1.1 or later)",
#else
#ifdef __386BSD__
      (BSD4_4 == 1)? " (386BSD, post-4.4 release)" : " (386BSD)",
#else
#if defined(i486) || defined(__i486) || defined(__i486__)
      " (Intel 486)",
#else
#if defined(i386) || defined(__i386) || defined(__i386__)
      " (Intel 386)",
#else
#ifdef pyr
      " (Pyramid)",
#else
#ifdef ultrix
#  ifdef mips
      " (DEC/MIPS)",
#  else
#  ifdef vax
      " (DEC/VAX)",
#  else /* __alpha? */
      " (DEC/Alpha)",
#  endif
#  endif
#else
#ifdef gould
      " (Gould)",
#else
#ifdef MTS
      " (MTS)",
#else
#ifdef __convexc__
      " (Convex)",
#else
#ifdef __QNX__
      " (QNX 4)",
#else
#ifdef __QNXNTO__
      " (QNX Neutrino)",
#else
#ifdef Lynx
      " (LynxOS)",
#else
      "",
#endif /* Lynx */
#endif /* QNX Neutrino */
#endif /* QNX 4 */
#endif /* Convex */
#endif /* MTS */
#endif /* Gould */
#endif /* DEC */
#endif /* Pyramid */
#endif /* 386 */
#endif /* 486 */
#endif /* 386BSD */
#endif /* BSDI BSD/386 */
#endif /* NetBSD */
#endif /* FreeBSD */
#endif /* SCO Xenix */
#endif /* SCO Unix */
#endif /* Minix */
#endif /* Linux */
#endif /* NeXT */
#endif /* Amdahl */
#endif /* Cray */
#endif /* RT/AIX */
#endif /* AIX */
#endif /* OSF/1 */
#endif /* HP/UX */
#endif /* Sun */
#endif /* SGI */

#ifdef __DATE__
      " on ", __DATE__
#else
      "", ""
#endif
    );

    (*G.message)((zvoid *)&G, slide, (ulg)strlen((char *)slide), 0);

} /* end function version() */

#endif /* !SFX */




#ifdef QLZIP

struct qdirect  {
    long            d_length __attribute__ ((packed));  /* file length */
    unsigned char   d_access __attribute__ ((packed));  /* file access type */
    unsigned char   d_type __attribute__ ((packed));    /* file type */
    long            d_datalen __attribute__ ((packed)); /* data length */
    long            d_reserved __attribute__ ((packed));/* Unused */
    short           d_szname __attribute__ ((packed));  /* size of name */
    char            d_name[36] __attribute__ ((packed));/* name area */
    long            d_update __attribute__ ((packed));  /* last update */
    long            d_refdate __attribute__ ((packed));
    long            d_backup __attribute__ ((packed));   /* EOD */
};

#define LONGID  "QDOS02"
#define EXTRALEN (sizeof(struct qdirect) + 8)
#define JBLONGID    "QZHD"
#define JBEXTRALEN  (sizeof(jbextra)  - 4 * sizeof(char))

typedef struct {
    char        eb_header[4] __attribute__ ((packed));  /* place_holder */
    char        longid[8] __attribute__ ((packed));
    struct      qdirect     header __attribute__ ((packed));
} qdosextra;

typedef struct {
    char        eb_header[4];                           /* place_holder */
    char        longid[4];
    struct      qdirect     header;
} jbextra;



/*  The following two functions SH() and LG() convert big-endian short
 *  and long numbers into native byte order.  They are some kind of
 *  counterpart to the generic UnZip's makeword() and makelong() functions.
 */
static ush SH(ush val)
{
    uch swapbuf[2];

    swapbuf[1] = (uch)(val & 0xff);
    swapbuf[0] = (uch)(val >> 8);
    return (*(ush *)swapbuf);
}



static ulg LG(ulg val)
{
    /*  convert the big-endian unsigned long number `val' to the machine
     *  dependant representation
     */
    ush swapbuf[2];

    swapbuf[1] = SH((ush)(val & 0xffff));
    swapbuf[0] = SH((ush)(val >> 16));
    return (*(ulg *)swapbuf);
}



static void qlfix(__G__ ef_ptr, ef_len)
    __GDEF
    uch *ef_ptr;
    unsigned ef_len;
{
    while (ef_len >= EB_HEADSIZE)
    {
        unsigned    eb_id  = makeword(EB_ID + ef_ptr);
        unsigned    eb_len = makeword(EB_LEN + ef_ptr);

        if (eb_len > (ef_len - EB_HEADSIZE)) {
            /* discovered some extra field inconsistency! */
            Trace((stderr,
              "qlfix: block length %u > rest ef_size %u\n", eb_len,
              ef_len - EB_HEADSIZE));
            break;
        }

        switch (eb_id) {
          case EF_QDOS:
          {
            struct _ntc_
            {
                long id;
                long dlen;
            } ntc;
            long dlen = 0;

            qdosextra   *extra = (qdosextra *)ef_ptr;
            jbextra     *jbp   = (jbextra   *)ef_ptr;

            if (!strncmp(extra->longid, LONGID, strlen(LONGID)))
            {
                if (eb_len != EXTRALEN)
                    if (uO.qflag)
                        Info(slide, 0x201, ((char *)slide,
                          "warning:  invalid length in Qdos field for %s\n",
                          FnFilter1(G.filename)));
                    else
                        Info(slide, 0x201, ((char *)slide,
                          "warning:  invalid length in Qdos field"));

                if (extra->header.d_type)
                {
                    dlen = extra->header.d_datalen;
                }
            }

            if (!strncmp(jbp->longid, JBLONGID, strlen(JBLONGID)))
            {
                if (eb_len != JBEXTRALEN)
                    if (uO.qflag)
                        Info(slide, 0x201, ((char *)slide,
                          "warning:  invalid length in QZ field for %s\n",
                          FnFilter1(G.filename)));
                    else
                        Info(slide, 0x201, ((char *)slide,
                          "warning:  invalid length in QZ field"));
                if(jbp->header.d_type)
                {
                    dlen = jbp->header.d_datalen;
                }
            }

            if ((long)LG(dlen) > 0)
            {
                G.outfile = fopen(G.filename,"r+");
                fseek(G.outfile, -8, SEEK_END);
                fread(&ntc, 8, 1, G.outfile);
                if(ntc.id != *(long *)"XTcc")
                {
                    ntc.id = *(long *)"XTcc";
                    ntc.dlen = dlen;
                    fwrite (&ntc, 8, 1, G.outfile);
                }
                Info(slide, 0x201, ((char *)slide, "QData = %d", LG(dlen)));
                fclose(G.outfile);
            }
            return;     /* finished, cancel further extra field scanning */
          }

          default:
            Trace((stderr,"qlfix: unknown extra field block, ID=%d\n",
               eb_id));
        }

        /* Skip this extra field block */
        ef_ptr += (eb_len + EB_HEADSIZE);
        ef_len -= (eb_len + EB_HEADSIZE);
    }
}
#endif /* QLZIP */
