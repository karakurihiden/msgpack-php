#ifndef PHP_MSGPACK_UNSERIALIZE_H
#define PHP_MSGPACK_UNSERIALIZE_H

#include "ext/standard/php_smart_str.h"

//#define PHP_MSGPACK_ENTRIES_MAX 1024
#define PHP_MSGPACK_ENTRIES_MAX 32

typedef struct {
    zval *data[PHP_MSGPACK_ENTRIES_MAX];
    long used_slots;
    void *next;
} php_msgpack_unserialize_entries_t;

struct php_msgpack_unserialize_data {
    void *first;
    void *last;
    smart_str *str;
};

typedef struct php_msgpack_unserialize_data php_msgpack_unserialize_data_t;

PHP_MSGPACK_API int php_msgpack_unserialize(zval **return_value, const unsigned char **p, const unsigned char *max, php_msgpack_unserialize_data_t *data, void *status TSRMLS_DC);

PHP_MSGPACK_API void php_msgpack_unserialize_destroy(php_msgpack_unserialize_data_t *data);

#define PHP_MSGPACK_UNSERIALIZE_INIT(data) \
do { \
    if (MSGPACK_G(lock) || !MSGPACK_G(unserialize).level || !MSGPACK_G(unserialize).data) { \
        php_msgpack_unserialize_data_t u; \
        smart_str str = { NULL, 0, 0 }; \
        php_msgpack_unserialize_entries_t *entries = emalloc(sizeof(php_msgpack_unserialize_entries_t)); \
        entries->used_slots = 0; \
        entries->next = NULL; \
        u.str = &str; \
        u.first = u.last = entries; \
        data = &u; \
        if (!MSGPACK_G(lock)) { \
           MSGPACK_G(unserialize).data = (void *)(data); \
           MSGPACK_G(unserialize).level = 1; \
        } \
    } else { \
        data = (php_msgpack_unserialize_data_t *)MSGPACK_G(unserialize).data; \
        ++MSGPACK_G(unserialize).level; \
    } \
} while (0)

#define PHP_MSGPACK_UNSERIALIZE_DESTROY(data) \
do { \
    if (MSGPACK_G(lock) || !MSGPACK_G(unserialize).level) { \
        smart_str_free((data)->str); \
        php_msgpack_unserialize_destroy(data);  \
    } else { \
        if (!--MSGPACK_G(unserialize).level) { \
            smart_str_free((data)->str); \
            php_msgpack_unserialize_destroy(data); \
            MSGPACK_G(unserialize).data = NULL; \
        } \
    } \
} while (0)

#endif  /* PHP_MSGPACK_UNSERIALIZE_H */
