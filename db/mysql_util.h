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

extern void escape_single_quotes(char*);
extern void unescape_single_quotes(char*);

class MYSQL_DB {
public:
    char** table_name;
    MYSQL* mp;
    MYSQL_RES* rp;
    MYSQL_ROW row;
    virtual void struct_to_str(void*, char*, int){}
    virtual void row_to_struct(MYSQL_ROW&, void*, int){}

    MYSQL_DB(){}
    int db_open(char* dbname, char* password);
    int db_close();
    const char* db_error_string();
    void db_print_error(char*);
    int db_new(void*, int);
    int db_insert_id();
    int db_delete(int, int);
    int db_lookup_id(int, void*, int);
    int db_lookup(void*, int, char*);
    int db_update(void*, int);
    int db_enum(ENUM&, void*, int, char* clause=0, int limit=0);
    int db_enum_field(ENUM&, int, char*, char*);
    int db_query_int(int*, char*);
    int db_count(int*, char*, int, char* clause=0);
    int db_query(char*);
};
