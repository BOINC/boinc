// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <mysql.h>

#include "error_numbers.h"
#include "str_util.h"
#include "str_replace.h"
#include "db_base.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#include "sched_msgs.h"
#endif

bool g_print_queries = false;

DB_CONN::DB_CONN() {
    mysql = 0;
}

int DB_CONN::open(
    char* db_name, char* db_host, char* db_user, char* dbpassword
) {
    mysql = mysql_init(0);
    if (!mysql) return ERR_DB_CANT_INIT;

    // MySQL's support for the reconnect option has changed over time:
    // see http://dev.mysql.com/doc/refman/5.0/en/mysql-options.html
    // and http://dev.mysql.com/doc/refman/5.1/en/mysql-options.html
    //
    // v < 5.0.13: not supported
    // 5.0.13 <= v < 5.0.19: set option after real_connect()
    // 5.0.19 < v < 5.1: set option before real_connect();
    // 5.1.0 <= v < 5.1.6: set option after real_connect()
    // 5.1.6 <= v: set option before real_connect

    int v = MYSQL_VERSION_ID;
    bool set_opt_before = false, set_opt_after = false;
    if (v < 50013 ) {
    } else if (v < 50019) {
        set_opt_after = true;
    } else if (v < 50100) {
        set_opt_before = true;
    } else if (v < 50106) {
        set_opt_after = true;
    } else {
        set_opt_before = true;
    }

    if (set_opt_before) {
        my_bool mbReconnect = 1;
        mysql_options(mysql, MYSQL_OPT_RECONNECT, &mbReconnect);
    }

    // parse hostname:port
    //
    char host[256];
    int port = 0;
    strcpy(host, db_host);
    char* p = strchr(host, ':');
    if (p) {
        *p = 0;
        port = atoi(p+1);
    }

    // CLIENT_FOUND_ROWS means that the # of affected rows for an update
    // is the # matched by the where, rather than the # actually changed
    //
    mysql = mysql_real_connect(
        mysql, host, db_user, dbpassword, db_name, port, 0, CLIENT_FOUND_ROWS
    );
    if (mysql == 0) return ERR_DB_CANT_CONNECT;

    if (set_opt_after) {
        my_bool mbReconnect = 1;
        mysql_options(mysql, MYSQL_OPT_RECONNECT, &mbReconnect);
    }
    return 0;
}

void DB_CONN::close() {
    if (mysql) mysql_close(mysql);
}

int DB_CONN::set_isolation_level(ISOLATION_LEVEL level) {
    const char* level_str;
    char query[256];

    switch(level) {
    case READ_UNCOMMITTED:
        level_str = "READ UNCOMMITTED";
        break;
    case READ_COMMITTED:
        level_str = "READ COMMITTED";
        break;
    case REPEATABLE_READ:
        level_str = "REPEATABLE READ";
        break;
    case SERIALIZABLE:
        level_str = "SERIALIZABLE";
        break;
    default:
        return -1;
    }
    sprintf(query, "SET SESSION TRANSACTION ISOLATION LEVEL %s", level_str);
    return do_query(query);
}

int DB_CONN::do_query(const char* p) {
    int retval;
    if (g_print_queries) {
#ifdef _USING_FCGI_
        log_messages.printf(MSG_NORMAL, "query: %s\n", p);
#else
        fprintf(stderr, "query: %s\n", p);
#endif
    }
    retval = mysql_query(mysql, p);
    if (retval) {
        fprintf(stderr, "Database error: %s\nquery=%s\n", error_string(), p);
    }
    return retval;
}

// returns the number of rows matched by an update query
//
int DB_CONN::affected_rows() {
    unsigned long x = (unsigned long)mysql_affected_rows(mysql);
    //fprintf(stderr, "x: %lu i: %d\n", x, (int)x);
    return (int)x;
}

int DB_CONN::insert_id() {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    retval = do_query("select LAST_INSERT_ID()");
    if (retval) return retval;
    rp = mysql_store_result(mysql);
    row = mysql_fetch_row(rp);
    int x = atoi(row[0]);
    mysql_free_result(rp);
    return x;
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

int DB_CONN::rollback_transaction() {
	return do_query("ROLLBACK");
}

int DB_CONN::ping() {
    int retval = mysql_ping(mysql);
    if (retval) return ERR_DB_CANT_CONNECT;
    return 0;
}

DB_BASE::DB_BASE(const char *tn, DB_CONN* p) : db(p), table_name(tn) {
}

int DB_BASE::get_id() { return 0;}
void DB_BASE::db_print(char*) {}
void DB_BASE::db_parse(MYSQL_ROW&) {}

int DB_BASE::insert() {
    char vals[MAX_QUERY_LEN*2], query[MAX_QUERY_LEN*2];
    db_print(vals);
    sprintf(query, "insert into %s set %s", table_name, vals);
    return db->do_query(query);
}

int DB_BASE::insert_batch(std::string& values) {
    std::string query;
    query = "insert into " + std::string(table_name) + " values " + values;
    return db->do_query(query.c_str());
}

int DB_BASE::affected_rows() {
    return db->affected_rows();
}

//////////// FUNCTIONS FOR TABLES THAT HAVE AN ID FIELD ///////

int DB_BASE::lookup_id(int id) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query, "select * from %s where id=%u", table_name, id);

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

// update an entire record
//
int DB_BASE::update() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "update %s set %s where id=%u", table_name, vals, get_id());
    int retval = db->do_query(query);
    if (retval) return retval;
    if (db->affected_rows() != 1) return ERR_DB_NOT_FOUND;
    return 0;
}

// update one or more fields
// "clause" is something like "foo=5, blah='xxx'" or "foo=foo+5"
//
int DB_BASE::update_field(const char* clause, const char* where_clause) {
    char query[MAX_QUERY_LEN];
    if (where_clause) {
        sprintf(query, "update %s set %s where id=%u and %s", table_name, clause, get_id(), where_clause);
    } else {
        sprintf(query, "update %s set %s where id=%u", table_name, clause, get_id());
    }
    return db->do_query(query);
}

// delete record
//
int DB_BASE::delete_from_db() {
    char query[MAX_QUERY_LEN];
    sprintf(query, "delete from %s where id=%u", table_name, get_id());
    return db->do_query(query);
}

int DB_BASE::delete_from_db_multi(const char* where_clause) {
    char query[MAX_QUERY_LEN];
    sprintf(query, "delete from %s where %s", table_name, where_clause);
    return db->do_query(query);
}

int DB_BASE::get_field_ints(const char* fields, int nfields, int* vals) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query,
        "select %s from %s where id=%u", fields, table_name, get_id()
    );
    retval = db->do_query(query);
    if (retval) return retval;
    rp = mysql_store_result(db->mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) {
        for (int i=0; i<nfields; i++) {
            vals[i] = atoi(row[i]);
        }
    }
    mysql_free_result(rp);
    if (row == 0) return ERR_DB_NOT_FOUND;
    return 0;
}

int DB_BASE::get_field_str(const char* field, char* buf, int buflen) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query,
        "select %s from %s where id=%u", field, table_name, get_id()
    );
    retval = db->do_query(query);
    if (retval) return retval;
    rp = mysql_store_result(db->mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row && row[0]) {
        strlcpy(buf, row[0], buflen);
    }
    mysql_free_result(rp);
    if (row == 0) return ERR_DB_NOT_FOUND;
    return 0;
}

int DB_BASE::max_id(int& n, const char* clause) {
    char query[MAX_QUERY_LEN];
    sprintf(query, "select max(id) from %s %s", table_name, clause);
    return get_integer(query, n);
}

/////////////// FUNCTIONS THAT DON'T REQUIRE AN ID FIELD ///////////////

int DB_BASE::lookup(const char* clause) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query, "select * from %s %s", table_name, clause);

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

int DB_BASE::update_fields_noid(char* set_clause, char* where_clause) {
    char query[MAX_QUERY_LEN];
    sprintf(query,
        "update %s set %s where %s",
        table_name, set_clause, where_clause
    );
    int retval = db->do_query(query);
    if (retval) return retval;
    return 0;
}

int DB_BASE::enumerate(const char* clause, bool use_use_result) {
    int x;
    char query[MAX_QUERY_LEN];
    MYSQL_ROW row;

    if (!cursor.active) {
        cursor.active = true;

        sprintf(query, "select * from %s %s", table_name, clause);

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
    if (!row || !row[0]) {
        retval = ERR_DB_NOT_FOUND;
    } else {
        n = atoi(row[0]);
    }
    mysql_free_result(resp);
    return retval;
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
    if (!row || !row[0]) {
        retval = ERR_DB_NOT_FOUND;
    } else {
        x = atof(row[0]);
    }
    mysql_free_result(resp);
    return retval;
}

int DB_BASE::count(int& n, const char* clause) {
    char query[MAX_QUERY_LEN];

    sprintf(query, "select count(*) from %s %s", table_name, clause);

    return get_integer(query, n);
}

int DB_BASE::sum(double& x, const char* field, const char* clause) {
    char query[MAX_QUERY_LEN];

    sprintf(query, "select sum(%s) from %s %s", field, table_name, clause);

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

    if (len > MAX_QUERY_LEN) {
        len = MAX_QUERY_LEN;
    }

    // Make sure that final result won't overflow field[].
    //
    while (*p && q < buf+len-3) {
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
