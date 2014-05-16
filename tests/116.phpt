--TEST--
Unserialization of classes derived from ArrayIterator fails
--FILE--
<?php
namespace MessagePack;
class Foo1 extends \ArrayIterator
{
}
class Foo2 {
}
$x = array(new Foo1(),new Foo2);
$s = serialize($x);
$s = str_replace("Foo", "Bar", $s);
$y = unserialize($s);
var_dump($y);
--EXPECTF--
Warning: MessagePack\unserialize(): Class __PHP_Incomplete_Class has no unserializer in %s on line %d
array(2) {
  [0]=>
  object(__PHP_Incomplete_Class)#%d (1) {
    ["__PHP_Incomplete_Class_Name"]=>
    string(16) "MessagePack\Bar1"
  }
  [1]=>
  object(__PHP_Incomplete_Class)#%d (1) {
    ["__PHP_Incomplete_Class_Name"]=>
    string(16) "MessagePack\Bar2"
  }
}
