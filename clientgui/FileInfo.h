// $Id$
//
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//
// Revision History:
//
// $Log$
// Revision 1.1  2004/06/25 22:50:57  rwalton
// Client spamming server hotfix
//
//
//

#ifndef _FILEINFO_H_
#define _FILEINFO_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "FileInfo.cpp"
#endif

#include "XMLParser.h"
#include "Project.h"


class CFileInfo : public CXMLParser
{
    DECLARE_DYNAMIC_CLASS(CFileInfo)

private:
    wxString        name;
    bool            generated_locally;
    bool            uploaded;
    bool            upload_when_present;
    bool            sticky;
    bool            pers_xfer_active;
    bool            xfer_active;
    wxInt32         num_retries;
    double          bytes_xferred;
    double          file_offset;
    double          xfer_speed;
    wxString        hostname;
    CProject*       project;

public:
    CFileInfo();
    ~CFileInfo();

    wxInt32         Parse(wxTextInputStream* input);

    wxString        GetName()                       { return name; }
    bool            IsGeneratedLocally()            { return generated_locally; }
    bool            IsUploaded()                    { return uploaded; }
    bool            IsUploadWhenPresent()           { return upload_when_present; }
    bool            IsSticky()                      { return sticky; }
    bool            IsPersXferActive()              { return pers_xfer_active; }
    bool            IsXferActive()                  { return xfer_active; }
    wxInt32         GetNumberOfRetries()            { return num_retries; }
    double          GetBytesXfered()                { return bytes_xferred; }
    double          GetFileOffset()                 { return file_offset; }
    double          GetXferSpeed()                  { return xfer_speed; }
    wxString        GetHostname()                   { return hostname; }
    CProject*       GetProject()                    { return project; }

};


#endif

