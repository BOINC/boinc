// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include "mysql.h"

struct ENUM {
    int active;
    MYSQL_RES *rp;
};

extern int db_open(char* dbname, char* password);
extern int db_close();
extern void db_print_error();

extern void escape(char*);
extern void unescape(char*);

extern int db_new(void*, int);
extern int db_insert_id();
extern int db_delete(int, int);
extern int db_lookup_id(int, void*, int);
extern int db_lookup(void*, int, char*);
extern int db_update(void*, int);
extern int db_enum(ENUM&, void*, int, char* clause=0, int limit=0);
extern int db_enum_field(ENUM&, int, char*, char*);
extern int db_query_int(int*, char*);
extern int db_count(int*, char*, int, char*);
