#include <sql.h>
#include <sqlext.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

#ifdef TT
#define TEST_COUNT 5000000
#endif
#ifdef PQ
#define TEST_COUNT 500000
#endif

typedef struct {
    int32_t id;
    int32_t value;
    int32_t rnd_key;
} test_data_t;

test_data_t test_data[TEST_COUNT];

#define DSN "dsn=test"
SQLHENV henv = SQL_NULL_HENV;
SQLHDBC hdbc = SQL_NULL_HDBC;
SQLHSTMT stmt_direct = SQL_NULL_HSTMT;
SQLHSTMT stmt_insert = SQL_NULL_HSTMT;
SQLHSTMT stmt_select = SQL_NULL_HSTMT;
SQLHSTMT stmt_update = SQL_NULL_HSTMT;
SQLHSTMT stmt_delete = SQL_NULL_HSTMT;

int32_t _id, _value;

static inline int64_t getTimeMs(void) {
    struct timeval val;
    gettimeofday(&val, NULL);
    return val.tv_sec * ((int64_t) 1000) + val.tv_usec / 1000;
}

#define CHECK(x) \
    do {\
        const SQLRETURN rc = (x);\
        if ( rc != SQL_SUCCESS ) {\
            fprintf(stderr, "Error [%d] in %s", rc, #x); \
            log_error(rc); \
            exit(EXIT_FAILURE); \
        }\
    } while(0)

#define CHECK_STMT(x, stmt) \
    do {\
        const SQLRETURN rc = (x);\
        if ( rc != SQL_SUCCESS ) {\
            fprintf(stderr, "Error [%d] in %s", rc, #x); \
            log_error(rc, stmt); \
            exit(EXIT_FAILURE); \
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
        puts(message);
        printf("ODBC Error/Warning = %s, Additional Error/Warning = %d\n", state, native_error);
        if (ret == SQL_SUCCESS_WITH_INFO) {
            puts("(Note: error message was truncated)");
        }
        if (ret == SQL_NO_DATA_FOUND) break;
    }
}

static SQLCHAR* get_str(const char* tt_str, const char* pq_str) {
#ifdef TT
    return const_cast<SQLCHAR*> ((const SQLCHAR*) tt_str);
#endif
#ifdef PQ
    return const_cast<SQLCHAR*> ((const SQLCHAR*) pq_str);
#endif    
}

void execute_direct(const char* sql) {
    printf("Execute direct [%s] ... ", sql);
    fflush(stdout);
    int64_t start = getTimeMs();
    //CHECK(SQLFreeStmt(stmt_direct, SQL_UNBIND));
    CHECK_STMT(SQLExecDirect(stmt_direct, const_cast<SQLCHAR*> ((const SQLCHAR*) sql), SQL_NTS), stmt_direct);
    CHECK_STMT(SQLFreeStmt(stmt_direct, SQL_UNBIND), stmt_direct);
    int64_t delta = getTimeMs() - start;
    printf("Done! (%ld ms)\n", delta);
}

void connect() {
    CHECK(SQLAllocEnv(&henv));
    CHECK(SQLAllocConnect(henv, &hdbc));
    CHECK(SQLDriverConnect(hdbc, NULL, (SQLCHAR *) DSN, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
    CHECK(SQLAllocStmt(hdbc, &stmt_direct));
    CHECK(SQLSetConnectOption(hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON));
    printf("Connection established\n");
}

void prepare() {
    printf("Prepare ... ");
    fflush(stdout);
    CHECK(SQLAllocStmt(hdbc, &stmt_insert));
    CHECK(SQLAllocStmt(hdbc, &stmt_select));
    CHECK(SQLAllocStmt(hdbc, &stmt_update));
    CHECK(SQLAllocStmt(hdbc, &stmt_delete));

    CHECK(SQLPrepare(stmt_insert, get_str(
            "INSERT INTO bench (id, value) VALUES (:id, :value)",
            "INSERT INTO bench (id, value) VALUES ($1, $2)"
            ),
            SQL_NTS));
    CHECK(SQLBindParameter(stmt_insert, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_id, 0, NULL));
    CHECK(SQLBindParameter(stmt_insert, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_value, 0, NULL));

    CHECK(SQLPrepare(stmt_select, get_str(
            "SELECT value FROM bench WHERE id=:id",
            "SELECT value FROM bench WHERE id=$1"),
            SQL_NTS));
    CHECK(SQLBindParameter(stmt_select, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_id, 0, NULL));
    CHECK(SQLBindCol(stmt_select, 1, SQL_C_SLONG, &_value, sizeof (_value), NULL));

    CHECK(SQLPrepare(stmt_update, get_str(
            "UPDATE bench SET value=:value WHERE id=:id",
            "UPDATE bench SET value=$1 WHERE id=$2"),
            SQL_NTS));
    CHECK(SQLBindParameter(stmt_update, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_value, 0, NULL));
    CHECK(SQLBindParameter(stmt_update, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_id, 0, NULL));

    CHECK(SQLPrepare(stmt_delete, get_str(
            "DELETE FROM bench WHERE id=:id",
            "DELETE FROM bench WHERE id=$1"),
            SQL_NTS));
    CHECK(SQLBindParameter(stmt_delete, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_id, 0, NULL));

    printf("Done!\n");
}

void insert(int32_t id, int32_t value) {
    _id = id;
    _value = value;
    CHECK_STMT(SQLExecute(stmt_insert), stmt_insert);
}

static void insert_func(uint64_t index) {
    insert(test_data[index].id, test_data[index].value);
}

void select(int32_t id, int32_t* value) {
    _id = id;
    _value = -1;
    CHECK_STMT(SQLExecute(stmt_select), stmt_select);
    CHECK_STMT(SQLFetch(stmt_select), stmt_select);
    CHECK_STMT(SQLFreeStmt(stmt_select, SQL_CLOSE), stmt_select);
    *value = _value;
}

static void select_func(uint64_t index) {
    int32_t value;
    select(test_data[index].id, &value);
    if (value != test_data[index].value) {
        printf("Invalid value selected!\n");
        exit(EXIT_FAILURE);
    }
}

void update(int32_t id, int32_t value) {
    _id = id;
    _value = value;
    CHECK_STMT(SQLExecute(stmt_update), stmt_update);
}

static void update_func(uint64_t index) {
    update(test_data[index].id, test_data[index].value + 1);
}

void del(int32_t id) {
    _id = id;
    CHECK_STMT(SQLExecute(stmt_delete), stmt_delete);
}

static void delete_func(uint64_t index) {
    del(test_data[index].id);
}

void disconnect() {
    CHECK(SQLDisconnect(hdbc));
    CHECK(SQLFreeConnect(hdbc));
    CHECK(SQLFreeEnv(henv));
    printf("Connection closed\n");
}

typedef void(*perf_func_t) (uint64_t index);

static void bench(perf_func_t f, uint64_t count, const char* operation) {
    int64_t start = getTimeMs();
    uint64_t step = count / 50;
    printf("    %s ", operation);
    fflush(stdout);
    for (uint64_t i = 0; i < count; i++) {
        f(i);
        if (i % step == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    int64_t delta = getTimeMs() - start;
    printf("\n    Execution time %ld ms, CPS=%ld, %ld operations\n", delta, count * ((int64_t) 1000) / delta, count);
}

static int compare_sort(const void* a, const void* b) {
    const test_data_t* pa = (const test_data_t*) a;
    const test_data_t* pb = (const test_data_t*) b;
    return pa->rnd_key - pb->rnd_key;
}

static void create_test_data(void) {
    printf("Create test data ... ");
    fflush(stdout);
    int i;
    for (i = 0; i < TEST_COUNT; i++) {
        test_data[i].id = i;
        test_data[i].value = rand();
        test_data[i].rnd_key = rand();
    }
    qsort(test_data, TEST_COUNT, sizeof (test_data[0]), compare_sort);
    printf("Done!\n");
}

static void reorder_test_data(void) {
    printf("Reorder test data ... ");
    fflush(stdout);
    int i;
    for (i = 0; i < TEST_COUNT; i++) {
        test_data[i].rnd_key = rand();
    }
    qsort(test_data, TEST_COUNT, sizeof (test_data[0]), compare_sort);
    printf("Done!\n");
}

int main(int argc, char** argv) {
#ifdef TT
    printf("Test TimesTen performance\n");
#endif
#ifdef PQ
    printf("Test PosgreSql performance\n");
#endif
    create_test_data();
    connect();
    //execute_direct("drop table bench");
    execute_direct(
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

    prepare();
    bench(insert_func, TEST_COUNT, "insert");
    reorder_test_data();
    bench(select_func, TEST_COUNT, "select");
    reorder_test_data();
    bench(update_func, TEST_COUNT, "update");
    reorder_test_data();
    bench(delete_func, TEST_COUNT, "delete");
    execute_direct("drop table bench");
    disconnect();
    return 0;
}

