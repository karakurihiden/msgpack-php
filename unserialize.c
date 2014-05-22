#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_incomplete_class.h"
#include "ext/standard/php_smart_str.h"
//#include "ext/standard/php_var.h"

#include "msgpack.h"
#include "php_msgpack.h"
#include "unserialize.h"
#include "extended.h"

ZEND_EXTERN_MODULE_GLOBALS(msgpack)

#if defined(_MSC_VER)
#define SWITCH_RANGE_BEGIN(byte)      if (0) {}
#define SWITCH_RANGE(byte, from, to)  else if (from <= byte && byte <= to)
#define SWITCH_RANGE_TO(byte, to)     else if (byte == to)
#define SWITCH_RANGE_WITH(byte, a, b) else if (byte == a || byte == b)
#define SWITCH_RANGE_DEFAULT()        else
#define SWITCH_RANGE_END()
#else
#define SWITCH_RANGE_BEGIN(byte)      switch (byte) {
#define SWITCH_RANGE(byte, from, to)  case from ... to:
#define SWITCH_RANGE_TO(byte, to)     case to:
#define SWITCH_RANGE_WITH(byte, a, b) case a: case b:
#define SWITCH_RANGE_DEFAULT()        default:
#define SWITCH_RANGE_END()            }
#endif

union php_msgpack_unserialize_cast_block_t {
    char buffer[8];
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    float f;
    double d;
};

typedef struct {
    smart_str *str;
    size_t begin;
    size_t size;
} php_msgpack_unserialize_str_t;

#define PHP_MSGPACK_UNSERIALIZE_PARAMETER zval **return_value, const unsigned char **p, const unsigned char *max, php_msgpack_unserialize_data_t *data, void *ext TSRMLS_DC
#define PHP_MSGPACK_UNSERIALIZE_PASSTHRU return_value, p, max, data, ext TSRMLS_CC

#define PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, c, n, p, l) \
    do { \
        if ((l-c) < n) { \
            return 0; \
        } \
        *p = c+n; \
        memcpy(cb.buffer, c, n); \
    } while(0)

#define PHP_MSGPACK_UNSERIALIZE_EXTENDED(c, n, p, l, r, d, e) \
    do { \
        int t; \
        if ((l-c) < (n+1)) { \
            return 0; \
        } \
        t = (int)*c; \
        *p = (++c); \
        if (!php_msgpack_unserialize_extended(r, p, c+n, d, e TSRMLS_CC, t)) { \
            return 0; \
        } \
    } while (0)

#define PHP_MSGPACK_UNSERIALIZE_STR_INIT(s, x) \
    do { \
        s.str = (x); \
        s.begin = (x)->len; \
        s.size = 0; \
    } while (0)

#define PHP_MSGPACK_UNSERIALIZE_STR_0(s) \
    do { \
        s.size = s.str->len - s.begin; \
        smart_str_0(s.str); \
        ++(s.str->len); \
    } while (0)

#define PHP_MSGPACK_UNSERIALIZE_STR_DATA(s) s.str->c + s.begin

#define PHP_MSGPACK_UNSERIALIZE_STR_SIZE(s) s.size

#define PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(s) s.str->len = s.begin

#define PHP_MSGPACK_UNSERIALIZE_STR_FREE(s) smart_str_free(s.str)

static inline void
php_msgpack_unserialize_push(php_msgpack_unserialize_data_t *data,
                             zval **return_value)
{
    php_msgpack_unserialize_entries_t *entries;

    if (!data) {
        return;
    }

    entries = data->last;

    if (!entries || entries->used_slots == PHP_MSGPACK_ENTRIES_MAX) {
        entries = emalloc(sizeof(php_msgpack_unserialize_entries_t));
        entries->used_slots = 0;
        entries->next = NULL;

        if (!(data->first)) {
            data->first = entries;
        } else {
            ((php_msgpack_unserialize_entries_t *)(data->last))->next = entries;
        }

        data->last = entries;
    }

    entries->data[entries->used_slots++] = *return_value;
}

static inline int
php_msgpack_unserialize_access(php_msgpack_unserialize_data_t *data,
                               long id, zval ***store)
{
    php_msgpack_unserialize_entries_t *entries = data->first;

    while (id >= PHP_MSGPACK_ENTRIES_MAX && entries &&
           entries->used_slots == PHP_MSGPACK_ENTRIES_MAX) {
        entries = entries->next;
        id -= PHP_MSGPACK_ENTRIES_MAX;
    }

    if (!entries) {
        return !SUCCESS;
    }

    if (id < 0 || id >= entries->used_slots) {
        return !SUCCESS;
    }

    *store = &entries->data[id];

    return SUCCESS;
}

static inline int
php_msgpack_unserialize_long(PHP_MSGPACK_UNSERIALIZE_PARAMETER, long *num)
{
    const unsigned char *cursor, *limit;
    unsigned char b;

    limit = max;
    cursor = *p;

    if (cursor >= limit) {
        return 0;
    }

    b = *cursor;

    SWITCH_RANGE_BEGIN(b)

    /* positive fixint */
    SWITCH_RANGE(b, 0x00, 0x7f)
    {
        *num = *(uint8_t *)cursor;
        *p = (++cursor);
        return 1;
    }
    /* negative fixint */
    SWITCH_RANGE(b, 0xe0, 0xff)
    {
        *num = *(int8_t *)cursor;
        *p = (++cursor);
        return 1;
    }
    /* uint 8 */
    SWITCH_RANGE_TO(b, 0xcc)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        *num = cb.u8;
        return 1;
    }
    /* uint 16 */
    SWITCH_RANGE_TO(b, 0xcd)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        *num = msgpack_be16(cb.u16);
        return 1;
    }
    /* uint 32 */
    SWITCH_RANGE_TO(b, 0xce)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        *num = msgpack_be32(cb.u32);
        return 1;
    }
    /* uint 64 */
    SWITCH_RANGE_TO(b, 0xcf)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 8, p, limit);
        cb.u64 = msgpack_be64(cb.u64);
        if (cb.u64 <= LONG_MAX) {
            *num = cb.u64;
        } else {
            int n;
            char *s;
            if ((n = asprintf(&s, "%llu", (unsigned long long)cb.u64)) < 0) {
                return 0;
            }
            MSGPACK_ERR(E_WARNING, "Illegal number, exceed INT_MAX");
            *num = 0;
            free(s);
        }
        return 1;
    }
    /* int 8 */
    SWITCH_RANGE_TO(b, 0xd0)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        *num = cb.i8;
        return 1;
    }
    /* int 16 */
    SWITCH_RANGE_TO(b, 0xd1)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        *num = (int16_t)msgpack_be16(cb.i16);
        return 1;
    }
    /* int 32 */
    SWITCH_RANGE_TO(b, 0xd2)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        *num = (int32_t)msgpack_be32(cb.i32);
        return 1;
    }
    /* int 64 */
    SWITCH_RANGE_TO(b, 0xd3)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 8, p, limit);
        *num = (uint64_t)msgpack_be64(cb.i64);
        return 1;
    }
    /* default */
    SWITCH_RANGE_DEFAULT()
    {
        return 0;
    }

    SWITCH_RANGE_END()

    return 0;
}

static inline int
php_msgpack_unserialize_string(PHP_MSGPACK_UNSERIALIZE_PARAMETER,
                               smart_str *str)
{
    const unsigned char *cursor, *limit;
    unsigned char b;

    limit = max;
    cursor = *p;

    if (cursor >= limit) {
        return 0;
    }

    b = *cursor;

    SWITCH_RANGE_BEGIN(b)

    /* fixstr */
    SWITCH_RANGE(b, 0xa0, 0xbf)
    {
        int count = b & 0x1f;
        *p = (++cursor);
        if (count == 0 || *p > max) {
            return 1;
        }
        *p += count;
        if (*p > max) {
            return 1;
        }
        smart_str_appendl(str, (char *)cursor, count);
        return 1;
    }
    /* str 8, bin 8 */
    SWITCH_RANGE_WITH(b, 0xd9, 0xc4)
    {
        uint8_t count;
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        count = cb.u8;
        if (count == 0) {
            return 1;
        }
        cursor = *p;
        *p += count;
        if (*p > max) {
            return 1;
        }
        smart_str_appendl(str, (char *)cursor, count);
        return 1;
    }
    /* str 16, bin 16 */
    SWITCH_RANGE_WITH(b, 0xda, 0xc5)
    {
        uint16_t count;
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        count = msgpack_be16(cb.u16);
        if (count == 0) {
            return 1;
        }
        cursor = *p;
        *p += count;
        if (*p > max) {
            return 1;
        }
        smart_str_appendl(str, (char *)cursor, count);
        return 1;
    }
    /* str 32, bin 32 */
    SWITCH_RANGE_WITH(b, 0xdb, 0xc6)
    {
        uint32_t count;
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        count = msgpack_be32(cb.u32);
        if (count == 0) {
            return 1;
        }
        cursor = *p;
        *p += count;
        if (*p > max) {
            return 1;
        }
        smart_str_appendl(str, (char *)cursor, count);
        return 1;
    }
    /* default */
    SWITCH_RANGE_DEFAULT()
    {
        return 0;
    }

    SWITCH_RANGE_END()

    return 0;
}

static inline int
php_msgpack_unserialize_key(PHP_MSGPACK_UNSERIALIZE_PARAMETER,
                            long *index, smart_str *str)
{
    const unsigned char *cursor, *limit;
    unsigned char b;

    limit = max;
    cursor = *p;

    if (cursor >= limit) {
        return 0;
    }

    b = *cursor;

    SWITCH_RANGE_BEGIN(b)

    /* positive fixint */
    SWITCH_RANGE(b, 0x00, 0x7f)
    {
        *index = *(uint8_t *)cursor;
        *p = (++cursor);
        return IS_LONG;
    }
    /* negative fixint */
    SWITCH_RANGE(b, 0xe0, 0xff)
    {
        *index = *(int8_t *)cursor;
        *p = (++cursor);
        return IS_LONG;
    }
    /* fixstr */
    SWITCH_RANGE(b, 0xa0, 0xbf)
    {
        int count = b & 0x1f;
        *p = (++cursor);
        if (count == 0 || *p > max) {
            return IS_STRING;
        }
        *p += count;
        if (*p > max) {
            return IS_STRING;
        }
        smart_str_appendl(str, (char *)cursor, count);
        return IS_STRING;
    }
    /* uint 8 */
    SWITCH_RANGE_TO(b, 0xcc)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        *index = cb.u8;
        return IS_LONG;
    }
    /* uint 16 */
    SWITCH_RANGE_TO(b, 0xcd)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        *index = msgpack_be16(cb.u16);
        return IS_LONG;
    }
    /* uint 32 */
    SWITCH_RANGE_TO(b, 0xce)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        *index = msgpack_be32(cb.u32);
        return IS_LONG;
    }
    /* uint 64 */
    SWITCH_RANGE_TO(b, 0xcf)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 8, p, limit);
        cb.u64 = msgpack_be64(cb.u64);
        if (cb.u64 <= LONG_MAX) {
            *index = cb.u64;
        } else {
            int n;
            char *s;
            if ((n = asprintf(&s, "%llu", (unsigned long long)cb.u64)) < 0) {
                return 0;
            }
            MSGPACK_ERR(E_WARNING, "Convert string, exceed INT_MAX");
            smart_str_appendl(str, s, strlen(s));
            free(s);
            return IS_STRING;
        }
        return IS_LONG;
    }
    /* int 8 */
    SWITCH_RANGE_TO(b, 0xd0)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        *index = cb.i8;
        return IS_LONG;
    }
    /* int 16 */
    SWITCH_RANGE_TO(b, 0xd1)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        *index = (int16_t)msgpack_be16(cb.i16);
        return IS_LONG;
    }
    /* int 32 */
    SWITCH_RANGE_TO(b, 0xd2)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        *index = (int32_t)msgpack_be32(cb.i32);
        return IS_LONG;
    }
    /* int 64 */
    SWITCH_RANGE_TO(b, 0xd3)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 8, p, limit);
        *index = (uint64_t)msgpack_be64(cb.i64);
        return IS_LONG;
    }
    /* str 8, bin 8 */
    SWITCH_RANGE_WITH(b, 0xd9, 0xc4)
    {
        uint8_t count;
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        count = cb.u8;
        if (count == 0) {
            return IS_STRING;
        }
        cursor = *p;
        *p += count;
        if (*p > max) {
            return IS_STRING;
        }
        smart_str_appendl(str, (char *)cursor, count);
        return IS_STRING;
    }
    /* str 16, bin 16 */
    SWITCH_RANGE_WITH(b, 0xda, 0xc5)
    {
        uint16_t count;
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        count = msgpack_be16(cb.u16);
        if (count == 0) {
            return IS_STRING;
        }
        cursor = *p;
        *p += count;
        if (*p > max) {
            return IS_STRING;
        }
        smart_str_appendl(str, (char *)cursor, count);
        return IS_STRING;
    }
    /* str 32, bin 32 */
    SWITCH_RANGE_WITH(b, 0xdb, 0xc6)
    {
        uint32_t count;
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        count = msgpack_be32(cb.u32);
        if (count == 0) {
            return IS_STRING;
        }
        cursor = *p;
        *p += count;
        if (*p > max) {
            return IS_STRING;
        }
        smart_str_appendl(str, (char *)cursor, count);
        return IS_STRING;
    }
    /* default */
    SWITCH_RANGE_DEFAULT()
    {
        return 0;
    }

    SWITCH_RANGE_END()

    return 0;
}

static inline int
php_msgpack_unserialize_nested_array(PHP_MSGPACK_UNSERIALIZE_PARAMETER,
                                     int count)
{
    array_init_size(*return_value, count);

    while (count-- > 0) {
        zval *val;
        ALLOC_INIT_ZVAL(val);
        /* php_msgpack_unserialize_push(data, &val); */
        if (!php_msgpack_unserialize(&val, p, max, data, NULL TSRMLS_CC)) {
            FREE_ZVAL(val);
            return 0;
        }

        zend_hash_next_index_insert(Z_ARRVAL_PP(return_value),
                                    &val, sizeof(val), NULL);
    }

    return 1;
}

static inline int
php_msgpack_unserialize_nested_map(PHP_MSGPACK_UNSERIALIZE_PARAMETER,
                                   int count)
{
    array_init_size(*return_value, count);

    while (count-- > 0) {
        zval *val;
        long index = 0;
        int type;
        php_msgpack_unserialize_str_t key;

        /* key */
        PHP_MSGPACK_UNSERIALIZE_STR_INIT(key, data->str);
        type = php_msgpack_unserialize_key(NULL, p, max, data, NULL TSRMLS_CC,
                                           &index, key.str);
        if (!type) {
            PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(key);
            return 0;
        }

        /* value */
        ALLOC_INIT_ZVAL(val);
        if (!php_msgpack_unserialize(&val, p, max, data, NULL TSRMLS_CC)) {
            PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(key);
            FREE_ZVAL(val);
            return 0;
        }

        if (type == IS_STRING) {
            PHP_MSGPACK_UNSERIALIZE_STR_0(key);
            zend_symtable_update(Z_ARRVAL_PP(return_value),
                                 PHP_MSGPACK_UNSERIALIZE_STR_DATA(key),
                                 PHP_MSGPACK_UNSERIALIZE_STR_SIZE(key) + 1,
                                 &val, sizeof(val), NULL);
            PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(key);
        } else {
            zend_hash_index_update(Z_ARRVAL_PP(return_value), index,
                                   &val, sizeof(val), NULL);
        }
    }

    return 1;
}

static inline zend_class_entry *
php_msgpack_unserialize_lookup_class(char *name, size_t len TSRMLS_DC)
{
    zend_class_entry *ce, **pce;

    if (zend_lookup_class(name, len, &pce TSRMLS_CC) == SUCCESS) {
        ce = *pce;
    } else if ((PG(unserialize_callback_func) == NULL) ||
               (PG(unserialize_callback_func)[0] == '\0')) {
        /* None unserialize_callback_func */
        ce = NULL;
    } else {
        zval func, **args[1], *tmp, *zv;
        INIT_PZVAL(&func);
        ZVAL_STRING(&func, PG(unserialize_callback_func), 0);
        MAKE_STD_ZVAL(zv);
        ZVAL_STRINGL(zv, name, len, 1);
        args[0] = &zv;
        MSGPACK_G(lock)++;
        if (call_user_function_ex(EG(function_table), NULL,
                                  &func, &tmp, 1, args, 0,
                                  NULL TSRMLS_CC) == SUCCESS) {
            /* The callback function may have defined the class */
            if (zend_lookup_class(name, len, &pce TSRMLS_CC) == SUCCESS) {
                ce = *pce;
            } else {
                MSGPACK_ERR(E_WARNING, "Function %s() hasn't defined "
                            "the class it was called for", Z_STRVAL_P(&func));
                ce = NULL;
            }
        } else {
            MSGPACK_ERR(E_WARNING, "defined (%s) but not found",
                        Z_STRVAL_P(&func));
            ce = NULL;
        }
        MSGPACK_G(lock)--;
        if (tmp) {
            zval_ptr_dtor(&tmp);
        }
        zval_ptr_dtor(&zv);
    }

    return ce;
}

static inline int
php_msgpack_unserialize_extended(PHP_MSGPACK_UNSERIALIZE_PARAMETER, int type)
{
    if (*p >= max) {
        return 0;
    }

    if (type == MSGPACK_G(type.class)) {
        zend_class_entry *ce;
        zend_bool incomplete_class = 0;
        long count = 0;
        php_msgpack_unserialize_str_t name;

        /* class name */
        PHP_MSGPACK_UNSERIALIZE_STR_INIT(name, data->str);
        if (!php_msgpack_unserialize_string(NULL, p, max, NULL, NULL TSRMLS_CC,
                                            name.str)) {
            MSGPACK_ERR(E_WARNING, "extended class name\n");
            PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(name);
            return 0;
        }
        PHP_MSGPACK_UNSERIALIZE_STR_0(name);

        ce = php_msgpack_unserialize_lookup_class(
            PHP_MSGPACK_UNSERIALIZE_STR_DATA(name),
            PHP_MSGPACK_UNSERIALIZE_STR_SIZE(name) TSRMLS_CC);
        if (EG(exception)) {
            PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(name);
            return 0;
        }
        if (ce == NULL) {
            incomplete_class = 1;
            ce = PHP_IC_ENTRY;
        }

        INIT_PZVAL(*return_value);
        object_init_ex(*return_value, ce);
        php_msgpack_unserialize_push(data, return_value);

        if (incomplete_class) {
            php_store_class_name(*return_value,
                                 PHP_MSGPACK_UNSERIALIZE_STR_DATA(name),
                                 PHP_MSGPACK_UNSERIALIZE_STR_SIZE(name));
            //MSGPACK_ERR(E_WARNING, "Incomplete class\n");
        }

        PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(name);

        /* property count */
        if (!php_msgpack_unserialize_long(NULL, p, max, NULL, NULL TSRMLS_CC,
                                          &count)) {
            MSGPACK_ERR(E_WARNING, "class property count\n");
            return 0;
        }

        /* property */
        while (count-- > 0) {
            long type = 0;
            php_msgpack_unserialize_str_t key;
            zval *val;

            /* type */
            const unsigned char *cursor = *p;
            unsigned char b = *cursor;
            if (0x00 <= b && b <= 0x7f) {
                type = (int)*cursor;
                *p = (++cursor);
            }

            /* name */
            PHP_MSGPACK_UNSERIALIZE_STR_INIT(key, data->str);
            if (type == MSGPACK_EXTENDED_PROTECTED) {
                /* protected */
                smart_str_appendc(key.str, 0x00);
                smart_str_appendc(key.str, 0x2a);
                smart_str_appendc(key.str, 0x00);
            } else if (type == MSGPACK_EXTENDED_PRIVATE) {
                /* private */
                smart_str_appendc(key.str, 0x00);
                smart_str_appendl(key.str, ce->name, ce->name_length);
                smart_str_appendc(key.str, 0x00);
            }
            if (!php_msgpack_unserialize_string(NULL, p, max,
                                                NULL, NULL TSRMLS_CC,
                                                key.str)) {
                PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(key);
                return 0;
            }
            PHP_MSGPACK_UNSERIALIZE_STR_0(key);

            /* value */
            ALLOC_INIT_ZVAL(val);
            if (!php_msgpack_unserialize(&val, p, max, data, NULL TSRMLS_CC)) {
                PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(key);
                FREE_ZVAL(val);
                return 0;
            }

            if (PHP_MSGPACK_UNSERIALIZE_STR_SIZE(key) == 0) {
                MSGPACK_ERR(E_NOTICE, "Illegal member variable name");
                zval_dtor(val);
                FREE_ZVAL(val);
            } else {
                zend_hash_update(Z_OBJPROP_PP(return_value),
                                 PHP_MSGPACK_UNSERIALIZE_STR_DATA(key),
                                 PHP_MSGPACK_UNSERIALIZE_STR_SIZE(key) + 1,
                                 &val, sizeof(val), NULL);
            }

            PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(key);
        }
        *p = max;

        if (!incomplete_class &&
            zend_hash_exists(&Z_OBJCE_PP(return_value)->function_table,
                             "__wakeup", sizeof("__wakeup"))) {
            zval func, *tmp = NULL;
            INIT_PZVAL(&func);
            ZVAL_STRINGL(&func, "__wakeup", sizeof("__wakeup") - 1, 0);
            MSGPACK_G(lock)++;
            call_user_function_ex(CG(function_table), return_value, &func,
                                  &tmp, 0, 0, 1, NULL TSRMLS_CC);
            MSGPACK_G(lock)--;

            if (tmp) {
                zval_ptr_dtor(&tmp);
            }

            if (EG(exception)) {
                return 0;
            }
        }

        return 1;
    } else if (type == MSGPACK_G(type.serializable)) {
        zval *zv, func, *tmp, **args[1];
        zend_class_entry *ce;
        zend_bool incomplete_class = 0;
        php_msgpack_unserialize_str_t name;

        /* class name */
        PHP_MSGPACK_UNSERIALIZE_STR_INIT(name, data->str);
        if (!php_msgpack_unserialize_string(NULL, p, max, NULL, NULL TSRMLS_CC,
                                            name.str)) {
            MSGPACK_ERR(E_WARNING, "extended serializable class name\n");
            PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(name);
            return 0;
        }
        PHP_MSGPACK_UNSERIALIZE_STR_0(name);

        ce = php_msgpack_unserialize_lookup_class(
            PHP_MSGPACK_UNSERIALIZE_STR_DATA(name),
            PHP_MSGPACK_UNSERIALIZE_STR_SIZE(name) TSRMLS_CC);
        if (EG(exception)) {
            PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(name);
            return 0;
        }
        if (ce == NULL) {
            incomplete_class = 1;
            ce = PHP_IC_ENTRY;
        }

        INIT_PZVAL(*return_value);
        object_init_ex(*return_value, ce);
        php_msgpack_unserialize_push(data, return_value);

        if (incomplete_class) {
            php_store_class_name(*return_value,
                                 PHP_MSGPACK_UNSERIALIZE_STR_DATA(name),
                                 PHP_MSGPACK_UNSERIALIZE_STR_SIZE(name));
            //MSGPACK_ERR(E_WARNING, "Incomplete class\n");
        }

        PHP_MSGPACK_UNSERIALIZE_STR_DESTROY(name);

        /* serialized data */
        ALLOC_INIT_ZVAL(zv);
        if (!php_msgpack_unserialize(&zv, p, max, NULL, NULL TSRMLS_CC)) {
            MSGPACK_ERR(E_WARNING, "extended serializable data\n");
            FREE_ZVAL(zv);
            *p = max;
            return 1;
        }

        if (ce->unserialize == NULL) {
            MSGPACK_ERR(E_WARNING, "Class %s has no unserializer", ce->name);
            zval_dtor(zv);
            FREE_ZVAL(zv);
            *p = max;
            return 1;
        }

        args[0] = &zv;

        INIT_PZVAL(&func);
        ZVAL_STRINGL(&func, "unserialize", sizeof("unserialize") - 1, 0);
        if (call_user_function_ex(EG(function_table), return_value, &func,
                                  &tmp, 1, args, 0, NULL TSRMLS_CC) != SUCCESS ||
            EG(exception)) {
            MSGPACK_ERR(E_WARNING, "invalid unserializer");
        }
        zval_ptr_dtor(&tmp);

        zval_dtor(zv);
        FREE_ZVAL(zv);

        *p = max;
        return 1;
    } else if (type == MSGPACK_G(type.reference_type)) {
        /* Reference(R) */
        zval **tmp;
        long num = 0;
        //int format
        if (!php_msgpack_unserialize_long(NULL, p, max, NULL, NULL TSRMLS_CC,
                                          &num)) {
            MSGPACK_ERR(E_WARNING, "reference number\n");
            return 0;
        }

        if (php_msgpack_unserialize_access(data, num, &tmp) != SUCCESS) {
            MSGPACK_ERR(E_WARNING, "reference type: %ld\n", num);
            return 0;
        }

        if (*return_value != NULL) {
            zval_ptr_dtor(return_value);
        }

        *return_value = *tmp;
        Z_ADDREF_PP(return_value);
        Z_SET_ISREF_PP(return_value);

        return 1;
    } else if (type == MSGPACK_G(type.reference_value)) {
        /* Reference(r) */
        zval **tmp;
        long num = 0;
        //int format
        if (!php_msgpack_unserialize_long(NULL, p, max, NULL, NULL TSRMLS_CC,
                                          &num)) {
            MSGPACK_ERR(E_WARNING, "reference number\n");
            return 0;
        }

        if (php_msgpack_unserialize_access(data, num, &tmp) != SUCCESS) {
            MSGPACK_ERR(E_WARNING, "reference value: %ld\n", num);
            return 0;
        }

        if (*return_value != NULL) {
            zval_ptr_dtor(return_value);
        }

        *return_value = *tmp;
        Z_ADDREF_PP(return_value);
        Z_UNSET_ISREF_PP(return_value);

        return 1;
    } else if (type == MSGPACK_G(type.reference)) {
        /* Reference */
        if (!php_msgpack_unserialize(return_value, p, max,
                                     NULL, NULL TSRMLS_CC)) {
            MSGPACK_ERR(E_WARNING, "reference\n");
            return 0;
        }
        php_msgpack_unserialize_push(data, return_value);
        return 1;
    } else {
        php_msgpack_extended_data_t *tmp;
        zval *zv, *retval = NULL;
        zval **args[1];

        if (zend_hash_index_find(MSGPACK_G(extended).unserializer,
                                 type, (void **)&tmp) != SUCCESS) {
            MSGPACK_ERR(E_WARNING, "not found unserializer type: %d", type);
            return 0;
        }

        MAKE_STD_ZVAL(zv);
        ZVAL_STRINGL(zv, (char *)*p, max - *p, 1);

        args[0] = &zv;

        if (call_user_function_ex(EG(function_table), NULL, tmp->function,
                                  &retval, 1, args, 0,
                                  NULL TSRMLS_CC) != SUCCESS) {
            MSGPACK_ERR(E_WARNING,
                        "failed unserializer call function: %d\n", type);
            zval_ptr_dtor(&zv);
            return 0;
        }

        if (retval) {
            INIT_PZVAL_COPY(*return_value, retval);
            zval_copy_ctor(*return_value);
            /* php_msgpack_unserialize_push(data, return_value); */
            zval_ptr_dtor(&retval);
        }

        zval_ptr_dtor(&zv);

        *p = max;
        return 1;
    }
}

/*
PHP_MSGPACK_API int
php_msgpack_unserialize(zval **return_value,
                        const unsigned char **p, const unsigned char *max,
                        php_msgpack_unserialize_data_t *data,
                        void *ext TSRMLS_DC)
*/
PHP_MSGPACK_API int
php_msgpack_unserialize(PHP_MSGPACK_UNSERIALIZE_PARAMETER)
{
    const unsigned char *cursor, *limit;
    unsigned char b;

    limit = max;
    cursor = *p;

    if (cursor >= limit) {
        return 0;
    }

    b = *cursor;

    SWITCH_RANGE_BEGIN(b)

    /* positive fixint */
    SWITCH_RANGE(b, 0x00, 0x7f)
    {
        INIT_PZVAL(*return_value);
        ZVAL_LONG(*return_value, *(uint8_t *)cursor);
        *p = (++cursor);
        return 1;
    }
    /* negative fixint */
    SWITCH_RANGE(b, 0xe0, 0xff)
    {
        INIT_PZVAL(*return_value);
        ZVAL_LONG(*return_value, *(int8_t *)cursor);
        *p = (++cursor);
        return 1;
    }
    /* fixstr */
    SWITCH_RANGE(b, 0xa0, 0xbf)
    {
        int count = b & 0x1f;
        *p = (++cursor);
        INIT_PZVAL(*return_value);
        if (count == 0) {
            ZVAL_EMPTY_STRING(*return_value);
            return 1;
        }
        *p += count;
        if (*p > max) {
            ZVAL_EMPTY_STRING(*return_value);
            return 1;
        }
        ZVAL_STRINGL(*return_value, (char *)cursor, count, 1);
        return 1;
    }
    /* fixarray */
    SWITCH_RANGE(b, 0x90, 0x9f)
    {
        int count = b & 0x0f;
        *p = (++cursor);
        if (count < 0) {
            return 0;
        }
        INIT_PZVAL(*return_value);
        php_msgpack_unserialize_push(data, return_value);
        return php_msgpack_unserialize_nested_array(
            PHP_MSGPACK_UNSERIALIZE_PASSTHRU, count);
    }
    /* fixmap */
    SWITCH_RANGE(b, 0x80, 0x8f)
    {
        int count = b & 0x0f;
        *p = (++cursor);
        if (count < 0) {
            return 0;
        }
        INIT_PZVAL(*return_value);
        php_msgpack_unserialize_push(data, return_value);
        return php_msgpack_unserialize_nested_map(
            PHP_MSGPACK_UNSERIALIZE_PASSTHRU, count);
    }
    /* nil */
    SWITCH_RANGE_TO(b, 0xc0)
    {
        INIT_PZVAL(*return_value);
        ZVAL_NULL(*return_value);
        *p = (++cursor);
        return 1;
    }
    /* false */
    SWITCH_RANGE_TO(b, 0xc2)
    {
        INIT_PZVAL(*return_value);
        ZVAL_BOOL(*return_value, 0);
        *p = (++cursor);
        return 1;
    }
    /* true */
    SWITCH_RANGE_TO(b, 0xc3)
    {
        INIT_PZVAL(*return_value);
        ZVAL_BOOL(*return_value, 1);
        *p = (++cursor);
        return 1;
    }
    /* float 32 */
    SWITCH_RANGE_TO(b, 0xca)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        cb.u32 = msgpack_be32(cb.u32);
        INIT_PZVAL(*return_value);
        ZVAL_DOUBLE(*return_value, cb.f);
        return 1;
    }
    /* double / float 64 */
    SWITCH_RANGE_TO(b, 0xcb)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 8, p, limit);
        cb.u64 = msgpack_be64(cb.u64);
        INIT_PZVAL(*return_value);
        ZVAL_DOUBLE(*return_value, cb.d);
        return 1;
    }
    /* uint 8 */
    SWITCH_RANGE_TO(b, 0xcc)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        INIT_PZVAL(*return_value);
        ZVAL_LONG(*return_value, cb.u8);
        return 1;
    }
    /* uint 16 */
    SWITCH_RANGE_TO(b, 0xcd)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        INIT_PZVAL(*return_value);
        ZVAL_LONG(*return_value, msgpack_be16(cb.u16));
        return 1;
    }
    /* uint 32 */
    SWITCH_RANGE_TO(b, 0xce)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        INIT_PZVAL(*return_value);
        ZVAL_LONG(*return_value, msgpack_be32(cb.u32));
        return 1;
    }
    /* uint 64 */
    SWITCH_RANGE_TO(b, 0xcf)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 8, p, limit);
        cb.u64 = msgpack_be64(cb.u64);
        if (cb.u64 <= LONG_MAX) {
            INIT_PZVAL(*return_value);
            ZVAL_LONG(*return_value, cb.u64);
        } else {
            int n;
            char *s;
            if ((n = asprintf(&s, "%llu", (unsigned long long)cb.u64)) < 0) {
                return 0;
            }
            MSGPACK_ERR(E_WARNING, "Convert string, exceed INT_MAX");
            INIT_PZVAL(*return_value);
            ZVAL_STRINGL(*return_value, s, n, 1);
            free(s);
        }
        return 1;
    }
    /* int 8 */
    SWITCH_RANGE_TO(b, 0xd0)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        INIT_PZVAL(*return_value);
        ZVAL_LONG(*return_value, cb.i8);
        return 1;
    }
    /* int 16 */
    SWITCH_RANGE_TO(b, 0xd1)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        INIT_PZVAL(*return_value);
        ZVAL_LONG(*return_value, (int16_t)msgpack_be16(cb.i16));
        return 1;
    }
    /* int 32 */
    SWITCH_RANGE_TO(b, 0xd2)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        INIT_PZVAL(*return_value);
        ZVAL_LONG(*return_value, (int32_t)msgpack_be32(cb.i32));
        return 1;
    }
    /* int 64 */
    SWITCH_RANGE_TO(b, 0xd3)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 8, p, limit);
        INIT_PZVAL(*return_value);
        ZVAL_LONG(*return_value, (uint64_t)msgpack_be64(cb.i64));
        return 1;
    }
    /* str 8, bin 8 */
    SWITCH_RANGE_WITH(b, 0xd9, 0xc4)
    {
        uint8_t count;
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        count = cb.u8;
        INIT_PZVAL(*return_value);
        if (count == 0) {
            ZVAL_EMPTY_STRING(*return_value);
            return 1;
        }
        cursor = *p;
        *p += count;
        if (*p > max) {
            ZVAL_EMPTY_STRING(*return_value);
            return 1;
        }
        ZVAL_STRINGL(*return_value, (char *)cursor, count, 1);
        return 1;
    }
    /* str 16, bin 16 */
    SWITCH_RANGE_WITH(b, 0xda, 0xc5)
    {
        uint16_t count;
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        count = msgpack_be16(cb.u16);
        INIT_PZVAL(*return_value);
        if (count == 0) {
            ZVAL_EMPTY_STRING(*return_value);
            return 1;
        }
        cursor = *p;
        *p += count;
        if (*p > max) {
            ZVAL_EMPTY_STRING(*return_value);
            return 1;
        }
        ZVAL_STRINGL(*return_value, (char *)cursor, count, 1);
        return 1;
    }
    /* str 32, bin 32 */
    SWITCH_RANGE_WITH(b, 0xdb, 0xc6)
    {
        uint32_t count;
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        count = msgpack_be32(cb.u32);
        INIT_PZVAL(*return_value);
        if (count == 0) {
            ZVAL_EMPTY_STRING(*return_value);
            return 1;
        }
        cursor = *p;
        *p += count;
        if (*p > max) {
            ZVAL_EMPTY_STRING(*return_value);
            return 1;
        }
        ZVAL_STRINGL(*return_value, (char *)cursor, count, 1);
        return 1;
    }
    /* array 16 */
    SWITCH_RANGE_TO(b, 0xdc)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        INIT_PZVAL(*return_value);
        php_msgpack_unserialize_push(data, return_value);
        return php_msgpack_unserialize_nested_array(
            PHP_MSGPACK_UNSERIALIZE_PASSTHRU, (uint16_t)msgpack_be16(cb.u16));
    }
    /* array 32 */
    SWITCH_RANGE_TO(b, 0xdd)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        INIT_PZVAL(*return_value);
        php_msgpack_unserialize_push(data, return_value);
        return php_msgpack_unserialize_nested_array(
            PHP_MSGPACK_UNSERIALIZE_PASSTHRU, (uint32_t)msgpack_be32(cb.u32));
    }
    /* map 16 */
    SWITCH_RANGE_TO(b, 0xde)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        INIT_PZVAL(*return_value);
        php_msgpack_unserialize_push(data, return_value);
        return php_msgpack_unserialize_nested_map(
            PHP_MSGPACK_UNSERIALIZE_PASSTHRU, (uint16_t)msgpack_be16(cb.u16));
    }
    /* map 32 */
    SWITCH_RANGE_TO(b, 0xdf)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        INIT_PZVAL(*return_value);
        php_msgpack_unserialize_push(data, return_value);
        return php_msgpack_unserialize_nested_map(
            PHP_MSGPACK_UNSERIALIZE_PASSTHRU, (uint32_t)msgpack_be32(cb.u32));
    }
    /* fixext 1 */
    SWITCH_RANGE_TO(b, 0xd4)
    {
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_EXTENDED(cursor, 1, p, limit,
                                         return_value, data, ext);
        return 1;
    }
    /* fixext 2 */
    SWITCH_RANGE_TO(b, 0xd5)
    {
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_EXTENDED(cursor, 2, p, limit,
                                         return_value, data, ext);
        return 1;
    }
    /* fixext 4 */
    SWITCH_RANGE_TO(b, 0xd6)
    {
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_EXTENDED(cursor, 4, p, limit,
                                         return_value, data, ext);
        return 1;
    }
    /* fixext 8 */
    SWITCH_RANGE_TO(b, 0xd7)
    {
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_EXTENDED(cursor, 8, p, limit,
                                         return_value, data, ext);
        return 1;
    }
    /* fixext 16 */
    SWITCH_RANGE_TO(b, 0xd8)
    {
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_EXTENDED(cursor, 16, p, limit,
                                         return_value, data, ext);
        return 1;
    }
    /* ext 8 */
    SWITCH_RANGE_TO(b, 0xc7)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        uint8_t count;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 1, p, limit);
        cursor = *p;
        count = cb.u8;
        PHP_MSGPACK_UNSERIALIZE_EXTENDED(cursor, count, p, limit,
                                         return_value, data, ext);
        return 1;
    }
    /* ext 16 */
    SWITCH_RANGE_TO(b, 0xc8)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        uint16_t count;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 2, p, limit);
        cursor = *p;
        count = msgpack_be16(cb.u16);
        PHP_MSGPACK_UNSERIALIZE_EXTENDED(cursor, count, p, limit,
                                         return_value, data, ext);
        return 1;
    }
    /* ext 32 */
    SWITCH_RANGE_TO(b, 0xc9)
    {
        union php_msgpack_unserialize_cast_block_t cb;
        uint32_t count;
        cursor++;
        PHP_MSGPACK_UNSERIALIZE_CAST_BLOCK(cb, cursor, 4, p, limit);
        cursor = *p;
        count = msgpack_be32(cb.u32);
        PHP_MSGPACK_UNSERIALIZE_EXTENDED(cursor, count, p, limit,
                                         return_value, data, ext);
        return 1;
    }
    /* default */
    SWITCH_RANGE_DEFAULT()
    {
        MSGPACK_ERR(E_WARNING, "invalid byte: %x", b);
        return 0;
    }

    SWITCH_RANGE_END()

    return 0;
}

PHP_MSGPACK_API void
php_msgpack_unserialize_destroy(php_msgpack_unserialize_data_t *data)
{
    void *next;
    php_msgpack_unserialize_entries_t *entries = data->first;

    while (entries) {
        next = entries->next;
        efree(entries);
        entries = next;
    }
}
