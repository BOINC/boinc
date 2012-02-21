// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
//
// Source Code Originally from:
// http://www.codeguru.com/cpp/w-p/files/comments.php/c4479/?thread=52622
//

#ifndef _BOINC_DIROPS_
#define _BOINC_DIROPS_

// Prototypes
BOOL RemoveReadOnly(tstring& csFileName);
BOOL RecursiveDeleteFolder(tstring& csPath);
BOOL RecursiveCopyFolder(tstring& csFromPath, tstring& csToPath);
BOOL MoveFolder(tstring& csPath, tstring& csNewPath);
BOOL RecursiveSetPermissions(tstring& csPath, PACL pACL);

#endif

