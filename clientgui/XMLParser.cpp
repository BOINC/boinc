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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "XMLParser.h"
#endif

#include "stdwx.h"
#include "XMLParser.h"


IMPLEMENT_DYNAMIC_CLASS(CXMLParser, wxObject)


CXMLParser::CXMLParser()
{
}


CXMLParser::~CXMLParser()
{
}


bool CXMLParser::match_tag(const wxString &strBuffer, const wxString &strTag)
{
    if (strBuffer.Contains(strTag))
        return true;
    return false;
}

bool CXMLParser::parse_int(const wxString &strBuffer, const wxString &strTag, long &lValue)
{
    wxString    strTemp;
    
    if (!parse_string(strBuffer, strTag, strTemp))
        return false;

    return strTemp.ToLong(&lValue);
}


bool CXMLParser::parse_double(const wxString &strBuffer, const wxString &strTag, double &dValue)
{
    wxString    strTemp;
    
    if (!parse_string(strBuffer, strTag, strTemp))
        return false;

    return strTemp.ToDouble(&dValue);
}


bool CXMLParser::parse_string(const wxString &strBuffer, const wxString &strTag, wxString &strValue)
{
    wxInt32     iStart = strBuffer.First(strTag) + (wxInt32)strTag.Length();
    wxInt32     iEnd = strValue.Find('<', true);

    wxASSERT(0  != iStart);
    wxASSERT(-1 != iEnd);

    if ((0  != iStart) && (-1 != iEnd)) {
        xml_unescape(strBuffer.Mid(iStart, (iStart - iEnd)), strValue);
        return true;
    } else {
        return false;
    }
}


void CXMLParser::xml_escape(const wxString& in, wxString& out) {
    int i;
	out = _T("");
	for (i=0; i<(int)in.length(); i++) {
        if (in.GetChar(i) == '<') {
			out += "&lt;";
        } else if (in.GetChar(i) == '&') {
			out += "&amp;";
		} else {
            out += in.GetChar(i);
		}
	}
}

void CXMLParser::xml_unescape(const wxString& in, wxString& out) {
	 int i;
	 out = _T("");
	 for (i=0; i<(int)in.length(); i++) {
		 if (in.substr(i, 4) == "&lt;") {
			 out += "<";
			 i += 3;
		 } else if (in.substr(i, 5) == "&amp;") {
			 out += "&";
			 i += 4;
		 } else {
             out += in.GetChar(i);
		 }
	 }
}

