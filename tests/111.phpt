--TEST--
__sleep and private property
--FILE--
<?php
namespace MessagePack;
class Parents {
   private $parents;
   public function __sleep() {
       return array("parents");
   }
}

class Child extends Parents {
    private $child;
    public function __sleep() {
        return array_merge(array("child"), parent::__sleep());
    }
}

$obj = new Child();
serialize($obj);

?>
--EXPECTF--
Notice: MessagePack\serialize(): "parents" returned as member variable from __sleep() but does not exist in %s on line %d
