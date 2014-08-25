#include <stdlib.h>
#include <stdio.h>

#include "test_utils.h"
#include "test_dao.h"

int main(int argc, char** argv) {
    db_connect();
    //db_execute_direct("drop table bench");
    db_execute_direct(
#ifdef TT
            "CREATE TABLE bench ("
            "id TT_INT NOT NULL, "
            "value TT_INT NOT NULL, "
            "PRIMARY KEY (id) "
            ")"
            " UNIQUE HASH ON (id) pages = 40000"
#endif
#ifdef PQ
            "CREATE TABLE bench ("
            "id integer NOT NULL, "
            "value integer NOT NULL, "
            "CONSTRAINT bench_pk PRIMARY KEY (id) "
            ")"
#endif
            );

    db_prepare();
    exit_on_error = false;
    debug=true;
    db_insert(1,1);
    db_insert(1,2);
    db_insert(2,3);
    int32_t value;
    db_select(2,&value);
    db_execute_direct("drop table bench");
    db_disconnect();
    return 0;
}


