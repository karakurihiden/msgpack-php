#ifndef PHP_MSGPACK_H
#define PHP_MSGPACK_H

#define MSGPACK_EXT_VERSION "0.1.1"

#define MSGPACK_NS "MessagePack"
#define MSGPACK_EXTENDED_NS MSGPACK_NS"\\Extended"

#define MSGPACK_FN(name) zif_msgpack_##name
#define MSGPACK_EXTENDED_FN(name) zif_msgpack_extended_##name
#define MSGPACK_FUNCTION(name) ZEND_NAMED_FUNCTION(MSGPACK_FN(name))
#define MSGPACK_EXTENDED_FUNCTION(name) ZEND_NAMED_FUNCTION(MSGPACK_EXTENDED_FN(name))
#define MSGPACK_FE(name, arg_info) ZEND_NS_FENTRY(MSGPACK_NS, name, MSGPACK_FN(name), arg_info, 0)
#define MSGPACK_EXTENDED_FE(name, arg_info) ZEND_NS_FENTRY(MSGPACK_EXTENDED_NS, name, MSGPACK_EXTENDED_FN(name), arg_info, 0)

#define MSGPACK_FALIAS(ns, zend_name, name, arg_info) ZEND_NS_FENTRY(ns, zend_name, name, arg_info, 0)

/*
#define MSGPACK_ZEND_METHOD(classname, name) ZEND_METHOD(MessagePack_##classname, name)
#define MSGPACK_ZEND_ME(classname, name, arg_info, flags) ZEND_ME(MessagePack_##classname, name, arg_info, flags)
#define MSGPACK_ZEND_MALIAS(classname, name, alias, arg_info, flags) ZEND_MALIAS(MessagePack_##classname, name, alias, arg_info, flags)
#define MSGPACK_LONG_CONSTANT(name, val) REGISTER_NS_LONG_CONSTANT(MSGPACK_NS, name, val, CONST_CS|CONST_PERSISTENT)
#define MSGPACK_STRING_CONSTANT(name, val) REGISTER_NS_STRING_CONSTANT(MSGPACK_NS, name, val, CONST_CS|CONST_PERSISTENT)
*/

extern zend_module_entry msgpack_module_entry;
#define phpext_msgpack_ptr &msgpack_module_entry

#ifdef PHP_WIN32
#    define PHP_MSGPACK_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#    define PHP_MSGPACK_API __attribute__ ((visibility("default")))
#else
#    define PHP_MSGPACK_API
#endif

#ifdef ZTS
#    include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(msgpack)
    /* zend_class_entry *incomplete_class; */
    unsigned lock;
    struct {
        void *data;
        unsigned level;
    } serialize;
    struct {
        void *data;
        unsigned level;
    } unserialize;
    struct {
        long class;
        long serializable;
        long reference;
        long reference_type;
        long reference_value;
    } type;
    struct {
        HashTable *unserializer;
    } extended;
ZEND_END_MODULE_GLOBALS(msgpack)

#ifdef ZTS
#    define MSGPACK_G(v) TSRMG(msgpack_globals_id, zend_msgpack_globals *, v)
#else
#    define MSGPACK_G(v) (msgpack_globals.v)
#endif

#define MSGPACK_ERR(e, ...) php_error_docref(NULL TSRMLS_CC, e, __VA_ARGS__)

#ifndef ZED_FE_END
#define ZEND_FE_END { NULL, NULL, NULL, 0, 0 }
#endif

#ifndef ZVAL_COPY_VALUE
#define ZVAL_COPY_VALUE(z, v)      \
    do {                           \
        (z)->value = (v)->value;   \
        Z_TYPE_P(z) = Z_TYPE_P(v); \
    } while (0)
#endif

#ifndef INIT_PZVAL_COPY
#define INIT_PZVAL_COPY(z, v)   \
    do {                        \
        ZVAL_COPY_VALUE(z, v);  \
        Z_SET_REFCOUNT_P(z, 1); \
        Z_UNSET_ISREF_P(z);     \
    } while (0)
#endif

#endif  /* PHP_MSGPACK_H */
