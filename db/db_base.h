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

#ifndef _DB_BASE_
#define _DB_BASE_

#include "mysql.h"

// if SQL columns are not 'not null', you must use these safe_atoi, safe_atof
// instead of atoi, atof, since the strings returned by MySQL may be NULL.
//

inline int safe_atoi(const char* s) {
    if (!s) return 0;
    return atoi(s);
}

inline float safe_atof(const char* s) {
    if (!s) return 0;
    return atof(s);
}

#define strcpy2(x, y) \
    { \
        char* z = y; \
        if (!z) { \
            x[0]=0; \
        } else { \
            strlcpy(x, z, sizeof(x)); \
        } \
    }

#define MAX_QUERY_LEN 256000
    // TODO: use string for queries, get rid of this

struct CURSOR {
    bool active;
    MYSQL_RES *rp;
    CURSOR() { active = false; rp = NULL; }
};

// represents a connection to a database
//
class DB_CONN {
public:
    DB_CONN();
    int open(char* name, char* host, char* user, char* passwd);
    int do_query(char*);
    void close();
    int insert_id();
    void print_error(char*);
    const char* error_string();

    MYSQL* mysql;
    int start_transaction();
    int commit_transaction();
};

// Base for derived classes that can access the DB
// Defines various generic operations on DB tables
//
class DB_BASE {
public:
    DB_BASE(char *table_name, DB_CONN*);
    int insert();
    int insert_batch(std::string&);
    int update();
    int update_field(char*);
    int delete_from_db();
    int get_field_int(char*, int&);
    int lookup_id(int id);
    int lookup(char*);
    int enumerate(char* clause="", bool use_use_result=false);
    int end_enumerate();
    int count(int&, char* clause="");
    int max_id(int&, char* clause="");
    int sum(double&, char* field, char* clause="");
    int get_double(char* query, double&);
    int get_integer(char* query, int&);
    bool is_high_priority;

    DB_CONN* db;
    const char *table_name;
    CURSOR cursor;
    virtual int get_id();
    virtual void db_print(char*);
    virtual void db_parse(MYSQL_ROW&);
};

// Base for derived classes that can get special-purpose data,
// perhaps spanning multiple tables
//
class DB_BASE_SPECIAL {
public:
    DB_BASE_SPECIAL(DB_CONN*);

    DB_CONN* db;
    CURSOR cursor;
};

void escape_string(char* field, int len);
void unescape_string(char* p, int len);

#endif
