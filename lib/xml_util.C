// $Id$
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
#include "config.h"
#include <cctype>
#include <vector>
#include <string>
#include <sstream>
#include "xml_util.h"

int xml_indent_level=0;

std::string xml_indent(int i) {
  if (i) xml_indent_level+=i;
  xml_indent_level = (xml_indent_level>0) ? xml_indent_level : 0;
  return std::string(xml_indent_level,' ');
}

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

#if 0
xml_ofstream::xml_ofstream() : my_tag(), os()  {}

xml_ofstream::xml_ofstream(const char *filename, const char *tag, 
    std::ios_base::openmode m) : , my_tag(tag), os(filename,m)
{
  if (is_open()) {
    write_head();
  }
}

xml_ostream::xml_ostream(std::ostream &o, const char *tag) 
  : my_tag(tag), os(o) 
{
  write_head();
}

xml_ostream::~xml_ostream() {
  write_foot();
}

xml_ofstream::~xml_ofstream() {
  close();
}

void xml_ofstream::open(const char *filename, const char *tag, 
    std::ios_base::openmode m) {
  my_tag=std::string(tag);
  os.open(filename,m);
  if (is_open()) {
    write_head();
  }
}

void xml_ofstream::close() {
  write_foot();
  os.close();
}

void xml_ostream::write_head() {
  xml_indent_level=0;
  os << xml_header << std::endl;
  os << '<' << my_tag << '>' << std::endl;
  xml_indent(2);
}

void xml_ofstream::write_head() {
  xml_indent_level=0;
  os << xml_header << std::endl;
  os << '<' << my_tag << '>' << std::endl;
  xml_indent(2);
}

void xml_ostream::write_foot() {
  xml_indent(-2);
  os << "</" << my_tag << '>' << std::endl;
}

void xml_ofstream::write_foot() {
  xml_indent(-2);
  os << "</" << my_tag << '>' << std::endl;
}

xml_ifstream::xml_ifstream() : , my_tag(""), xml_start(0), ifs()
  xml_end(0) {}

xml_ifstream::xml_ifstream(const char *filename, const char *tag, 
    std::ios_base::openmode m) : std::ifstream(filename,m), my_tag(tag),
    xml_start(0), xml_end(0) {
  if (is_open()) {
    seek_head();
  }
}

xml_istream::xml_istream(std::istream &i, const char *tag) 
  : my_tag(tag), is(i) {
}

xml_ifstream::~xml_ifstream() {
  close();
}

void xml_ifstream::open(const char *filename, const char *tag, 
    std::ios_base::openmode m) {
  my_tag=std::string(tag);
  std::ifstream::open(filename,m);
  if (is_open()) {
    seek_head();
  }
}

void xml_istream::seek_head() {
  std::string tmp;
  char c;
  unsigned int i=0;
  bool start_found=false;
  if (my_tag.size()) {
    while (is) {
        is.get(c);
	if (c=='<') {
	  do {
	    is.get(c);
	    i++;
	  } while (c == my_tag[i-1]);
	  if ((i==my_tag.size()) && !isalnum(c)) {
	    start_found=true;
	    break;
	  }
	}
    }
  } else {
    while (is) {
      is.get(c);
      if (c=='<') {
	do {
	  is.get(c);
          if (isalnum(c)) my_tag+=c;
	} while (isalnum(c));
      }
      if (my_tag.size()) {
	start_found=true;
	break;
      }
    }
  }
  if (start_found) {
    while ((c != '>') && is) is.get(c);
  }
}


void xml_ifstream::seek_head() {
  if (!xml_start) {
    std::string tmp;
    std::string::size_type tag_start, tag_end;
    bool start_found=false;
    std::ifstream::seekg(0,std::ios::beg);
    if (my_tag.size()) {
      do {
        *this >> tmp;
        if ((tag_start=tmp.find(std::string("<")+my_tag)) != std::string::npos) {
	  tag_start=tmp.find('>');
	  std::ifstream::seekg(tag_start-tmp.size()+my_tag.size()+2,std::ios::cur);
	  start_found=true;
        } else {
          if ((tag_start=tmp.find("<")) != std::string::npos) {
	    if (isalpha(tmp[tag_start+1])) {
              while (isalnum(tmp[++tag_start])) my_tag+=tmp[tag_start];
	      start_found=true;
	      tag_start=tmp.find(">",tag_start-1);
	      std::ifstream::seekg(tag_start-tmp.size(),std::ios::cur);
	    }
          }
        }
      } while (!start_found && !std::ifstream::eof());
      xml_start=std::ifstream::tellg();
    }
    if (my_tag.size()) {
      int nstarts=1;
      std::string start_tag(std::string("<")+my_tag);
      std::string end_tag(std::string("</")+my_tag);
      do {
	*this >> tmp;
	if (tmp.find(start_tag)!=std::string::npos) {
	  nstarts++;
	}
	if ((tag_end=tmp.find(end_tag))!=std::string::npos) {
	  nstarts--;
	}
      } while (nstarts && !std::ifstream::eof());
      std::ifstream::seekg(tag_end-tmp.size(),std::ios::cur);
      xml_end=std::ifstream::tellg();
    }
  } 
  if (xml_start) std::ifstream::seekg(xml_start,std::ios::beg);
}

xml_ifstream &xml_ifstream::seekg(pos_type p) {
  if (xml_start) std::ifstream::seekg(xml_start+p);
  return *this;
}

xml_ifstream &xml_ifstream::seekg(off_type o, std::ios::seekdir d) {
  switch (d) {
    case std::ios::beg:
      seekg(o);
      break;
    case std::ios::end:
      std::ifstream::seekg(xml_end+o);
      break;
    default:
      std::ifstream::seekg(o,d);
      break;
  }
  return *this;
}

std::ios::pos_type xml_ifstream::tellg() {
  return std::ifstream::tellg()-xml_start;
}
	
bool xml_ifstream::eof() {
  if (std::ifstream::tellg() >= xml_end) {
    return true;
  } else {
    return std::ifstream::tellg();
  }
}
#endif  // 0

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

const char * encode_arr="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char * encode_arr85=
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy!#$()*+,-./:;=?@^`{|}~z_";


bool isencchar(char c) {
  bool rv=((c>='A') && (c<='Z'));
  rv|=((c>='a') && (c<='z'));
  rv|=((c>='0') && (c<='9'));
  rv|=((c=='+') || (c=='/') || (c=='='));
  return rv;
}

bool isencchar85(char c) {
  bool rv=((c>='A') && (c<='Z'));
  rv|=((c>='a') && (c<='z'));
  rv|=((c>='0') && (c<='9'));
  switch (c) {
    case '!':
    case '#':
    case '$':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case ';':
    case '=':
    case '?':
    case '@':
    case '^':
    case '`':
    case '{':
    case '|':
    case '}':
    case '~':
    case '_':
      rv=true;
      break;
    default:
      break;
  }
  return rv;
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

unsigned char decode_char(const char *s) {
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

std::string x_csv_encode_char(const unsigned char *bin, size_t nelements) {
  std::ostringstream rv("");
  size_t lastlen=0,i;
  rv << std::endl << xml_indent(2);
  for (i=0;i<(nelements-1);i++) {
    unsigned int ival=bin[i];
    rv << ival << ',';
    if ((rv.str().size()-lastlen-xml_indent_level)>73) {
      rv << std::endl << xml_indent();
      lastlen=rv.str().size();
    }
  }
  unsigned int ival=bin[i];
  rv << ival << std::endl << xml_indent(-2);
  return rv.str();
}

//
// $Log$
// Revision 1.16  2003/10/23 19:58:20  jeffc
// jeffc - bug fix in csv encode routine
//
// Revision 1.15  2003/10/23 19:18:38  jeffc
// jeffc - put back in line feeds - no longer using parese_str().
//
// Revision 1.14  2003/10/23 15:39:54  korpela
// no message
//
// Revision 1.13  2003/10/23 00:25:15  jeffc
// jeffc - no line feeds in CSV encoding
//
// Revision 1.12  2003/10/22 18:23:23  korpela
// *** empty log message ***
//
// Revision 1.11  2003/10/22 18:01:41  korpela
// *** empty log message ***
//
// Revision 1.10  2003/10/22 15:24:10  korpela
// *** empty log message ***
//
// Revision 1.9  2003/10/21 18:14:36  korpela
// *** empty log message ***
//
//





