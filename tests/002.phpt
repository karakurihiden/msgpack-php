--TEST--
unserialize() pack data
--FILE--
<?php
namespace MessagePack;
var_dump(unserialize(pack('H*','c2')));
var_dump(unserialize(pack('H*','c3')));
var_dump(unserialize(pack('H*','cd0337')));
var_dump(unserialize(pack('H*','a0')));
var_dump(unserialize(pack('H*','a3666f6f')));
var_dump(unserialize(pack('H*','8100a23132')));
var_dump(unserialize(pack('H*','8200800180')));
var_dump(unserialize(pack('H*','8300a3666f6f01a362617202a362617a')));
var_dump(unserialize(pack('H*','d801a8737464436c61737300000000000000')));
?>
===DONE===
--EXPECTF--
bool(false)
bool(true)
int(823)
string(0) ""
string(3) "foo"
array(1) {
  [0]=>
  string(2) "12"
}
array(2) {
  [0]=>
  array(0) {
  }
  [1]=>
  array(0) {
  }
}
array(3) {
  [0]=>
  string(3) "foo"
  [1]=>
  string(3) "bar"
  [2]=>
  string(3) "baz"
}
object(stdClass)#%d (0) {
}
===DONE===
