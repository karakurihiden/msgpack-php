--TEST--
unserialize array format
--FILE--
<?php
namespace MessagePack;

var_dump(unserialize(pack('H*', '93a161a162a163')));
var_dump(unserialize(pack('H*', '93a161a16293a163a16491a165')));

?>
--EXPECTF--
array(3) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
  [2]=>
  string(1) "c"
}
array(3) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
  [2]=>
  array(3) {
    [0]=>
    string(1) "c"
    [1]=>
    string(1) "d"
    [2]=>
    array(1) {
      [0]=>
      string(1) "e"
    }
  }
}
