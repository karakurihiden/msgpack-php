--TEST--
serialize() and floats/doubles
--FILE--
<?php
namespace MessagePack;
    $foo = 1.428571428571428647642857142;
    $bar = unserialize(serialize($foo));
    var_dump(($foo === $bar));
?>
--EXPECT--
bool(true)
