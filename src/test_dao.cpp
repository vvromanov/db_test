#include <sql.h>
#include <sqlext.h>
#include <stdlib.h>
#include <stdio.h>
#include "test_utils.h"
#include "test_dao.h"

#ifdef MY
#define DSN "dsn=mytest"
#else
#define DSN "dsn=test"
//#define DSN "TTC_SERVER=127.0.0.1;TTC_SERVER_DSN=test"
#endif     
SQLHENV henv = SQL_NULL_HENV;
SQLHDBC hdbc = SQL_NULL_HDBC;
SQLHSTMT stmt_direct = SQL_NULL_HSTMT;
SQLHSTMT stmt_insert = SQL_NULL_HSTMT;
SQLHSTMT stmt_select = SQL_NULL_HSTMT;
SQLHSTMT stmt_select1 = SQL_NULL_HSTMT;
SQLHSTMT stmt_update = SQL_NULL_HSTMT;
SQLHSTMT stmt_delete = SQL_NULL_HSTMT;

static int32_t _id, _value;

#define CHECK(x) \
    do {\
        const SQLRETURN rc = (x);\
        if (debug) { \
           fprintf(stderr, "Retcode [%d] when executing %s\n", rc, #x); \
        } \
        if ( rc != SQL_SUCCESS ) {\
            fprintf(stderr, "Error [%d] in %s", rc, #x); \
            log_error(rc); \
            if (exit_on_error) exit(EXIT_FAILURE); \
        }\
    } while(0)

#define CHECK_STMT(x, stmt) \
    do {\
        const SQLRETURN rc = (x);\
        if (debug) { \
           fprintf(stderr, "Retcode [%d] when executing %s\n", rc, #x); \
        } \
        if ( rc != SQL_SUCCESS ) {\
            fprintf(stderr, "Error [%d] in %s", rc, #x); \
            log_error(rc, stmt); \
            if (exit_on_error) exit(EXIT_FAILURE); \
        }\
    } while(0)

void log_error(SQLRETURN rc, SQLHSTMT stmt = SQL_NULL_HSTMT) {
    if (rc == SQL_SUCCESS || rc == SQL_NO_DATA_FOUND) {
        return;
    }

    if (rc == SQL_INVALID_HANDLE) {
        printf("ERROR: invalid handle\n");
        return;
    } else if (rc == SQL_SUCCESS_WITH_INFO) {
        printf("WARNING:\n");
    } else if (rc == SQL_ERROR) {
        printf("ERROR:\n");
    }
    while (1) {
        char state[100] = "", message[10240] = "";
        SQLINTEGER native_error = 0;
        SQLSMALLINT pcbErrorMsg;
        SQLRETURN ret = SQLError(henv, hdbc, stmt, (SQLCHAR*) state, &native_error, (SQLCHAR*) message, sizeof (message), &pcbErrorMsg);
        if (ret == SQL_NO_DATA_FOUND) break;
        puts(message);
        printf("ODBC Error/Warning = %s, Additional Error/Warning = %d\n", state, native_error);
        if (ret == SQL_SUCCESS_WITH_INFO) {
            puts("(Note: error message was truncated)");
        }
    }
}

void db_execute_direct(const char* sql) {
    printf("Execute direct [%s] ... ", sql);
    fflush(stdout);
    int64_t start = getTimeMs();
    //CHECK(SQLFreeStmt(stmt_direct, SQL_UNBIND));
    CHECK_STMT(SQLExecDirect(stmt_direct, const_cast<SQLCHAR*> ((const SQLCHAR*) sql), SQL_NTS), stmt_direct);
    CHECK_STMT(SQLFreeStmt(stmt_direct, SQL_UNBIND), stmt_direct);
    int64_t delta = getTimeMs() - start;
    printf("Done! (%ld ms)\n", delta);
}

void db_connect() {
    CHECK(SQLAllocEnv(&henv));
    CHECK(SQLAllocConnect(henv, &hdbc));
    CHECK(SQLDriverConnect(hdbc, NULL, (SQLCHAR *) DSN, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
    CHECK(SQLAllocStmt(hdbc, &stmt_direct));
    CHECK(SQLSetConnectOption(hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON));
    printf("Connection established\n");
}

void db_disconnect() {
    CHECK(SQLDisconnect(hdbc));
    CHECK(SQLFreeConnect(hdbc));
    CHECK(SQLFreeEnv(henv));
    printf("Connection closed\n");
}

void db_prepare() {
    printf("Prepare ... ");
    fflush(stdout);
    CHECK(SQLAllocStmt(hdbc, &stmt_insert));
    CHECK(SQLAllocStmt(hdbc, &stmt_select));
    CHECK(SQLAllocStmt(hdbc, &stmt_select1));
    CHECK(SQLAllocStmt(hdbc, &stmt_update));
    CHECK(SQLAllocStmt(hdbc, &stmt_delete));

    CHECK(SQLPrepare(stmt_insert, (SQLCHAR*) Q_INSERT, SQL_NTS));
    CHECK(SQLBindParameter(stmt_insert, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_id, 0, NULL));
    CHECK(SQLBindParameter(stmt_insert, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_value, 0, NULL));

    CHECK(SQLPrepare(stmt_select, (SQLCHAR*) Q_SELECT, SQL_NTS));
    CHECK(SQLBindParameter(stmt_select, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_id, 0, NULL));
    CHECK(SQLBindCol(stmt_select, 1, SQL_C_SLONG, &_value, sizeof (_value), NULL));
   
    CHECK(SQLPrepare(stmt_select1, (SQLCHAR*) Q_SELECT1, SQL_NTS));
    CHECK(SQLBindCol(stmt_select1, 1, SQL_C_SLONG, &_value, sizeof (_value), NULL));

    CHECK(SQLPrepare(stmt_update, (SQLCHAR*) Q_UPDATE, SQL_NTS));
    CHECK(SQLBindParameter(stmt_update, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_value, 0, NULL));
    CHECK(SQLBindParameter(stmt_update, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_id, 0, NULL));

    CHECK(SQLPrepare(stmt_delete, (SQLCHAR*) Q_DELETE, SQL_NTS));
    CHECK(SQLBindParameter(stmt_delete, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_id, 0, NULL));

    printf("Done!\n");
}

void db_insert(int32_t id, int32_t value) {
    if (debug) {
        fprintf(stderr, "db_insert(%d,%d)\n", id, value);
    }
    _id = id;
    _value = value;
    CHECK_STMT(SQLExecute(stmt_insert), stmt_insert);
}

void db_select(int32_t id, int32_t* value) {
    _id = id;
    _value = -1;
    if (debug) {
        fprintf(stderr, "db_select(%d)\n", id);
    }
    CHECK_STMT(SQLExecute(stmt_select), stmt_select);
    CHECK_STMT(SQLFetch(stmt_select), stmt_select);
    CHECK_STMT(SQLFreeStmt(stmt_select, SQL_CLOSE), stmt_select);
    if (debug) {
        fprintf(stderr, "db_select(%d) return %d\n", id, _value);
    }
    *value = _value;
}

void db_select1(int32_t* value) {
    _value = -1;
    if (debug) {
        fprintf(stderr, "db_select1()\n");
    }
    CHECK_STMT(SQLExecute(stmt_select1), stmt_select);
    CHECK_STMT(SQLFetch(stmt_select1), stmt_select);
    CHECK_STMT(SQLFreeStmt(stmt_select1, SQL_CLOSE), stmt_select);
    if (debug) {
        fprintf(stderr, "db_select1() return %d\n", _value);
    }
    *value = _value;
}

void db_update(int32_t id, int32_t value) {
    _id = id;
    _value = value;
    if (debug) {
        fprintf(stderr, "db_update(%d,%d)\n", id, value);
    }
    CHECK_STMT(SQLExecute(stmt_update), stmt_update);
}

void db_delete(int32_t id) {
    _id = id;
    if (debug) {
        fprintf(stderr, "db_delete(%d)\n", id);
    }
    CHECK_STMT(SQLExecute(stmt_delete), stmt_delete);
}

