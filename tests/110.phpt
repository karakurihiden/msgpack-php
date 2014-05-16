--TEST--
unserialize broken on 64-bit systems
--SKIPIF--
--FILE--
<?php
namespace MessagePack;
echo unserialize(serialize(2147483648));
?>
--EXPECT--
2147483648
