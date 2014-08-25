#include <stdlib.h>
#include <arpa/inet.h>
#include <libpq-fe.h>
#include "p_types.h"

#include "test_dao.h"
#include "test_utils.h"

static Oid oidTypes[] = {INT4OID, INT4OID};
static int paramLengths[] = {sizeof (int32_t), sizeof (int32_t)};
static int paramFormats[] = {1, 1};
static PGconn *conn = NULL;

#define STMT_ID_SELECT "pq_select"
#define STMT_ID_SELECT1 "pq_select1"
#define STMT_ID_INSERT "pq_insert"
#define STMT_ID_UPDATE "pq_update"
#define STMT_ID_DELETE "pq_delete"

#define CHECK_RESULT(res) \
     if (res == NULL) { \
        fprintf(stderr, "Error: %s\n", PQerrorMessage(conn)); \
        if (exit_on_error) { \
            exit(EXIT_FAILURE); \
        } \
    } \
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) { \
        fprintf(stderr, "Error %s\n", PQresultErrorMessage(res)); \
        if (exit_on_error) { \
            exit(EXIT_FAILURE); \
        } \
    }

void db_execute_direct(const char* sql) {
    printf("Execute direct [%s] ... ", sql);
    fflush(stdout);
    int64_t start = getTimeMs();
    PGresult * result = PQexec(conn, sql);
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error %s\n", PQresultErrorMessage(result));
        if (exit_on_error) {
            exit(EXIT_FAILURE);
        }
    }
    PQclear(result);
    int64_t delta = getTimeMs() - start;
    printf("Done! (%ld ms)\n", delta);
}

void db_connect() {
    conn = PQconnectdb("host=localhost dbname=test user=appuser password=password");
    //conn = PQconnectdb("host=/tmp dbname=test user=appuser password=password");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection error %s\n", PQerrorMessage(conn));
        exit(EXIT_FAILURE);
    }
    printf("Connection established\n");
}

void db_disconnect() {
    PQfinish(conn);
    conn = NULL;
    printf("Connection closed\n");
}

void db_prepare() {
    printf("Prepare ... ");
    fflush(stdout);

    PGresult* res = PQprepare(conn, STMT_ID_INSERT, Q_INSERT, 2, oidTypes);
    CHECK_RESULT(res);
    PQclear(res);

    res = PQprepare(conn, STMT_ID_SELECT, Q_SELECT, 1, oidTypes);
    CHECK_RESULT(res);
    PQclear(res);

    res = PQprepare(conn, STMT_ID_SELECT1, Q_SELECT1, 0, NULL);
    CHECK_RESULT(res);
    PQclear(res);

    res = PQprepare(conn, STMT_ID_UPDATE, Q_UPDATE, 2, oidTypes);
    CHECK_RESULT(res);
    PQclear(res);

    res = PQprepare(conn, STMT_ID_DELETE, Q_DELETE, 1, oidTypes);
    CHECK_RESULT(res);
    PQclear(res);

    printf("Done!\n");
}

void db_insert(int32_t id, int32_t value) {
    if (debug) {
        fprintf(stderr, "db_insert(%d,%d)\n", id, value);
    }
    int32_t _id = htonl(id);
    int32_t _value = htonl(value);
    const char* paramValues[] = {
        (const char*) &_id,
        (const char*) &_value,
    };
    PGresult* res = PQexecPrepared(conn,
            STMT_ID_INSERT,
            2,
            paramValues,
            paramLengths,
            paramFormats,
            1);
    CHECK_RESULT(res);
    PQclear(res);
}

void db_select(int32_t id, int32_t* value) {
    if (debug) {
        fprintf(stderr, "db_select(%d)\n", id);
    }
    int32_t _id = htonl(id);
    const char* paramValues[] = {
        (const char*) &_id,
    };
    PGresult* res = PQexecPrepared(conn,
            STMT_ID_SELECT,
            1,
            paramValues,
            paramLengths,
            paramFormats,
            1);
    CHECK_RESULT(res);
    *value = -1;
    if (PQntuples(res) == 0) {
        fprintf(stderr, "db_select(%d) return no rows\n", id);
        if (exit_on_error) {
            exit(EXIT_FAILURE);
        }
        PQclear(res);
    } else {
        *value = ntohl(*(int32_t *) PQgetvalue(res, 0, 0));
        if (debug) {
            fprintf(stderr, "db_select(%d) return %d\n", id, *value);
        }
    }
    PQclear(res);
}

void db_select1(int32_t* value) {
    if (debug) {
        fprintf(stderr, "db_select1()\n");
    }
    PGresult* res = PQexecPrepared(conn,
            STMT_ID_SELECT1,
            0,
            NULL,
            NULL,
            NULL,
            1);
    CHECK_RESULT(res);
    *value = -1;
    if (PQntuples(res) == 0) {
        fprintf(stderr, "db_select1() return no rows\n");
        if (exit_on_error) {
            exit(EXIT_FAILURE);
        }
        PQclear(res);
    } else {
        *value = ntohl(*(int32_t *) PQgetvalue(res, 0, 0));
        if (debug) {
            fprintf(stderr, "db_select1() return %d\n", *value);
        }
    }
    PQclear(res);
}

void db_update(int32_t id, int32_t value) {
    if (debug) {
        fprintf(stderr, "db_update(%d,%d)\n", id, value);
    }
    int32_t _id = htonl(id);
    int32_t _value = htonl(value);
    const char* paramValues[] = {
        (const char*) &_value,
        (const char*) &_id,
    };
    PGresult* res = PQexecPrepared(conn,
            STMT_ID_UPDATE,
            2,
            paramValues,
            paramLengths,
            paramFormats,
            1);
    CHECK_RESULT(res);
    PQclear(res);
}

void db_delete(int32_t id) {
    if (debug) {
        fprintf(stderr, "db_delete(%d)\n", id);
    }
    int32_t _id = htonl(id);
    const char* paramValues[] = {
        (const char*) &_id,
    };
    PGresult* res = PQexecPrepared(conn,
            STMT_ID_DELETE,
            1,
            paramValues,
            paramLengths,
            paramFormats,
            1);
    CHECK_RESULT(res);
    PQclear(res);
}

