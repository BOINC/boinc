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
// Revision 1.3  2004/07/13 05:56:02  rwalton
// Hooked up the Project and Work tab for the new GUI.
//
// Revision 1.2  2004/07/12 08:46:26  rwalton
// Document parsing of the <get_state/> message
//
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
#include "error_numbers.h"


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

bool CXMLParser::parse_int(const wxString &strBuffer, const wxString &strTag, wxInt32 &lValue)
{
    wxString    strTemp;
    
    if (!parse_str(strBuffer, strTag, strTemp))
        return false;

    return strTemp.ToLong((long*)&lValue);
}


bool CXMLParser::parse_double(const wxString &strBuffer, const wxString &strTag, double &dValue)
{
    wxString    strTemp;
    
    if (!parse_str(strBuffer, strTag, strTemp))
        return false;

    return strTemp.ToDouble(&dValue);
}


bool CXMLParser::parse_str(const wxString &strBuffer, const wxString &strTag, wxString &strValue)
{
    wxInt32     iStart = strBuffer.First(strTag);
    wxInt32     iEnd = strBuffer.Find('<', true);

    if (-1 == iStart)
        return false;

    wxASSERT(-1 != iEnd);

    iStart += (wxInt32)strTag.Length();

    if ((0  != iStart) && (-1 != iEnd)) {
        xml_unescape(strBuffer.Mid(iStart, (iEnd - iStart)), strValue);
        return true;
    } else {
        return false;
    }
}


int CXMLParser::copy_element_contents(wxTextInputStream* input, const wxString &strTag, wxString &strValue)
{
    wxString buf;
    while (buf = input->ReadLine()) {
        if (match_tag(buf, strTag)) return 0;
        strValue += buf;
    }
    return ERR_XML_PARSE;
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

