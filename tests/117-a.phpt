--TEST--
Serialize / Unserialize misbehaviour under OS with different bit numbers
--SKIPIF--
<?php
if (PHP_INT_SIZE == 8) {
    die("skip this test is for 64bit platform only");
}
?>
--FILE--
<?php
namespace MessagePack;
var_dump(unserialize(pack('H*','cf000000012a05f200')) == 5000000000);
?>
--EXPECTF--
Warning: MessagePack\unserialize(): Convert string, exceed INT_MAX in %s on line %d
bool(true)
