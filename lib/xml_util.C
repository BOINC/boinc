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

#include <cctype>
#include <vector>
#include <string>
#include "xml_util.h"

// Most of these entries are for reverse translation of poorly written HTML.
// Forward translation doesn't translate most printable characters.
const xml_entity xml_trans[]= {
  { 0x07, "&bel;" },
  { 0x0a, "&lf;" },
  { 0x0d, "&cr;" },
  { ' ', "&sp;" }, 
  { '!', "&excl;" },
  { '\"', "&quot;" },
  { '\"', "&dquot;" },
  { '#', "&num;" },
  { '$', "&dollar;" },
  { '%', "&percnt;" },
  { '&', "&amp;" },
  { '\'', "&apos;" },
  { '(', "&lpar;" },
  { ')', "&rpar;" },
  { '*', "&ast;" },
  { '+', "&plus;" },
  { ',', "&comma;" },
  { '-', "&hyphen;" },
  { '-', "&minus;" },
  { '.', "&period;" },
  { '/', "&sol;" },
  { ':', "&colon;" },
  { ';', "&semi;" },
  { '<', "&lt;" },
  { '=', "&equals;" },
  { '>', "&gt;" },
  { '?', "&quest;" },
  { '@', "&commat;" },
  { '[', "&lsqb;" },
  { '\\', "&bsol;" },
  { ']', "&rsqb;" },
  { '^', "&circ;" },
  { '_', "&lowbar;" },
  { '_', "&horbar;" },
  { '`', "&grave;" },
  { '{', "&lcub;" },
  { '|', "&verbar;" },
  { '}', "&rcub;" },
  { '~', "&tilde;" },
  { 0x82, "&lsquor;" }, 
  { 0x84, "&ldquor;" }, 
  { 0x85, "&ldots;" }, 
  { 0x8a, "&Scaron;" }, 
  { 0x8b, "&lsaquo;" }, 
  { 0x8c, "&OElig;" }, 
  { 0x91, "&lsquo;" }, 
  { 0x91, "&rsquor;" }, 
  { 0x92, "&rsquo;" }, 
  { 0x93, "&ldquo;" },
  { 0x93, "&rdquor;" },
  { 0x94, "&rdquo;" },
  { 0x95, "&bull;" },
  { 0x96, "&ndash;" },
  { 0x96, "&endash;" },
  { 0x97, "&mdash;" },
  { 0x97, "&emdash;" },
  { 0xa0, "&nbsp;" },
  { 0xa1, "&iexcl;" },
  { 0xa2, "&cent;" },
  { 0xa3, "&pound;" },
  { 0xa4, "&curren;" },
  { 0xa5, "&yen;" },
  { 0xa6, "&brvbar;" },
  { 0xa7, "&sect;" },
  { 0xa8, "&uml;" },
  { 0xa9, "&copy;" },
  { 0xaa, "&ordf;" },
  { 0xab, "&laquo;" },
  { 0xac, "&not;" },
  { 0xad, "&shy;" },
  { 0xae, "&reg;" },
  { 0xaf, "&macr;" },
  { 0xb0, "&deg;" },
  { 0xb1, "&plusmn;" },
  { 0xb2, "&sup2;" },
  { 0xb3, "&sup3;" },
  { 0xb4, "&acute;" },
  { 0xb5, "&micro;" },
  { 0xb6, "&para;" },
  { 0xb7, "&middot;" },
  { 0xb8, "&cedil;" },
  { 0xb9, "&sup1;" },
  { 0xba, "&ordm;" },
  { 0xbb, "&raquo;" },
  { 0xbc, "&frac14;" },
  { 0xbd, "&frac12;" },
  { 0xbe, "&frac34;" },
  { 0xbf, "&iquest;" },
  { 0xc0, "&Agrave;" },
  { 0xc1, "&Aacute;" },
  { 0xc2, "&Acirc;" },
  { 0xc3, "&Atilde;" },
  { 0xc4, "&Auml;" },
  { 0xc5, "&Aring;" },
  { 0xc6, "&AElig;" },
  { 0xc7, "&Ccedil;" },
  { 0xc8, "&Egrave;" },
  { 0xc9, "&Eacute;" },
  { 0xca, "&Ecirc;" },
  { 0xcb, "&Euml;" },
  { 0xcc, "&Igrave;" },
  { 0xcd, "&Iacute;" },
  { 0xce, "&Icirc;" },
  { 0xcf, "&Iuml;" },
  { 0xd0, "&ETH;" },
  { 0xd1, "&Ntilde;" },
  { 0xd2, "&Ograve;" },
  { 0xd3, "&Oacute;" },
  { 0xd4, "&Ocirc;" },
  { 0xd5, "&Otilde;" },
  { 0xd6, "&Ouml;" },
  { 0xd7, "&times;" },
  { 0xd8, "&Oslash;" },
  { 0xd9, "&Ugrave;" },
  { 0xda, "&Uacute;" },
  { 0xdb, "&Ucirc;" },
  { 0xdc, "&Uuml;" },
  { 0xdd, "&Yacute;" },
  { 0xde, "&THORN;" },
  { 0xdf, "&szlig;" },
  { 0xe0, "&agrave;" },
  { 0xe1, "&aacute;" },
  { 0xe2, "&acirc;" },
  { 0xe3, "&atilde;" },
  { 0xe4, "&auml;" },
  { 0xe5, "&aring;" },
  { 0xe6, "&aelig;" },
  { 0xe7, "&ccedil;" },
  { 0xe8, "&egrave;" },
  { 0xe9, "&eacute;" },
  { 0xea, "&ecirc;" },
  { 0xeb, "&euml;" },
  { 0xec, "&igrave;" },
  { 0xed, "&iacute;" },
  { 0xee, "&icirc;" },
  { 0xef, "&iuml;" },
  { 0xf0, "&eth;" },
  { 0xf1, "&ntilde;" },
  { 0xf2, "&ograve;" },
  { 0xf3, "&oacute;" },
  { 0xf4, "&ocirc;" },
  { 0xf5, "&otilde;" },
  { 0xf6, "&ouml;" },
  { 0xf7, "&divide;" },
  { 0xf8, "&oslash;" },
  { 0xf9, "&ugrave;" },
  { 0xfa, "&uacute;" },
  { 0xfb, "&ucirc;" },
  { 0xfc, "&uuml;" },
  { 0xfd, "&yacute;" },
  { 0xfe, "&thorn;" },
  { 0xff, "&yuml;" },
  { 0x00, 0 }
}; 

#ifdef HAVE_MAP
#include <map>


std::multimap<unsigned char,const char *> encode_map;
std::map<std::string, unsigned char> decode_map;

void populate_encode_map() {
  int i=0;
  do {
    encode_map.insert(std::make_pair(xml_trans[i].c,xml_trans[i].s));
  } while (xml_trans[++i].s);
}

void populate_decode_map() {
  int i=0;
  do {
    decode_map[xml_trans[i].s]=xml_trans[i].c;
  } while (xml_trans[++i].s);
}
#endif

static int xml_indent_level=0;

std::string xml_indent(int i) {
  if (i) xml_indent_level+=i;
  xml_indent_level=std::max(xml_indent_level,0);
  return std::string(xml_indent_level,' ');
}

std::string encode_char(unsigned char c) {
#ifdef HAVE_MAP
  if (!(encode_map.size())) populate_encode_map();
  std::multimap<unsigned char,const char *>::iterator p=encode_map.find(c);
  if (p!=encode_map.end()) {
    return (p->second);
  } else {
#else
  int i=0;
  while (xml_trans[i].s) {
    if (xml_trans[i].c == c) return std::string(xml_trans[i].s);
    i++;
  }
  {
#endif
    char buf[16];
    sprintf(buf,"&#%.3d;",static_cast<int>(c));
#ifdef HAVE_MAP
    encode_map.insert(std::make_pair(c,buf));
#endif
    return std::string(buf);
  }
}  

unsigned char decode_char(const unsigned char *s) {
  char code[32];
  int i=0;
  while (*s && (*s != ';')) {
    code[i]=*s;
    s++;
    i++;
  }
  code[i]=';';
  code[i+1]=0;
#ifdef HAVE_MAP
  if (!(decode_map.size())) populate_decode_map();
  std::map<std::string,unsigned char>::iterator p=decode_map.find(code);
  if (p!=decode_map.end()) {
    return (p->second);
  } else {
#else
  while (xml_trans[i].s) {
    if (!strcmp(xml_trans[i].s,(const char *)(&code[0]))) return xml_trans[i].c;
    i++;
  }
  {
#endif
    if (code[1]=='#') {
      sscanf((const char *)(code+2),"%d",&i);
#ifdef HAVE_MAP
      decode_map.insert(std::make_pair(code,static_cast<unsigned char>(i&0xff)));
#endif
    } else {
      fprintf(stderr,"Unknown XML entity \"%s\"\n",code);
      i='&';
    }
    return static_cast<unsigned char>(i&0xff);
  }
}  

std::vector<unsigned char> xml_decode_string(const unsigned char *input, size_t length) {
  unsigned int i;
  char c;
  if (!length) {
    // We're going to decode until we see a null.  Including the null.
    length=strlen((const char *)input);
  }
  std::vector<unsigned char> rv;
  unsigned char *p;
  rv.reserve(length);
  for (i=0; i<length; i++) {
    if (input[i]=='&') {
      rv.push_back(c=decode_char(input+i));
      if ((c!='&') || strncmp((const char *)(input+i),"&amp;",5)) {
	p=(unsigned char *)strchr((const char *)(input+i),';');
	i=(p-input);
      }
    } else {
      rv.push_back(input[i]);
    }
  }
  return rv;
}

std::string xml_encode_string(const unsigned char *input, size_t length) {
  unsigned int i;
  if (!length) {
    // This is bad form.  Are you sure there are no nulls in the input?
    length=strlen((const char *)input);
  }
  std::string rv;
  rv.reserve(length);
  for (i=0; i<length; i++) {
    if (isprint(input[i])) {
      switch (input[i]) {
	case '>':
	case '<':
	case '&':
	case '\'':
	case '"': 
	  rv+=encode_char(input[i]);
	  break;
        default:
	  rv+=input[i];
       }
    } else {
      char buf[16];
      sprintf(buf,"&#%.3d;",input[i]);
      rv+=buf;
    }
  }
  return rv;
}


