DB_BASE::DB_BASE(char *tn) : table_name(tn) {
    cursor.active = false;
}

int DB_BASE::get_id() { return 0;}
void DB_BASE::db_print(char*) {}

void DB_BASE::db_parse(MYSQL_ROW&) {}

int DB_BASE::insert() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "insert into %s set %s", table_name, vals);
    return mysql_query(mysql, query);
}

int DB_BASE::update() {
    char vals[MAX_QUERY_LEN], query[MAX_QUERY_LEN];
    db_print(vals);
    sprintf(query, "update %s set %s where id=%d", table_name, vals, get_id());
    return mysql_query(mysql, query);
}

int DB_BASE::lookup(char* clause) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query, "select * from %s %s", table_name, clause);
    retval = mysql_query(mysql, query);
    if (retval) return retval;
    rp = mysql_store_result(mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) db_parse(row);
    mysql_free_result(rp);
    return (row == 0);
}

int DB_BASE::lookup_id(int id) {
    char query[MAX_QUERY_LEN];
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* rp;

    sprintf(query, "select * from %s where id=%d", table_name, id);
    retval = mysql_query(mysql, query);
    if (retval) return retval;
    rp = mysql_store_result(mysql);
    if (!rp) return -1;
    row = mysql_fetch_row(rp);
    if (row) db_parse(row);
    mysql_free_result(rp);
    return (row == 0);
}

int DB_BASE::enumerate(char* clause) {
    int x;
    char query[MAX_QUERY_LEN];
    MYSQL_ROW row;

    if (!cursor.active) {
        cursor.active = true;
        sprintf(query, "select * from %s %s", table_name, clause);
        x = mysql_query(mysql, query);
        if (x) return mysql_errno(mysql);
        cursor.rp = mysql_store_result(mysql);
        if (!cursor.rp) return mysql_errno(mysql);
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

    retval = mysql_query(mysql, query);
    if (retval) return retval;
    resp = mysql_store_result(mysql);
    if (!resp) return -1;
    row = mysql_fetch_row(resp);
    if (!row) return -1;
    if (!row[0]) return -1;
    n = atoi(row[0]);
    mysql_free_result(resp);
    return 0;
}

int DB_BASE::get_double(char* query, double& x) {
    int retval;
    MYSQL_ROW row;
    MYSQL_RES* resp;

    retval = mysql_query(mysql, query);
    if (retval) return retval;
    resp = mysql_store_result(mysql);
    if (!resp) return -1;
    row = mysql_fetch_row(resp);
    if (!row) return -1;
    if (!row[0]) return -1;
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

static void strcpy2(char* dest, char* src) {
    if (!src) *dest = 0;
    else strcpy(dest, src);
}

// convert ' to \' in place
static void escape_single_quotes(char* field) {
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

static void unescape_single_quotes(char* p) {
    char* q;
    while (1) {
        q = strstr(p, "\\'");
        if (!q) break;
        strcpy(q, q+1);
    }
}

