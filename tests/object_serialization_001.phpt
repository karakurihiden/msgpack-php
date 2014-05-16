--TEST--
object property empty
--FILE--
<?php
class s {
    public $a = 'a';
    protected $b = 'b';
    private $c = 'c';
}

var_dump(MessagePack\unserialize(pack('H*', 'c71001a17303a0a14101a162a14202a163a143')));

?>
--EXPECTF--
Notice: MessagePack\unserialize(): Illegal member variable name in %s on line %d
object(s)#%d (3) {
  ["a"]=>
  string(1) "a"
  ["b":protected]=>
  string(1) "B"
  ["c":"s":private]=>
  string(1) "C"
}
