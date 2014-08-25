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

#ifdef	__cplusplus
}
#endif

#endif	/* TEST_UTILS_H */

