#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "zend_exceptions.h"
#include "ext/standard/php_incomplete_class.h"
//#include "ext/standard/php_var.h"

#include "msgpack.h"
#include "php_msgpack.h"
#include "serialize.h"
#include "extended.h"

ZEND_EXTERN_MODULE_GLOBALS(msgpack)

static inline void
php_msgpack_serialize_nil(smart_str *buf)
{
    smart_str_appendc(buf, 0xc0);
}

static inline void
php_msgpack_serialize_bool(smart_str *buf, zend_bool val)
{
    if (val) {
        smart_str_appendc(buf, 0xc3);
    } else {
        smart_str_appendc(buf, 0xc2);
    }
}

static inline void
php_msgpack_serialize_write_fixint(smart_str *buf, int8_t val)
{
    smart_str_appendc(buf, val);
}

static inline void
php_msgpack_serialize_write_uint8(smart_str *buf, uint8_t val)
{
    smart_str_appendc(buf, 0xcc);
    smart_str_appendc(buf, val);
}

static inline void
php_msgpack_serialize_write_uint16(smart_str *buf, uint16_t val)
{
    uint16_t be = msgpack_be16(val);
    smart_str_appendc(buf, 0xcd);
    smart_str_appendl(buf, (const void *)&be, 2);
}

static inline void
php_msgpack_serialize_write_uint32(smart_str *buf, uint32_t val)
{
    uint32_t be = msgpack_be32(val);
    smart_str_appendc(buf, 0xce);
    smart_str_appendl(buf, (const void *)&be, 4);
}

static inline void
php_msgpack_serialize_write_uint64(smart_str *buf, uint64_t val)
{
    uint64_t be = msgpack_be64(val);
    smart_str_appendc(buf, 0xcf);
    smart_str_appendl(buf, (const void *)&be, 8);
}

static inline void
php_msgpack_serialize_write_int8(smart_str *buf, int8_t val)
{
    smart_str_appendc(buf, 0xd0);
    smart_str_appendc(buf, val);
}

static inline void
php_msgpack_serialize_write_int16(smart_str *buf, int16_t val)
{
    uint16_t be = msgpack_be16(val);
    smart_str_appendc(buf, 0xd1);
    smart_str_appendl(buf, (const void *)&be, 2);
}

static inline void
php_msgpack_serialize_write_int32(smart_str *buf, int32_t val)
{
    uint32_t be = msgpack_be32(val);
    smart_str_appendc(buf, 0xd2);
    smart_str_appendl(buf, (const void *)&be, 4);
}

static inline void
php_msgpack_serialize_write_int64(smart_str *buf, int64_t val)
{
    uint64_t be = msgpack_be64(val);
    smart_str_appendc(buf, 0xd3);
    smart_str_appendl(buf, (const void *)&be, 8);
}

static inline void
php_msgpack_serialize_long_32(smart_str *buf, long val)
{
    if (val < -0x20L) {
        if (val < -0x8000L) {
            php_msgpack_serialize_write_int32(buf, (int32_t)val);
        } else if (val < -0x80L) {
            php_msgpack_serialize_write_int16(buf, (int16_t)val);
        } else {
            php_msgpack_serialize_write_int8(buf, (int8_t)val);
        }
    } else if (val <= 0x7fL) {
        php_msgpack_serialize_write_fixint(buf, (int8_t)val);
    } else {
        if (val <= 0xffL) {
            php_msgpack_serialize_write_uint8(buf, (uint8_t)val);
        } else if (val <= 0xffffL) {
            php_msgpack_serialize_write_uint16(buf, (uint16_t)val);
        } else {
            php_msgpack_serialize_write_uint32(buf, (uint32_t)val);
        }
    }
}

static inline void
php_msgpack_serialize_long_64(smart_str *buf, long long val)
{
    if (val < -0x20LL) {
        if (val < -0x8000LL) {
            if (val < -0x80000000LL) {
                php_msgpack_serialize_write_int64(buf, (int64_t)val);
            } else {
                php_msgpack_serialize_write_int32(buf, (int32_t)val);
            }
        } else {
            if (val < -0x80LL) {
                php_msgpack_serialize_write_int16(buf, (int16_t)val);
            } else {
                php_msgpack_serialize_write_int8(buf, (int8_t)val);
            }
        }
    } else if (val <= 0x7fLL) {
        php_msgpack_serialize_write_fixint(buf, (int8_t)val);
    } else {
        if (val <= 0xffffLL) {
            if (val <= 0xffLL) {
                php_msgpack_serialize_write_uint8(buf, (uint8_t)val);
            } else {
                php_msgpack_serialize_write_uint16(buf, (uint16_t)val);
            }
        } else {
            if (val <= 0xffffffffLL) {
                php_msgpack_serialize_write_uint32(buf, (uint32_t)val);
            } else {
                php_msgpack_serialize_write_uint64(buf, (uint64_t)val);
            }
        }
    }
}

static inline void
php_msgpack_serialize_long(smart_str *buf, long val)
{
#if defined(SIZEOF_LONG)
#    if SIZEOF_LONG <= 4
    php_msgpack_serialize_long_32(buf, val);
#    else
    php_msgpack_serialize_long_64(buf, val);
#    endif
#elif defined(LONG_MAX)
#    if LONG_MAX <= 0x7fffffffL
    php_msgpack_serialize_long_32(buf, val);
#    else
    php_msgpack_serialize_long_64(buf, val);
#    endif
#else
    if (sizeof(long) <= 4) {
        php_msgpack_serialize_32(buf, val);
    } else {
        php_msgpack_serialize_64(buf, val);
    }
#endif
}

static inline void
php_msgpack_serialize_long_long(smart_str *buf, long long val)
{
    php_msgpack_serialize_long_64(buf, val);
}

static inline void
php_msgpack_serialize_uint64(smart_str *buf, uint64_t val)
{
    if (val <= 0xffULL) {
        if (val <= 0x7fULL) {
            php_msgpack_serialize_write_fixint(buf, (int8_t)val);
        } else {
            php_msgpack_serialize_write_uint8(buf, (uint8_t)val);
        }
    } else {
        if (val <= 0xffffULL) {
            php_msgpack_serialize_write_uint16(buf, (uint16_t)val);
        } else if (val <= 0xffffffffULL) {
            php_msgpack_serialize_write_uint32(buf, (uint32_t)val);
        } else {
            php_msgpack_serialize_write_uint64(buf, (uint64_t)val);
        }
    }
}

static inline void
php_msgpack_serialize_double(smart_str *buf, double val)
{
    //float 64 (double)
    union { double d; uint64_t i; } mem = { val };
    uint64_t be = msgpack_be64(mem.i);
    smart_str_appendc(buf, 0xcb);
    smart_str_appendl(buf, (const void *)&be, 8);

    /*
    //float 32
    union { float d; uint32_t i; } mem = { val };
    uint32_t be = msgpack_be32(mem.i);
    smart_str_appendc(buf, 0xca);
    smart_str_appendl(buf, (const void *)&be, 4);
    */
}

static inline void
php_msgpack_serialize_write_raw_header(smart_str *buf, size_t len)
{
    if (len < 32) {
        unsigned char h = 0xa0 | (uint8_t)len;
        smart_str_appendc(buf, h);
    } else if (len < 256) {
        smart_str_appendc(buf, 0xd9);
        smart_str_appendc(buf, (uint8_t)len);
    } else if (len < 65536) {
        uint16_t be = msgpack_be16(len);
        smart_str_appendc(buf, 0xda);
        smart_str_appendl(buf, (const void *)&be, 2);
    } else {
        uint32_t be = msgpack_be32(len);
        smart_str_appendc(buf, 0xdb);
        smart_str_appendl(buf, (const void *)&be, 4);
    }
}

static inline void
php_msgpack_serialize_string(smart_str *buf, char *str, size_t len)
{
    php_msgpack_serialize_write_raw_header(buf, len);
    smart_str_appendl(buf, str, len);
}

static inline void
php_msgpack_serialize_write_array_header(smart_str *buf, size_t n)
{
    //php_printf(">> array: %d\n", n);
    if (n < 16) {
        unsigned char h = 0x90 | (uint8_t)n;
        smart_str_appendc(buf, h);
    } else if (n < 65536) {
        uint16_t be = msgpack_be16(n);
        smart_str_appendc(buf, 0xdc);
        smart_str_appendl(buf, (const void *)&be, 2);
    } else {
        uint32_t be = msgpack_be32(n);
        smart_str_appendc(buf, 0xdd);
        smart_str_appendl(buf, (const void *)&be, 4);
    }
}

static inline void
php_msgpack_serialize_array(smart_str *buf, zval *val, HashTable *data TSRMLS_DC)
{
    zval **tmp;
    ulong i = 0, n = 0;

    n = zend_hash_num_elements(HASH_OF(val));
    if (n <= 0) {
        php_msgpack_serialize_write_array_header(buf, 0);
        return;
    }

    php_msgpack_serialize_write_array_header(buf, n);

    while (i < n) {
        if (zend_hash_index_find(HASH_OF(val), i, (void **)&tmp) != SUCCESS ||
            !tmp || tmp == &val ||
            (Z_TYPE_PP(tmp) == IS_ARRAY && Z_ARRVAL_PP(tmp)->nApplyCount > 1)) {
            php_msgpack_serialize_nil(buf);
        } else {
            if (Z_TYPE_PP(tmp) == IS_ARRAY) {
                Z_ARRVAL_PP(tmp)->nApplyCount++;
            }
            php_msgpack_serialize(buf, *tmp, data TSRMLS_CC);
            if (Z_TYPE_PP(tmp) == IS_ARRAY) {
                Z_ARRVAL_PP(tmp)->nApplyCount--;
            }
        }
        ++i;
    }
}

static inline void
php_msgpack_serialize_write_map_header(smart_str *buf, unsigned int n)
{
    //php_printf(">> map: %d\n", n);
    if (n < 16) {
        unsigned char h = 0x80 | (uint8_t)n;
        smart_str_appendc(buf, h);
    } else if (n < 65536) {
        uint16_t be = msgpack_be16(n);
        smart_str_appendc(buf, 0xde);
        smart_str_appendl(buf, (const void *)&be, 2);
    } else {
        uint32_t be = msgpack_be32(n);
        smart_str_appendc(buf, 0xdf);
        smart_str_appendl(buf, (const void *)&be, 4);
    }
}

static inline void
php_msgpack_serialize_map(smart_str *buf, zval *val, HashTable *data TSRMLS_DC)
{
    ulong i, n = 0;
    char *key;
    zval **tmp;
    uint key_len;
    HashPosition pos;

    n = zend_hash_num_elements(HASH_OF(val));
    if (n <= 0) {
        php_msgpack_serialize_write_map_header(buf, 0);
        return;
    }

    php_msgpack_serialize_write_map_header(buf, n);

    zend_hash_internal_pointer_reset_ex(HASH_OF(val), &pos);
    for (;; zend_hash_move_forward_ex(HASH_OF(val), &pos)) {
        n = zend_hash_get_current_key_ex(HASH_OF(val), &key, &key_len,
                                         &i, 0, &pos);
        if (n == HASH_KEY_IS_LONG) {
            php_msgpack_serialize_long(buf, i);
        } else if (n == HASH_KEY_IS_STRING) {
            php_msgpack_serialize_string(buf, key, key_len - 1);
        } else {
            break;
        }

        if (zend_hash_get_current_data_ex(HASH_OF(val),
                                          (void **)&tmp, &pos) != SUCCESS ||
            !tmp || tmp == &val ||
            (Z_TYPE_PP(tmp) == IS_ARRAY && Z_ARRVAL_PP(tmp)->nApplyCount > 1)) {
            php_msgpack_serialize_nil(buf);
        } else {
            if (Z_TYPE_PP(tmp) == IS_ARRAY) {
                Z_ARRVAL_PP(tmp)->nApplyCount++;
            }
            php_msgpack_serialize(buf, *tmp, data TSRMLS_CC);
            if (Z_TYPE_PP(tmp) == IS_ARRAY) {
                Z_ARRVAL_PP(tmp)->nApplyCount--;
            }
        }
    }
}

static inline zend_bool
php_msgpack_serialize_class_name(smart_str *buf, zval *val TSRMLS_DC)
{
    PHP_CLASS_ATTRIBUTES;

    PHP_SET_CLASS_ATTRIBUTES(val);
    php_msgpack_serialize_string(buf, class_name, name_len);
    PHP_CLEANUP_CLASS_ATTRIBUTES();

    return incomplete_class;
}

static inline void
php_msgpack_serialize_class_property(smart_str *buf, zend_class_entry *ce,
                                     zval *val, HashTable *data TSRMLS_DC)
{
    smart_str ext = { NULL, 0, 0 };
    zend_bool incomplete_class = 0;
    ulong n = 0;

    incomplete_class = php_msgpack_serialize_class_name(&ext, val TSRMLS_CC);

    n = zend_hash_num_elements(Z_OBJPROP_P(val));
    if (incomplete_class) {
        --n;
    }

    if (n > 0) {
        ulong i;
        char *key;
        zval **tmp;
        uint key_len;
        HashPosition pos;

        php_msgpack_serialize_long(&ext, n);

        zend_hash_internal_pointer_reset_ex(Z_OBJPROP_P(val), &pos);
        for (;; zend_hash_move_forward_ex(Z_OBJPROP_P(val), &pos)) {
            n = zend_hash_get_current_key_ex(Z_OBJPROP_P(val), &key, &key_len,
                                             &i, 0, &pos);
            if (n == HASH_KEY_NON_EXISTENT) {
                break;
            }

            if (n != HASH_KEY_IS_STRING ||
                (incomplete_class && strcmp(key, MAGIC_MEMBER) == 0)) {
                php_msgpack_serialize_write_fixint(&ext, 0);
                php_msgpack_serialize_string(&ext, "", 0);
                php_msgpack_serialize_nil(&ext);
                continue;
            }

            if (key_len > 3 && key[0] == '\0') {
                char *prop_name;
                uint prop_len;

                if (key[1] == '*') {
                    /* protected */
                    php_msgpack_serialize_write_fixint(
                        &ext, MSGPACK_EXTENDED_PROTECTED);
                    prop_name = key + 3;
                    prop_len = key_len - 4;
                } else if (strncmp(key + 1, ce->name, ce->name_length) == 0) {
                    /* private */
                    php_msgpack_serialize_write_fixint(
                        &ext, MSGPACK_EXTENDED_PRIVATE);
                    prop_name = key + ce->name_length + 2;
                    prop_len = key_len - ce->name_length - 3;
                } else {
                    /* public */
                    prop_name = key;
                    prop_len = key_len - 1;
                }
                php_msgpack_serialize_string(&ext, prop_name, prop_len);
            } else {
                /* public */
                php_msgpack_serialize_string(&ext, key, key_len - 1);
            }

            if (zend_hash_get_current_data_ex(Z_OBJPROP_P(val),
                                              (void **)&tmp, &pos) != SUCCESS ||
                !tmp || tmp == &val ||
                (Z_TYPE_PP(tmp) == IS_ARRAY &&
                 Z_ARRVAL_PP(tmp)->nApplyCount > 1)) {
                php_msgpack_serialize_nil(&ext);
            } else {
                if (Z_TYPE_PP(tmp) == IS_ARRAY) {
                    Z_ARRVAL_PP(tmp)->nApplyCount++;
                }
                php_msgpack_serialize(&ext, *tmp, data TSRMLS_CC);
                if (Z_TYPE_PP(tmp) == IS_ARRAY) {
                    Z_ARRVAL_PP(tmp)->nApplyCount--;
                }
            }
        }
    } else {
        php_msgpack_serialize_long(&ext, 0);
    }

    php_msgpack_serialize_extended(buf, MSGPACK_G(type.class), ext.c, ext.len);
    smart_str_free(&ext);
}

static inline void
php_msgpack_serialize_class_sleep(smart_str *buf, zend_class_entry *ce,
                                  zval *val, zval *props,
                                  HashTable *data TSRMLS_DC)
{
    smart_str ext = { NULL, 0, 0 };
    zend_bool incomplete_class = 0;
    zval **key;
    HashPosition pos;
    ulong n = 0;

    incomplete_class = php_msgpack_serialize_class_name(&ext, val TSRMLS_CC);

    n = zend_hash_num_elements(HASH_OF(props));
    if (n > 0) {
        php_msgpack_serialize_long(&ext, n);

        zend_hash_internal_pointer_reset_ex(HASH_OF(props), &pos);
        while (zend_hash_get_current_data_ex(HASH_OF(props),
                                             (void **)&key, &pos) == SUCCESS) {
            char *prop_name = NULL;
            int prop_len = 0;
            zval **tmp = NULL;

            if (Z_TYPE_PP(key) != IS_STRING) {
                MSGPACK_ERR(E_NOTICE, "__sleep should return an array only "
                            "containing the names of instance-variables "
                            "to serialize.");
                php_msgpack_serialize_string(&ext, "", 0);
                php_msgpack_serialize_nil(&ext);
                zend_hash_move_forward_ex(HASH_OF(props), &pos);
                continue;
            }

            if (incomplete_class &&
                strncmp(Z_STRVAL_PP(key), MAGIC_MEMBER, Z_STRLEN_PP(key)) == 0) {
                php_msgpack_serialize_string(&ext, "", 0);
                php_msgpack_serialize_nil(&ext);
                zend_hash_move_forward_ex(HASH_OF(props), &pos);
                continue;
            }

            do {
                /* public */
                if (zend_hash_find(Z_OBJPROP_P(val),
                                   Z_STRVAL_PP(key), Z_STRLEN_PP(key) + 1,
                                   (void **)&tmp) == SUCCESS) {
                    php_msgpack_serialize_string(&ext,
                                                 Z_STRVAL_PP(key),
                                                 Z_STRLEN_PP(key));
                    break;
                }

                /* protected */
                zend_mangle_property_name(&prop_name, &prop_len, "*", 1,
                                          Z_STRVAL_PP(key), Z_STRLEN_PP(key),
                                          ce->type & ZEND_INTERNAL_CLASS);
                if (zend_hash_find(Z_OBJPROP_P(val),
                                   prop_name, prop_len + 1,
                                   (void **)&tmp) == SUCCESS) {
                    php_msgpack_serialize_write_fixint(
                        &ext, MSGPACK_EXTENDED_PROTECTED);
                    php_msgpack_serialize_string(&ext, prop_name, prop_len);
                    break;
                }
                pefree(prop_name, ce->type & ZEND_INTERNAL_CLASS);

                /* private */
                zend_mangle_property_name(&prop_name, &prop_len,
                                          ce->name, ce->name_length,
                                          Z_STRVAL_PP(key), Z_STRLEN_PP(key),
                                          ce->type & ZEND_INTERNAL_CLASS);
                if (zend_hash_find(Z_OBJPROP_P(val),
                                   prop_name, prop_len + 1,
                                   (void **)&tmp) == SUCCESS) {
                    php_msgpack_serialize_write_fixint(
                        &ext, MSGPACK_EXTENDED_PRIVATE);
                    php_msgpack_serialize_string(&ext, prop_name, prop_len);
                    break;
                }
                pefree(prop_name, ce->type & ZEND_INTERNAL_CLASS);

                prop_name = NULL;
                tmp = NULL;

                /* property does not exist */
                MSGPACK_ERR(E_NOTICE, "\"%s\" returned as member variable "
                            "from __sleep() but does not exist",
                            Z_STRVAL_PP(key));
                php_msgpack_serialize_string(&ext,
                                             Z_STRVAL_PP(key),
                                             Z_STRLEN_PP(key));
            } while (0);

            if (prop_name) {
                pefree(prop_name, ce->type & ZEND_INTERNAL_CLASS);
            }

            if (tmp) {
                php_msgpack_serialize(&ext, *tmp, data TSRMLS_CC);
            } else {
                php_msgpack_serialize_nil(&ext);
            }

            zend_hash_move_forward_ex(HASH_OF(props), &pos);
        }
    } else {
        php_msgpack_serialize_long(&ext, 0);
    }

    php_msgpack_serialize_extended(buf, MSGPACK_G(type.class), ext.c, ext.len);
    smart_str_free(&ext);
}

static inline void
php_msgpack_serialize_object(smart_str *buf, zval *val,
                             HashTable *data TSRMLS_DC)
{
    zend_class_entry *ce = NULL;
    zval *retval = NULL;
    zval func;
    int rc;

    if (Z_OBJ_HT_P(val)->get_class_entry) {
        ce = Z_OBJCE_P(val);
    }

    if (!ce) {
        php_msgpack_serialize_map(buf, val, data TSRMLS_CC);
        return;
    }

    if (ce->serialize != NULL) {
        INIT_PZVAL(&func);
        ZVAL_STRINGL(&func, "serialize", sizeof("serialize") - 1, 0);
        if (call_user_function_ex(EG(function_table), &val, &func, &retval,
                                  0, NULL, 0, NULL TSRMLS_CC) == SUCCESS) {
            if (!retval || EG(exception)) {
            }

            if (Z_TYPE_P(retval) == IS_STRING) {
                smart_str ext = { NULL, 0, 0 };
                //php_printf(">> obj[0]\n");
                php_msgpack_serialize_string(&ext,
                                             (char *)ce->name, ce->name_length);
                php_msgpack_serialize_string(&ext,
                                             Z_STRVAL_P(retval),
                                             Z_STRLEN_P(retval));
                php_msgpack_serialize_extended(buf, MSGPACK_G(type.serializable),
                                               ext.c, ext.len);
                smart_str_free(&ext);
            } else if (Z_TYPE_P(retval) == IS_NULL) {
                php_msgpack_serialize_nil(buf);
            } else {
                if (retval) {
                    zval_ptr_dtor(&retval);
                }
                zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
                                        0 TSRMLS_CC, "%s::serialize() "
                                        "must return a string or NULL",
                                        (char *)ce->name);
                return;
            }
        } else {
            //php_printf(">> obj[2]\n");
            php_msgpack_serialize_nil(buf);
        }

        if (retval) {
            zval_ptr_dtor(&retval);
        }

        return;
    }

    if (ce != PHP_IC_ENTRY &&
        zend_hash_exists(&ce->function_table, "__sleep", sizeof("__sleep"))) {
        INIT_PZVAL(&func);
        ZVAL_STRINGL(&func, "__sleep", sizeof("__sleep") - 1, 0);
        MSGPACK_G(lock)++;
        rc = call_user_function_ex(CG(function_table), &val, &func,
                                   &retval, 0, 0, 1, NULL TSRMLS_CC);
        MSGPACK_G(lock)--;

        if (EG(exception)) {
            if (retval) {
                zval_ptr_dtor(&retval);
            }
            //php_printf(">> obj[3]\n");
            return;
        }

        if (rc == SUCCESS) {
            if (retval) {
                if (HASH_OF(retval)) {
                    //php_printf(">> obj[4]\n");
                    php_msgpack_serialize_class_sleep(buf, ce, val, retval,
                                                      data TSRMLS_CC);
                } else {
                    MSGPACK_ERR(E_NOTICE, "__sleep should return an array "
                                "only containing the names of "
                                "instance-variables to serialize");
                    //php_printf(">> obj[5]\n");
                    php_msgpack_serialize_nil(buf);
                }
                zval_ptr_dtor(&retval);
            }
            return;
        }
    }

    if (retval) {
        zval_ptr_dtor(&retval);
    }

    php_msgpack_serialize_class_property(buf, ce, val, data TSRMLS_CC);
}

static inline int
php_msgpack_add_data(HashTable *data, zval *val, void *old TSRMLS_DC)
{
    ulong no;
    char id[32], *p;
    register int len;

    if (Z_TYPE_P(val) == IS_OBJECT) {
        if (Z_OBJ_HT_P(val)->get_class_entry) {
            p = smart_str_print_long(id + sizeof(id) - 1,
                                     (long)zend_objects_get_address(val
                                                                    TSRMLS_CC));
            len = id + sizeof(id) - 1 - p;
        } else {
            p = smart_str_print_long(id + sizeof(id) - 1, (long)val);
            len = id + sizeof(id) - 1 - p;
        }
    } else {
        p = smart_str_print_long(id + sizeof(id) - 1, (long)val);
        len = id + sizeof(id) - 1 - p;
    }

    if (old && zend_hash_find(data, p, len, old) == SUCCESS) {
        if (!Z_ISREF_P(val)) {
            no = -1;
            zend_hash_next_index_insert(data, &no, sizeof(no), NULL);
        }
        return FAILURE;
    }

    no = zend_hash_num_elements(data);
    zend_hash_add(data, p, len, &no, sizeof(no), NULL);

    return SUCCESS;
}

PHP_MSGPACK_API void
php_msgpack_serialize(smart_str *buf, zval *struc, HashTable *data TSRMLS_DC)
{
    ulong *already = NULL;
    smart_str ext = { NULL, 0, 0 };

    if (EG(exception)) {
        return;
    }

    if (Z_ISREF_P(struc)) {
        if (data &&
            php_msgpack_add_data(data, struc,
                                 (void *)&already TSRMLS_CC) == FAILURE) {
            /* Reference(R) */
            php_msgpack_serialize_long(&ext, (long)*already);
            php_msgpack_serialize_extended(buf, MSGPACK_G(type.reference_type),
                                           ext.c, ext.len);
            smart_str_free(&ext);
            return;
        }

        switch (Z_TYPE_P(struc)) {
            case IS_NULL:
                php_msgpack_serialize_nil(&ext);
                break;
            case IS_BOOL:
                php_msgpack_serialize_bool(&ext, Z_BVAL_P(struc));
                break;
            case IS_LONG:
                php_msgpack_serialize_long(&ext, Z_LVAL_P(struc));
                break;
            case IS_DOUBLE:
                php_msgpack_serialize_double(&ext, Z_DVAL_P(struc));
                break;
            case IS_STRING:
                php_msgpack_serialize_string(&ext,
                                             Z_STRVAL_P(struc),
                                             Z_STRLEN_P(struc));
                break;
            case IS_ARRAY:
                php_msgpack_serialize_map(buf, struc, data TSRMLS_CC);
                return;
            case IS_OBJECT:
                php_msgpack_serialize_object(buf, struc, data TSRMLS_CC);
                return;
            default:
                php_msgpack_serialize_long(&ext, 0);
                break;
        }
        /* Reference */
        php_msgpack_serialize_extended(buf, MSGPACK_G(type.reference),
                                       ext.c, ext.len);
        smart_str_free(&ext);
        return;
    }

    switch (Z_TYPE_P(struc)) {
        case IS_NULL:
            php_msgpack_serialize_nil(buf);
            return;
        case IS_BOOL:
            php_msgpack_serialize_bool(buf, Z_BVAL_P(struc));
            return;
        case IS_LONG:
            php_msgpack_serialize_long(buf, Z_LVAL_P(struc));
            return;
        case IS_DOUBLE:
            php_msgpack_serialize_double(buf, Z_DVAL_P(struc));
            return;
        case IS_STRING:
            php_msgpack_serialize_string(buf,
                                         Z_STRVAL_P(struc),
                                         Z_STRLEN_P(struc));
            return;
        case IS_ARRAY:
            //php_msgpack_serialize_array(buf, struc, data TSRMLS_CC);
            if (data &&
                php_msgpack_add_data(data, struc,
                                     (void *)&already TSRMLS_CC) == FAILURE) {
                /* Reference(r) */
                php_msgpack_serialize_long(&ext, (long)*already);
                php_msgpack_serialize_extended(buf,
                                               MSGPACK_G(type.reference_value),
                                               ext.c, ext.len);
                smart_str_free(&ext);
                return;
            }
            php_msgpack_serialize_map(buf, struc, data TSRMLS_CC);
            return;
        case IS_OBJECT:
            if (data &&
                php_msgpack_add_data(data, struc,
                                     (void *)&already TSRMLS_CC) == FAILURE) {
                /* Reference(r) */
                php_msgpack_serialize_long(&ext, (long)*already);
                php_msgpack_serialize_extended(buf,
                                               MSGPACK_G(type.reference_value),
                                               ext.c, ext.len);
                smart_str_free(&ext);
                return;
            }
            php_msgpack_serialize_object(buf, struc, data TSRMLS_CC);
            return;
        default:
            php_msgpack_serialize_long(buf, 0);
            return;
    }
}

PHP_MSGPACK_API void
php_msgpack_serialize_extended(smart_str *buf, long type, char *str, int len)
{
    int n = 0;

    if (len <= 1) {
        smart_str_appendc(buf, 0xd4);
        smart_str_appendc(buf, (unsigned char)type);
        n = 1;
    } else if (len <= 2) {
        smart_str_appendc(buf, 0xd5);
        smart_str_appendc(buf, (unsigned char)type);
        n = 2;
    } else if (len == 4) { /* len <= 4 */
        smart_str_appendc(buf, 0xd6);
        smart_str_appendc(buf, (unsigned char)type);
        n = 4;
    } else if (len == 8) { /* len <= 8 */
        smart_str_appendc(buf, 0xd7);
        smart_str_appendc(buf, (unsigned char)type);
        n = 8;
    } else if (len == 16) { /* len <= 16 */
        smart_str_appendc(buf, 0xd8);
        smart_str_appendc(buf, (unsigned char)type);
        n = 16;
    } else if (len < 256) {
        smart_str_appendc(buf, 0xc7);
        smart_str_appendc(buf, (uint8_t)len);
        smart_str_appendc(buf, (unsigned char)type);
    } else if (len < 65536) {
        uint16_t be = msgpack_be16(len);
        smart_str_appendc(buf, 0xc8);
        smart_str_appendl(buf, (const void *)&be, 2);
        smart_str_appendc(buf, (unsigned char)type);
    } else {
        uint32_t be = msgpack_be32(len);
        smart_str_appendc(buf, 0xc9);
        smart_str_appendl(buf, (const void *)&be, 4);
        smart_str_appendc(buf, (unsigned char)type);
    }

    if (n == 0 || len >= n) {
        smart_str_appendl(buf, str, len);
    } else {
        int i = 0;
        smart_str_appendl(buf, str, len);
        do {
            smart_str_appendc(buf, 0x00);
            ++i;
        } while (i < n);
    }
}
