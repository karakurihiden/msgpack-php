--TEST--
Serialize / Unserialize misbehaviour under OS with different bit numbers
--SKIPIF--
<?php
if (PHP_INT_SIZE == 4) {
    die("skip this test is for 32bit platform only");
}
?>
--FILE--
<?php
namespace MessagePack;
var_dump(unserialize(pack('H*','cf000000012a05f200')) == 5000000000);
?>
--EXPECT--
bool(true)
