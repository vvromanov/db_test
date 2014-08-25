#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "test_dao.h"
#include "test_utils.h"

#ifdef TT
#define TEST_COUNT 5000000
#endif
#ifdef PQ
#define TEST_COUNT 500000
#endif
#ifdef MY
#define TEST_COUNT 50000
#endif

typedef struct {
    int32_t id;
    int32_t value;
    int32_t rnd_key;
} test_data_t;

test_data_t test_data[TEST_COUNT];

static void insert_func(uint64_t index) {
    db_insert(test_data[index].id, test_data[index].value);
}

static void select_func(uint64_t index) {
    int32_t value;
    db_select(test_data[index].id, &value);
    if (value != test_data[index].value) {
        printf("Invalid value selected!\n");
        if (exit_on_error) exit(EXIT_FAILURE);
    }
}

static void update_func(uint64_t index) {
    db_update(test_data[index].id, test_data[index].value + 1);
}

static void delete_func(uint64_t index) {
    db_delete(test_data[index].id);
}

typedef void(*perf_func_t) (uint64_t index);

static void bench(perf_func_t f, uint64_t count, const char* operation, int32_t* cps) {
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
    *cps = count * ((int64_t) 1000) / delta;
    printf("\n    Execution time %ld ms, CPS=%d, %ld operations\n", delta, *cps, count);
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
    printf("Test %s performance\n", DB_NAME);
    create_test_data();
    db_connect();
#ifdef TT
    //db_execute_direct("drop table bench");
#else
    exit_on_error = false;
    db_execute_direct("drop table if exists bench");
    exit_on_error = true;
#endif
    db_execute_direct(
            get_str(
            "CREATE TABLE bench ("
            "id TT_INT NOT NULL, "
            "value TT_INT NOT NULL, "
            "PRIMARY KEY (id) "
            ")"
            " UNIQUE HASH ON (id) pages = 40000",

            "CREATE TABLE bench ("
            "id integer NOT NULL, "
            "value integer NOT NULL, "
            "CONSTRAINT bench_pk PRIMARY KEY (id) "
            ")",

            "CREATE TABLE IF NOT EXISTS bench ("
            "id integer NOT NULL, "
            "value integer NOT NULL, "
            "PRIMARY KEY (id)"
            ") ENGINE = MEMORY"
            )
            );

    db_prepare();
    int32_t insert_cps, select_cps, update_cps, delete_cps;
    bench(insert_func, TEST_COUNT, "insert", &insert_cps);
    reorder_test_data();
    bench(select_func, TEST_COUNT, "select", &select_cps);
    reorder_test_data();
    bench(update_func, TEST_COUNT, "update", &update_cps);
    reorder_test_data();
    bench(delete_func, TEST_COUNT, "delete", &delete_cps);
    db_execute_direct("drop table bench");
    db_disconnect();
    printf("%12s | %6s | %6s | %6s | %6s |\n", "Db Name", "insert", "select", "update", "delete");
    printf("%12s | %6d | %6d | %6d | %6d |\n", DB_NAME, insert_cps, select_cps, update_cps, delete_cps);
    return 0;
}

