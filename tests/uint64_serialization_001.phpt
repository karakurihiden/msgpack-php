--TEST--
unserialize exceed INT_MAX
--FILE--
<?php
namespace MessagePack;

var_dump(unserialize(pack('H*', 'cfffffffffffffffff')));
var_dump(unserialize(pack('H*', '81cfffffffffffffffff01')));
var_dump(unserialize(pack('H*', '91cfffffffffffffffff')));

?>
--EXPECTF--
Warning: MessagePack\unserialize(): Convert string, exceed INT_MAX in %s on line %d
string(20) "18446744073709551615"

Warning: MessagePack\unserialize(): Convert string, exceed INT_MAX in %s on line %d
array(1) {
  ["18446744073709551615"]=>
  int(1)
}

Warning: MessagePack\unserialize(): Convert string, exceed INT_MAX in %s on line %d
array(1) {
  [0]=>
  string(20) "18446744073709551615"
}
