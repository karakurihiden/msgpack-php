--TEST--
Test serialize_precision (part 1)
--INI--
serialize_precision=10
--FILE--
<?php
namespace MessagePack;

var_dump(bin2hex(serialize(0.1)));
?>
--EXPECTF--
string(18) "cb3fb999999999999a"
