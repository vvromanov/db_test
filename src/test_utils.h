#ifndef TEST_UTILS_H
#define	TEST_UTILS_H
#include <stdint.h>
#include <sys/time.h>

#ifdef	__cplusplus
extern "C" {
#endif

    static inline int64_t getTimeMs(void) {
        struct timeval val;
        gettimeofday(&val, NULL);
        return val.tv_sec * ((int64_t) 1000) + val.tv_usec / 1000;
    }

    static inline char* get_str(const char* tt_str, const char* pq_str, const char* my_str) {
#ifdef TT
        return const_cast<char*> (tt_str);
#endif
#ifdef PQ
        return const_cast<char*> (pq_str);
#endif
#ifdef MY
        return const_cast<char*> (my_str);
#endif
    }

#define DB_NAME get_str("TimesTen", "PostgreSql", "MySql")

#ifdef	__cplusplus
}
#endif

#endif	/* TEST_UTILS_H */

