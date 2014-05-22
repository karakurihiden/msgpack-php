#ifndef PHP_MSGPACK_SERIALIZE_H
#define PHP_MSGPACK_SERIALIZE_H

#include "ext/standard/php_smart_str.h"

typedef HashTable php_msgpack_serialize_data_t;

PHP_MSGPACK_API void php_msgpack_serialize(smart_str *buf, zval *struc, php_msgpack_serialize_data_t *data TSRMLS_DC);
PHP_MSGPACK_API void php_msgpack_serialize_extended(smart_str *buf, long type, char *str, int len);

#define PHP_MSGPACK_SERIALIZE_INIT(data) \
do  { \
    if (MSGPACK_G(lock) || !MSGPACK_G(serialize).level || !MSGPACK_G(serialize).data) { \
        HashTable h; \
        data = &h; \
        zend_hash_init(data, 10, NULL, NULL, 0); \
        if (!MSGPACK_G(lock)) { \
            MSGPACK_G(serialize).data = (void *)(data); \
            MSGPACK_G(serialize).level = 1; \
        } \
    } else { \
        (data) = (php_msgpack_serialize_data_t *)MSGPACK_G(serialize).data; \
        ++MSGPACK_G(serialize).level; \
    } \
} while(0)

#define PHP_MSGPACK_SERIALIZE_DESTROY(data) \
do { \
    if (MSGPACK_G(lock) || !MSGPACK_G(serialize).level) { \
        zend_hash_destroy(data); \
    } else { \
        if (!(--(MSGPACK_G(serialize).level))) { \
            zend_hash_destroy((php_msgpack_serialize_data_t *)MSGPACK_G(serialize).data); \
            MSGPACK_G(serialize).data = NULL; \
        } \
    } \
} while (0)

#endif  /* PHP_MSGPACK_SERIALIZE_H */
