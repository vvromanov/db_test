#ifndef TEST_DAO_H
#define	TEST_DAO_H
#include <stdint.h>

#define Q_INSERT get_str( \
            "INSERT INTO bench (id, value) VALUES (:id, :value)", \
            "INSERT INTO bench (id, value) VALUES ($1, $2)", \
            "INSERT INTO bench (id, value) VALUES (?, ?)" \
            )

#define Q_SELECT get_str( \
            "SELECT value FROM bench WHERE id=:id", \
            "SELECT value FROM bench WHERE id=$1", \
            "SELECT value FROM bench WHERE id=?" \
            )

#define Q_SELECT1 get_str( \
            "SELECT 1 from dual", \
            "SELECT 1", \
            "SELECT 1" \
            )

#define Q_UPDATE get_str( \
            "UPDATE bench SET value=:value WHERE id=:id", \
            "UPDATE bench SET value=$1 WHERE id=$2", \
            "UPDATE bench SET value=? WHERE id=?" \
            )

#define Q_DELETE get_str( \
            "DELETE FROM bench WHERE id=:id", \
            "DELETE FROM bench WHERE id=$1", \
            "DELETE FROM bench WHERE id=?" \
            )

void db_execute_direct(const char* sql);
void db_connect(void);
void db_disconnect(void);
void db_prepare(void);
void db_insert(int32_t id, int32_t value);
void db_select(int32_t id, int32_t* value);
void db_select1(int32_t* value);
void db_update(int32_t id, int32_t value);
void db_delete(int32_t id);

#endif	/* TEST_DAO_H */

