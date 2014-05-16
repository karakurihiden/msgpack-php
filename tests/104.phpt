--TEST--
unserialize() crashes with invalid data
--SKIPIF--
--FILE--
<?php
namespace MessagePack;
var_dump(unserialize("s:-1:\"\";"));
var_dump(unserialize("i:823"));
var_dump(unserialize("O:8:\"stdClass :0:{}"));
var_dump(unserialize("O:8:\"stdClass\"+0:{}"));
var_dump(unserialize("O:1000:\"stdClass\":0:{}"));
var_dump(unserialize("a:2:{i:0;s:2:\"12\":"));
var_dump(unserialize("a:2:{i:0;s:2:\"12\";i:1;s:3000:\"123"));
var_dump(unserialize("a:2:{i:0;s:2:\"12\"+i:1;s:3:\"123\";}"));
var_dump(unserialize("a:2:{i:0;s:2:\"12\";i:1;s:3:\"123\";"));
var_dump(unserialize("s:3000:\"123\";"));
var_dump(unserialize("s:3000:\"123"));
var_dump(unserialize("s:3:\"123;"));
var_dump(unserialize("s:0:\"123\";"));
?>
===DONE===
--EXPECTF--
Notice: MessagePack\unserialize(): Error at offset 1 of 8 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 5 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 19 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 19 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 22 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 18 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 33 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 33 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 32 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 13 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 11 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 9 bytes in %s on line %d
bool(false)

Notice: MessagePack\unserialize(): Error at offset 1 of 10 bytes in %s on line %d
bool(false)
===DONE===
