#ifndef TEST_DAO_H
#define	TEST_DAO_H

extern bool exit_on_error;
extern bool debug;

void db_execute_direct(const char* sql);
void db_connect(void);
void db_disconnect(void);
void db_prepare(void);
void db_insert(int32_t id, int32_t value);
void db_select(int32_t id, int32_t* value);
void db_update(int32_t id, int32_t value);
void db_delete(int32_t id);

#endif	/* TEST_DAO_H */

