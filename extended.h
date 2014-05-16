#ifndef PHP_MSGPACK_EXTENDED_H
#define PHP_MSGPACK_EXTENDED_H

typedef struct php_msgpack_extended_data {
    int dtor;
    zval *function;
} php_msgpack_extended_data_t;

#define MSGPACK_EXTENDED_CLASS           1
#define MSGPACK_EXTENDED_SERIALIZABLE    2
#define MSGPACK_EXTENDED_REFERENCE       3
#define MSGPACK_EXTENDED_REFERENCE_TYPE  4
#define MSGPACK_EXTENDED_REFERENCE_VALUE 5

#define MSGPACK_EXTENDED_PROTECTED 1
#define MSGPACK_EXTENDED_PRIVATE   2

#endif  /* PHP_MSGPACK_EXTENDED_H */
