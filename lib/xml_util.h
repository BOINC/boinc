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
//
//  Additional routines to help maintain XML compliance.

#ifndef _XML_UTIL_H_
#define _XML_UTIL_H_

// XML entity for tranlation table (not wchar_t compatible) 
struct xml_entity {
  unsigned char c;
  const char *s;
}; 

// change the xml indent level (number of spaces) by adding or subtracting
// "i" spaces.  return a string of spaces corresponding to the current xml 
// indent level.
std::string xml_indent(int i=0);

// decode an XML character string.  Return a the decoded string in a vector
// (null not necessarily a terminator).
std::vector<unsigned char> xml_decode_string(const unsigned char *input, size_t length=0);

// encode an XML character string.  Return the encoded string.
std::string xml_encode_string(const unsigned char *input, size_t length=0);

#endif
