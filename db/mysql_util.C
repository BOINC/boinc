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

// A collection of functions to facilitate MySQL access.
// Should be independent of the actual database
// (other than the assumption that ID is always field zero)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mysql.h"
#include "mysql_util.h"

#define MAX_QUERY_LEN   8192

int MYSQL_DB::db_open(char* dbname, char* password) {
    mp = mysql_init(0);
    if (!mp) return -1;
    mp = mysql_real_connect(mp, 0, 0, password, dbname, 0, 0, 0);
    if (!mp) return -1;
    return 0;
}

int MYSQL_DB::db_close() {
    mysql_close(mp);
    return 0;
}

const char* MYSQL_DB::db_error_string() {
    return mp?mysql_error(mp):"Not connected";
}

void MYSQL_DB::db_print_error(char* p) {
    if (mp) {
        fprintf(stderr, "%s: Database error: %s\n", p, mysql_error(mp));
    }
}

// convert ' to \' in place
void escape_single_quotes(char* field) {
    char buf[MAX_QUERY_LEN];
    char* q = buf, *p = field;
    while (*p) {
        if (*p == '\'') {
            *q++ = '\\';
            *q++ = '\'';
        } else {
            *q++ = *p;
        }
        p++;
    }
    *q = 0;
    strcpy(field, buf);
}

void unescape_single_quotes(char* p) {
    char* q;
    while (1) {
        q = strstr(p, "\\'");
        if (!q) break;
        strcpy(q, q+1);
    }
}

// assumes ID is first
static int* id(void* vp, int type) {
    return (int*) vp;
}

int MYSQL_DB::db_new(void* vp, int type) {
    char buf[MAX_QUERY_LEN], sbuf[MAX_QUERY_LEN];
    struct_to_str(vp, sbuf, type);
    sprintf(buf, "insert into %s set %s", table_name[type], sbuf);
    return mysql_query(mp, buf);
}

int MYSQL_DB::db_insert_id() {
    mysql_query(mp, "select LAST_INSERT_ID()");
    rp = mysql_store_result(mp);
    row = mysql_fetch_row(rp);
    return atoi(row[0]);
}

int MYSQL_DB::db_delete(int id, int type) {
    char buf[MAX_QUERY_LEN];

    sprintf(buf, "delete from %s where id=%d", table_name[type], id);
    return mysql_query(mp, buf);
}

int MYSQL_DB::db_lookup_id(int i, void* vp, int type) {
    char buf[MAX_QUERY_LEN];
    sprintf(buf, "select * from %s where id=%d", table_name[type], i);
    mysql_query(mp, buf);
    rp = mysql_store_result(mp);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) row_to_struct(row, vp, type);
    mysql_free_result(rp);
    return (row == 0);
}

int MYSQL_DB::db_lookup(void* vp, int type, char* clause) {
    char buf[MAX_QUERY_LEN];
    sprintf(buf, "select * from %s where %s", table_name[type], clause);
    mysql_query(mp, buf);
    rp = mysql_store_result(mp);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) row_to_struct(row, vp, type);
    mysql_free_result(rp);
    return (row == 0);
}

int MYSQL_DB::db_update(void* vp, int type) {
    char buf[MAX_QUERY_LEN], sbuf[MAX_QUERY_LEN];
    struct_to_str(vp, sbuf, type);
    sprintf(
        buf,
        "update %s set %s where id=%d",
        table_name[type], sbuf, *id(vp, type)
    );
    return mysql_query(mp, buf);
}

int MYSQL_DB::db_enum(ENUM& e, void* p, int type, char* clause, int limit) {
    char buf[MAX_QUERY_LEN], buf2[256];
    if (!e.active) {
        e.active = 1;
        sprintf(buf, "select * from %s %s", table_name[type], clause?clause:"");
        if (limit) {
            sprintf(buf2, " limit %d", limit);
            strcat(buf, buf2);
        }
        mysql_query(mp, buf);
        e.rp = mysql_store_result(mp);
        if (!e.rp) return -1;
    }
    row = mysql_fetch_row(e.rp);
    if (!row) {
        mysql_free_result(e.rp);
        e.active = 0;
        return 1;
    } else {
        row_to_struct(row, p, type);
        return 0;
    }
}

int MYSQL_DB::db_enum_field(ENUM& e, int type, char* field, char* clause) {
    char buf[MAX_QUERY_LEN];
    if (!e.active) {
        e.active = 1;
        sprintf(buf, "select %s from %s %s", field, table_name[type], clause);
        mysql_query(mp, buf);
        e.rp = mysql_store_result(mp);
        if (!e.rp) return -1;
    }
    row = mysql_fetch_row(e.rp);
    if (!row) {
        mysql_free_result(e.rp);
        e.active = 0;
        return 1;
    } else {
        return 0;
    }
}

int MYSQL_DB::db_query_int(int* ip, char* query) {
    mysql_query(mp, query);
    rp = mysql_store_result(mp);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (!row) return -1;
    *ip = atoi(row[0]);
    mysql_free_result(rp);
    return 0;
}

int MYSQL_DB::db_count(int* np, char* what, int type, char* clause) {
    char buf[MAX_QUERY_LEN];
    sprintf(buf, "select count(%s) from %s ", what, table_name[type]);
    if (clause) strcat(buf, clause);
    return db_query_int(np, buf);
}

int MYSQL_DB::db_query(char* p) {
    return mysql_query(mp, p);
}
