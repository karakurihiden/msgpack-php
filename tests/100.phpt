--TEST--
serialize() and __sleep()
--FILE--
<?php
namespace MessagePack;
class t
{
    function __construct()
    {
        $this->a = 'hello';
    }

    function __sleep()
    {
        echo "__sleep called\n";
        return array('a','b');
    }
}

$t = new t();
$data = serialize($t);
echo bin2hex($data)."\n";
$t = unserialize($data);
var_dump($t);
?>
--EXPECTF--
__sleep called

Notice: MessagePack\serialize(): "b" returned as member variable from __sleep() but does not exist in %s on line %d
c71a01ad4d6573736167655061636b5c7402a161a568656c6c6fa162c0
object(MessagePack\t)#%d (2) {
  ["a"]=>
  string(5) "hello"
  ["b"]=>
  NULL
}
