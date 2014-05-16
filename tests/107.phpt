--TEST--
Problem in serialisation of circular references
--FILE--
<?php
namespace MessagePack;
class a {
    public $b;
}
class b {
    public $c;
}
class c {
    public $d;
}
$a = new a();
$a->b = new b();
$a->b->c = new c();
$a->b->c->d = $a;
var_dump(unserialize(serialize($a)));
?>
--EXPECTF--
object(MessagePack\a)#%d (1) {
  ["b"]=>
  object(MessagePack\b)#%d (1) {
    ["c"]=>
    object(MessagePack\c)#%d (1) {
      ["d"]=>
      *RECURSION*
    }
  }
}
