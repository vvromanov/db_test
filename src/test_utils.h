#ifndef TEST_UTILS_H
#define	TEST_UTILS_H
#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>

extern bool exit_on_error;
extern bool debug;

#ifdef	__cplusplus
extern "C" {
#endif

    static inline int64_t getTimeMs(void) {
        struct timeval val;
        gettimeofday(&val, NULL);
        return val.tv_sec * ((int64_t) 1000) + val.tv_usec / 1000;
    }

    static inline char* get_str(const char* tt_str, const char* pq_str, const char* my_str, const char* libpq_str = NULL) {
#ifdef TT
        return const_cast<char*> (tt_str);
#endif
#ifdef PQ
        return const_cast<char*> (pq_str);
#endif
#ifdef MY
        return const_cast<char*> (my_str);
#endif
#ifdef LIBPQ
        if (libpq_str) {
            return const_cast<char*> (libpq_str);
        } else {
            return const_cast<char*> (pq_str);
        }
#endif
    }

#define DB_NAME get_str("TimesTen (ODBC)", "PostgreSql (ODBC)", "MySql (ODBC)", "PostgreSql (libpq)")

#ifdef	__cplusplus
}
#endif

#endif	/* TEST_UTILS_H */

