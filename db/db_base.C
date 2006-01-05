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

#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "error_numbers.h"
#include "util.h"
#include "db_base.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

// uncomment the following to print all queries.
// Useful for low-level debugging

//#define SHOW_QUERIES

DB_CONN::DB_CONN() {
    mysql = 0;
}

int DB_CONN::open(char* db_name, char* db_host, char* db_user, char* dbpassword) {
    mysql = mysql_init(0);
    if (!mysql) return ERR_DB_CANT_INIT;
    mysql = mysql_real_connect(mysql, db_host, db_user, dbpassword, db_name, 0, 0, 0);
    if (mysql == 0) return ERR_DB_CANT_CONNECT;
    return 0;
}

void DB_CONN::close() {
    if (mysql) mysql_close(mysql);
}

int DB_CONN::do_query(const char* p) {
    int retval;
#ifdef SHOW_QUERIES
    fprintf(stderr, "query: %s\n", p);
#endif
    retval = mysql_query(mysql, p);
    if (retval) {
        fprintf(stderr, "Database error: %s\nquery=%s\n", error_string(), p);
    }
    return retval;
}

int DB_CONN::insert_id() {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    retval = do_query("select LAST_INSERT_ID()");
    if (retval) return retval;
    rp = mysql_store_result(mysql);
    row = mysql_fetch_row(rp);
    mysql_free_result(rp);
    return atoi(row[0]);
}

void DB_CONN::print_error(const char* p) {
    fprintf(stderr, "%s: Database error: %s\n", p, error_string());
}

const char* DB_CONN::error_string() {
    return mysql?mysql_error(mysql):"Not connected";
}

int DB_CONN::start_transaction() {
    return do_query("START TRANSACTION");
}

int DB_CONN::commit_transaction() {
    return do_query("COMMIT");
}

DB_BASE::DB_BASE(const char *tn, DB_CONN* p) : db(p), table_name(tn) {
    is_high_priority = false;
}

int DB_BASE::get_id() { return 0;}
void DB_BASE::db_print(char*) {}

void DB_BASE::db_parse(MYSQL_ROW&) {}

int DB_BASE::insert() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "insert into %s set %s", table_name, vals);
    return db->do_query(query);
}

int DB_BASE::insert_batch(std::string& values) {
    std::string query;
    query = "insert into " + std::string(table_name) + " values " + values;
    return db->do_query(query.c_str());
}

// update an entire record
//
int DB_BASE::update() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "update %s set %s where id=%d", table_name, vals, get_id());
    return db->do_query(query);
}

// update one or more fields
// "clause" is something like "foo=5, blah='xxx'" or "foo=foo+5"
//
int DB_BASE::update_field(const char* clause) {
    char query[MAX_QUERY_LEN];
    sprintf(query, "update %s set %s where id=%d", table_name, clause, get_id());
    return db->do_query(query);
}

// delete record
//
int DB_BASE::delete_from_db() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "delete from %s where id=%d", table_name, get_id());
    return db->do_query(query);
}

int DB_BASE::get_field_int(const char* field, int& val) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query, "select %s from %s where id=%d", field, table_name, get_id());
    retval = db->do_query(query);
    if (retval) return retval;
    rp = mysql_store_result(db->mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) val = atoi(row[0]);
    mysql_free_result(rp);
    if (row == 0) return ERR_DB_NOT_FOUND;
    return 0;
}

int DB_BASE::lookup(const char* clause) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    if (is_high_priority) {
        sprintf(query, "select HIGH_PRIORITY * from %s %s", table_name, clause);
    } else {
        sprintf(query, "select * from %s %s", table_name, clause);
    }

    retval = db->do_query(query);
    if (retval) return retval;
    rp = mysql_store_result(db->mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) db_parse(row);
    mysql_free_result(rp);
    if (row == 0) return ERR_DB_NOT_FOUND;
    return 0;
}

int DB_BASE::lookup_id(int id) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    if (is_high_priority) {
        sprintf(query, "select HIGH_PRIORITY * from %s where id=%d", table_name, id);
    } else {
        sprintf(query, "select * from %s where id=%d", table_name, id);
    }

    retval = db->do_query(query);
    if (retval) return retval;
    rp = mysql_store_result(db->mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) db_parse(row);
    mysql_free_result(rp);
    if (row == 0) return ERR_DB_NOT_FOUND;

    // don't bother checking for uniqueness here
    return 0;
}

int DB_BASE::enumerate(const char* clause, bool use_use_result) {
    int x;
    char query[MAX_QUERY_LEN];
    MYSQL_ROW row;

    if (!cursor.active) {
        cursor.active = true;

        if (is_high_priority) {
            sprintf(query, "select HIGH_PRIORITY * from %s %s", table_name, clause);
        } else {
            sprintf(query, "select * from %s %s", table_name, clause);
        }

        x = db->do_query(query);
        if (x) return mysql_errno(db->mysql);

        // if you use mysql_use_result() here,
        // any other transactions will fail
        //
        if (use_use_result) {
            cursor.rp = mysql_use_result(db->mysql);
        } else {
            cursor.rp = mysql_store_result(db->mysql);
        }
        if (!cursor.rp) return mysql_errno(db->mysql);
    }
    row = mysql_fetch_row(cursor.rp);
    if (!row) {
        mysql_free_result(cursor.rp);
        cursor.active = false;
        x = mysql_errno(db->mysql);
        if (x) return x;
        return ERR_DB_NOT_FOUND;
    } else {
        db_parse(row);
    }
    return 0;
}

// call this to end an enumeration before reaching end
//
int DB_BASE::end_enumerate() {
    if (cursor.active) {
        mysql_free_result(cursor.rp);
        cursor.active = false;
    }
    return 0;
}

int DB_BASE::get_integer(const char* query, int& n) {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* resp;

    retval = db->do_query(query);
    if (retval) return retval;
    resp = mysql_store_result(db->mysql);
    if (!resp) return ERR_DB_NOT_FOUND;
    row = mysql_fetch_row(resp);
    mysql_free_result(resp);
    if (!row) return ERR_DB_NOT_FOUND;
    if (!row[0]) return ERR_DB_NOT_FOUND;
    n = atoi(row[0]);
    return 0;
}

int DB_BASE::get_double(const char* query, double& x) {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* resp;

    retval = db->do_query(query);
    if (retval) return retval;
    resp = mysql_store_result(db->mysql);
    if (!resp) return ERR_DB_NOT_FOUND;
    row = mysql_fetch_row(resp);
    mysql_free_result(resp);
    if (!row) return ERR_DB_NOT_FOUND;
    if (!row[0]) return ERR_DB_NOT_FOUND;
    x = atof(row[0]);
    return 0;
}

int DB_BASE::count(int& n, const char* clause) {
    char query[MAX_QUERY_LEN];

    if (is_high_priority) {
        sprintf(query, "select HIGH_PRIORITY count(*) from %s %s", table_name, clause);
    } else {
        sprintf(query, "select count(*) from %s %s", table_name, clause);
    }

    return get_integer(query, n);
}

int DB_BASE::max_id(int& n, const char* clause) {
    char query[MAX_QUERY_LEN];
    sprintf(query, "select max(id) from %s %s", table_name, clause);
    return get_integer(query, n);
}

int DB_BASE::sum(double& x, const char* field, const char* clause) {
    char query[MAX_QUERY_LEN];

    if (is_high_priority) {
        sprintf(query, "select HIGH_PRIORITY sum(%s) from %s %s", field, table_name, clause);
    } else {
        sprintf(query, "select sum(%s) from %s %s", field, table_name, clause);
    }

    return get_double(query, x);
}

DB_BASE_SPECIAL::DB_BASE_SPECIAL(DB_CONN* p) : db(p) {
}

// convert a string into a form that allows it to be used
// in SQL queries delimited by single quotes:
// replace ' with \', '\' with '\\'
//
void escape_string(char* field, int len) {
    char buf[MAX_QUERY_LEN];
    char* q = buf, *p = field;

    // Make sure that final result won't overflow field[].
    // Don't need to worry about overflowing buf[] since 
    // in worst case string length only doubles.
    //
    while (*p && q < buf+len-2) {
        if (*p == '\'') {
            // this does ' to \' transformation 
            //
            *q++ = '\\';
            *q++ = '\'';
        } else if (*p == '\\') {
            // this does \ to \\ transformation
            //
            *q++ = '\\';
            *q++ = '\\';
        } else {
            // this handles all other characters
            //
            *q++ = *p;
        }
        p++;
    }
    *q = 0;
    strlcpy(field, buf, len);
}

// undo the above process
// (len not used because this doesn't expand the string)
//
void unescape_string(char* p, int /*len*/) {
    char* q = p;
    while (*p) {
        if (*p == '\\') {
            p++;
            if (!*p) break; // SHOULD NEVER HAPPEN!
        }
        *q++ = *p++;
    }
    *q='\0';
}

// replace _ with \\_, % with \\%
//
void escape_mysql_like_pattern(const char* in, char* out) {
    while (*in) {
        if (*in == '_' || *in == '%') {
            *out++ = '\\';
            *out++ = '\\';
        }
        *out++ = *in++;
    }
}

const char *BOINC_RCSID_43d919556b = "$Id$";
