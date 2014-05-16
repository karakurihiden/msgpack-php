--TEST--
serialize() objects of incomplete class
--SKIPIF--
--FILE--
<?php
namespace MessagePack;
$str = pack('H*', 'c70b01a954657374436c61737300');
$obj = unserialize($str);
var_dump($obj);
echo bin2hex(serialize($obj))."\n";
var_dump($obj);
echo bin2hex(serialize($obj))."\n";
var_dump($obj);
?>
--EXPECT--
object(__PHP_Incomplete_Class)#1 (1) {
  ["__PHP_Incomplete_Class_Name"]=>
  string(9) "TestClass"
}
c70b01a954657374436c61737300
object(__PHP_Incomplete_Class)#1 (1) {
  ["__PHP_Incomplete_Class_Name"]=>
  string(9) "TestClass"
}
c70b01a954657374436c61737300
object(__PHP_Incomplete_Class)#1 (1) {
  ["__PHP_Incomplete_Class_Name"]=>
  string(9) "TestClass"
}
