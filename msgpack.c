#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
//#include "ext/standard/php_var.h"

#include "php_msgpack.h"
#include "serialize.h"
#include "unserialize.h"
#include "extended.h"

ZEND_DECLARE_MODULE_GLOBALS(msgpack)

static
ZEND_INI_MH(OnMsgPackType)
{
    long *p, tmp;
    int i, types[] = { MSGPACK_G(type.class),
                       MSGPACK_G(type.serializable),
                       MSGPACK_G(type.reference),
                       MSGPACK_G(type.reference_type),
                       MSGPACK_G(type.reference_value) };
#ifndef ZTS
    char *base = (char *)mh_arg2;
#else
    char *base;
    base = (char *)ts_resource(*((int *)mh_arg2));
#endif

    tmp = zend_atol(new_value, new_value_length);
    if (tmp < 0 || tmp > 255) {
        MSGPACK_ERR(E_WARNING, "invalid msgpack extended type");
        return FAILURE;
    }

    if (strcmp(entry->value, new_value) == 0) {
        return SUCCESS;
    }

    if (strcmp(entry->name, "msgpack.extended_class") == 0) {
        types[0] = -1;
    } else if (strcmp(entry->name, "msgpack.extended_serializable") == 0) {
        types[1] = -1;
    } else if (strcmp(entry->name, "msgpack.extended_reference") == 0) {
        types[2] = -1;
    } else if (strcmp(entry->name, "msgpack.extended_reference_type") == 0) {
        types[3] = -1;
    } else if (strcmp(entry->name, "msgpack.extended_reference_value") == 0) {
        types[4] = -1;
    }

    for (i = 0; i < (sizeof(types)/sizeof(int)); i++) {
        if (tmp == types[i]) {
            MSGPACK_ERR(E_WARNING, "exsist msgpack extended type");
            return FAILURE;
        }
    }

    p = (long *)(base+(size_t)mh_arg1);
    *p = tmp;

    return SUCCESS;
}

#define STR(s)  #s
#define XSTR(s) STR(s)

ZEND_INI_BEGIN()
    STD_ZEND_INI_ENTRY("msgpack.extended_class",
                       XSTR(MSGPACK_EXTENDED_CLASS),
                       ZEND_INI_ALL, OnMsgPackType, type.class,
                       zend_msgpack_globals, msgpack_globals)
    STD_ZEND_INI_ENTRY("msgpack.extended_serializable",
                       XSTR(MSGPACK_EXTENDED_SERIALIZABLE),
                       ZEND_INI_ALL, OnMsgPackType, type.serializable,
                       zend_msgpack_globals, msgpack_globals)
    STD_ZEND_INI_ENTRY("msgpack.extended_reference",
                       XSTR(MSGPACK_EXTENDED_REFERENCE),
                       ZEND_INI_ALL, OnMsgPackType, type.reference,
                       zend_msgpack_globals, msgpack_globals)
    STD_ZEND_INI_ENTRY("msgpack.extended_reference_type",
                       XSTR(MSGPACK_EXTENDED_REFERENCE_TYPE),
                       ZEND_INI_ALL, OnMsgPackType, type.reference_type,
                       zend_msgpack_globals, msgpack_globals)
    STD_ZEND_INI_ENTRY("msgpack.extended_reference_value",
                       XSTR(MSGPACK_EXTENDED_REFERENCE_VALUE),
                       ZEND_INI_ALL, OnMsgPackType, type.reference_value,
                       zend_msgpack_globals, msgpack_globals)
ZEND_INI_END()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_serialize, 0, 0, 1)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_unserialize, 0, 0, 1)
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_register_unserialize_function, 0, 0, 2)
    ZEND_ARG_INFO(0, type)
    ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_msgpack_extended_serialize, 0, 0, 2)
    ZEND_ARG_INFO(0, type)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

MSGPACK_FUNCTION(serialize)
{
    zval *struc;
    php_msgpack_serialize_data_t data;
    smart_str buf = { NULL, 0, 0 };

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
                              &struc) == FAILURE) {
        return;
    }

    Z_TYPE_P(return_value) = IS_STRING;
    Z_STRVAL_P(return_value) = NULL;
    Z_STRLEN_P(return_value) = 0;

    PHP_MSGPACK_SERIALIZE_INIT(data);

    php_msgpack_serialize(&buf, struc, data TSRMLS_CC);

    PHP_MSGPACK_SERIALIZE_DESTROY(data);

    if (EG(exception)) {
        smart_str_free(&buf);
        RETURN_FALSE;
    }

    if (buf.c) {
        smart_str_0(&buf);
        RETURN_STRINGL(buf.c, buf.len, 0);
    } else {
        RETURN_NULL();
    }
}

MSGPACK_FUNCTION(unserialize)
{
    char *buf;
    int buf_len;
    const unsigned char *p;
    php_msgpack_unserialize_data_t data;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                              &buf, &buf_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (buf_len == 0) {
        RETURN_FALSE;
    }

    p = (const unsigned char *)buf;

    PHP_MSGPACK_UNSERIALIZE_INIT(data);

    if (!php_msgpack_unserialize(&return_value, &p, p + buf_len,
                                 &data, NULL TSRMLS_CC) ||
        ((char *)p - buf) != buf_len) {
        PHP_MSGPACK_UNSERIALIZE_DESTROY(data);
        zval_dtor(return_value);
        if (!EG(exception)) {
            MSGPACK_ERR(E_NOTICE, "Error at offset %ld of %d bytes",
                        (long)((char *)p - buf), buf_len);
        }
        RETURN_FALSE;
    }

    PHP_MSGPACK_UNSERIALIZE_DESTROY(data);
}

MSGPACK_EXTENDED_FUNCTION(register_unserialize_function)
{
    long type;
    char *fname = NULL;
    php_msgpack_extended_data_t data = { 1, NULL };

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz",
                              &type, &data.function) == FAILURE) {
        return;
    }

    RETVAL_FALSE;

    if (type < 0 || type > 255) {
        MSGPACK_ERR(E_WARNING, "invalid extended type");
        return;
    }

    if (!zend_is_callable(data.function, 0, &fname TSRMLS_CC)) {
        MSGPACK_ERR(E_WARNING, "invalid callback function '%s'", fname);
        if (fname) {
            efree(fname);
        }
        return;
    }

    if (zend_hash_index_exists(MSGPACK_G(extended).unserializer, type)) {
        MSGPACK_ERR(E_WARNING, "override extended type serializer: %ld", type);
    }

    Z_ADDREF_P(data.function);

    if (zend_hash_index_update(MSGPACK_G(extended).unserializer, type,
                               &data, sizeof(php_msgpack_extended_data_t),
                               NULL) == SUCCESS) {
        RETVAL_TRUE;
    } else {
        zval_ptr_dtor(&data.function);
    }

    if (fname) {
        efree(fname);
    }
}

MSGPACK_EXTENDED_FUNCTION(serialize)
{
    long type;
    char *str;
    int str_len;
    smart_str buf = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lp",
                              &type, &str, &str_len) == FAILURE) {
        return;
    }

    if (type < 0 || type > 255) {
        MSGPACK_ERR(E_WARNING, "invalid msgpack extended type");
        RETURN_FALSE;
    }

    if (type == MSGPACK_G(type.class) ||
        type == MSGPACK_G(type.serializable) ||
        type == MSGPACK_G(type.reference) ||
        type == MSGPACK_G(type.reference_type) ||
        type == MSGPACK_G(type.reference_value)) {
        MSGPACK_ERR(E_WARNING, "reserved msgpack extended type: %ld", type);
        RETURN_FALSE;
    }

    Z_TYPE_P(return_value) = IS_STRING;
    Z_STRVAL_P(return_value) = NULL;
    Z_STRLEN_P(return_value) = 0;

    php_msgpack_serialize_extended(&buf, type, str, str_len);
    smart_str_0(&buf);

    if (buf.c) {
        RETURN_STRINGL(buf.c, buf.len, 0);
    } else {
        RETURN_NULL();
    }
}

const zend_function_entry msgpack_functions[] = {
    MSGPACK_FE(serialize, arginfo_msgpack_serialize)
    MSGPACK_FE(unserialize, arginfo_msgpack_unserialize)
    MSGPACK_EXTENDED_FE(register_unserialize_function,
                        arginfo_msgpack_register_unserialize_function)
    MSGPACK_EXTENDED_FE(serialize, arginfo_msgpack_extended_serialize)
    MSGPACK_FALIAS(MSGPACK_EXTENDED_NS, unserialize,
                   MSGPACK_FN(unserialize), arginfo_msgpack_unserialize)
    ZEND_FE_END
};

static void
php_msgpack_extended_unserializer_dtor(php_msgpack_extended_data_t *data)
{
    if (data && data->function && data->dtor) {
        zval_ptr_dtor(&data->function);
    }
}

static void
msgpack_init_globals(zend_msgpack_globals *msgpack_globals)
{
    msgpack_globals->lock = 0;
    memset(&msgpack_globals->serialize,
           0, sizeof(msgpack_globals->serialize));
    memset(&msgpack_globals->unserialize,
           0, sizeof(msgpack_globals->unserialize));
    memset(&msgpack_globals->extended,
           0, sizeof(msgpack_globals->extended));

    msgpack_globals->type.class = MSGPACK_EXTENDED_CLASS;
    msgpack_globals->type.serializable = MSGPACK_EXTENDED_SERIALIZABLE;
    msgpack_globals->type.reference = MSGPACK_EXTENDED_REFERENCE;
    msgpack_globals->type.reference_type = MSGPACK_EXTENDED_REFERENCE_TYPE;
    msgpack_globals->type.reference_value = MSGPACK_EXTENDED_REFERENCE_VALUE;
}

ZEND_MINIT_FUNCTION(msgpack)
{
    ZEND_INIT_MODULE_GLOBALS(msgpack, msgpack_init_globals, NULL);
    REGISTER_INI_ENTRIES();

    //php_msgpack_class_register(TSRMLS_C);

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(msgpack)
{
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}

ZEND_RINIT_FUNCTION(msgpack)
{
    MSGPACK_G(lock) = 0;
    memset(&MSGPACK_G(serialize), 0, sizeof(MSGPACK_G(serialize)));
    memset(&MSGPACK_G(unserialize), 0, sizeof(MSGPACK_G(unserialize)));

    /* Initilize extended unserializer */
    ALLOC_HASHTABLE(MSGPACK_G(extended).unserializer);
    zend_hash_init(MSGPACK_G(extended).unserializer, 5, NULL,
                   (dtor_func_t)php_msgpack_extended_unserializer_dtor, 0);

    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(msgpack)
{
    /* Cleanup extended unserializer */
    if (MSGPACK_G(extended).unserializer) {
        zend_hash_destroy(MSGPACK_G(extended).unserializer);
        efree(MSGPACK_G(extended).unserializer);
    }
    return SUCCESS;
}

ZEND_MINFO_FUNCTION(msgpack)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "MessagePack support", "enabled");
    php_info_print_table_row(2, "Extension Version", MSGPACK_EXT_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

zend_module_entry msgpack_module_entry = {
    STANDARD_MODULE_HEADER,
    "msgpack",
    msgpack_functions,
    ZEND_MINIT(msgpack),
    ZEND_MSHUTDOWN(msgpack),
    ZEND_RINIT(msgpack),
    ZEND_RSHUTDOWN(msgpack),
    ZEND_MINFO(msgpack),
    MSGPACK_EXT_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#if COMPILE_DL_MSGPACK
ZEND_GET_MODULE(msgpack)
#endif
