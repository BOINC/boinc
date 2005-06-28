// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
        const char* z = y; \
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
    int do_query(const char*);
    void close();
    int insert_id();
    void print_error(const char*);
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
    DB_BASE(const char *table_name, DB_CONN*);
    int insert();
    int insert_batch(std::string&);
    int update();
    int update_field(const char*);
    int delete_from_db();
    int get_field_int(const char*, int&);
    int lookup_id(int id);
    int lookup(const char*);
    int enumerate(const char* clause="", bool use_use_result=false);
    int end_enumerate();
    int count(int&, const char* clause="");
    int max_id(int&, const char* clause="");
    int sum(double&, const char* field, const char* clause="");
    int get_double(const char* query, double&);
    int get_integer(const char* query, int&);
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
void escape_mysql_like_pattern(const char* in, char* out);
    // if you're going to use a "like X" clause,
    // call this function to escape the non-wildcard part of X.
    // If it contains wildcard chars (%, _) this will put
    // two (2) underscores before each one,
    // so that they don't get treated as wildcards

#endif
