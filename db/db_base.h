#include "mysql.h"

struct CURSOR {
    bool active;
    MYSQL_RES *rp;
};

// Base for derived classes that can access the DB
// Defines various generic operations on DB tables
//
class DB_BASE {
public:
    DB_BASE(char *table_name);
    int insert();
    int update();
    int lookup_id(int id);
    int lookup(char*);
    int enumerate(char* clause="");
    int count(int&, char* clause="");
    int sum(double&, char* field, char* clause="");
    int get_double(char* query, double&);
    int get_integer(char* query, int&);

    const char *table_name;
    CURSOR cursor;
    virtual int get_id();
    virtual void db_print(char*);
    virtual void db_parse(MYSQL_ROW&);
};

