--TEST--
Unserializing of namespaced class object fails
--FILE--
<?php
namespace Foo;
class Bar {
}
echo bin2hex(\MessagePack\serialize(new Bar)) . "\n";
$x = \MessagePack\unserialize(\MessagePack\serialize(new Bar));
echo get_class($x) . "\n";
?>
--EXPECT--
c70901a7466f6f5c42617200
Foo\Bar
