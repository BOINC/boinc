#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "error_numbers.h"
#include "db_base.h"

#define MAX_QUERY_LEN 8192

DB_CONN::DB_CONN() {
    mysql = 0;
}

int DB_CONN::open(char* dbname, char* dbpassword) {
    char buf[256],*db_name,*db_host,*p;
    if (dbname) {
        strncpy(buf,dbname,254);
        buf[255]=0;
        db_name=buf;
    } else {
        db_name=0;
    }
    if ((p=strchr(buf,'@'))) {
        db_host=p+1;
      *p=0;
    } else {
        db_host=0;
    }
    mysql = mysql_init(0);
    if (!mysql) return 0;
    mysql = mysql_real_connect(mysql, db_host, 0, dbpassword, db_name, 0, 0, 0);
    if (mysql == 0) return ERR_DB_CANT_CONNECT;
    return 0;
}

void DB_CONN::close() {
    if (mysql) mysql_close(mysql);
}

int DB_CONN::insert_id() {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;
            
    retval = mysql_query(mysql, "select LAST_INSERT_ID()");
    if (retval) return retval;
    rp = mysql_store_result(mysql);
    row = mysql_fetch_row(rp);
    return atoi(row[0]);
}

void DB_CONN::print_error(char* p) {
    if (mysql) {
        fprintf(stderr, "%s: Database error: %s\n", p, mysql_error(mysql));
    } else {
        fprintf(stderr, "%s: Database error\n", p);
    }
}

const char* DB_CONN::error_string() {
    return mysql?mysql_error(mysql):"Not connected";
}

DB_BASE::DB_BASE(DB_CONN& p, char *tn) : db(&p), table_name(tn) {
    cursor.active = false;
}

int DB_BASE::get_id() { return 0;}
void DB_BASE::db_print(char*) {}

void DB_BASE::db_parse(MYSQL_ROW&) {}

int DB_BASE::insert() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "insert into %s set %s", table_name, vals);
    return mysql_query(db->mysql, query);
}

int DB_BASE::update() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "update %s set %s where id=%d", table_name, vals, get_id());
    return mysql_query(db->mysql, query);
}

int DB_BASE::lookup(char* clause) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query, "select * from %s %s", table_name, clause);
    retval = mysql_query(db->mysql, query);
    if (retval) return retval;
    rp = mysql_store_result(db->mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) db_parse(row);
    mysql_free_result(rp);
    if (row == 0) return ERR_DB_NOT_FOUND;

    // make sure there's exactly one row
    //
    row = mysql_fetch_row(rp);
    if (row) return ERR_DB_NOT_UNIQUE;
    return 0;
}

int DB_BASE::lookup_id(int id) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query, "select * from %s where id=%d", table_name, id);
    retval = mysql_query(db->mysql, query);
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

int DB_BASE::enumerate(char* clause) {
    int x;
    char query[MAX_QUERY_LEN];
    MYSQL_ROW row;

    if (!cursor.active) {
        cursor.active = true;
        sprintf(query, "select * from %s %s", table_name, clause);
        x = mysql_query(db->mysql, query);
        if (x) return mysql_errno(db->mysql);
        cursor.rp = mysql_store_result(db->mysql);
        if (!cursor.rp) return mysql_errno(db->mysql);
    }
    row = mysql_fetch_row(cursor.rp);
    if (!row) {
        mysql_free_result(cursor.rp);
        cursor.active = false;
        return 1;
    } else {
        db_parse(row);
    }
    return 0;
}

int DB_BASE::get_integer(char* query, int& n) {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* resp;

    retval = mysql_query(db->mysql, query);
    if (retval) return retval;
    resp = mysql_store_result(db->mysql);
    if (!resp) return ERR_DB_NOT_FOUND;
    row = mysql_fetch_row(resp);
    if (!row) return ERR_DB_NOT_FOUND;
    if (!row[0]) return ERR_DB_NOT_FOUND;
    n = atoi(row[0]);
    mysql_free_result(resp);
    return 0;
}

int DB_BASE::get_double(char* query, double& x) {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* resp;

    retval = mysql_query(db->mysql, query);
    if (retval) return retval;
    resp = mysql_store_result(db->mysql);
    if (!resp) return ERR_DB_NOT_FOUND;
    row = mysql_fetch_row(resp);
    if (!row) return ERR_DB_NOT_FOUND;
    if (!row[0]) return ERR_DB_NOT_FOUND;
    x = atof(row[0]);
    mysql_free_result(resp);
    return 0;
}

int DB_BASE::count(int& n, char* clause) {
    char query[MAX_QUERY_LEN];
    sprintf(query, "select count(*) from %s %s", table_name, clause);
    return get_integer(query, n);
}

int DB_BASE::sum(double& x, char* field, char* clause) {
    char query[MAX_QUERY_LEN];
    sprintf(query, "select sum(%s) from %s %s", field, table_name, clause);
    return get_double(query, x);
}

#if 0
void strcpy2(char* dest, char* src) {
    if (!src) *dest = 0;
    else strcpy(dest, src);
}
#endif

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

