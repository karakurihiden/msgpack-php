--TEST--
serialize()/unserialize()
--INI--
--FILE--
<?php
namespace MessagePack;
class t
{
    public function __construct()
    {
        $this->a = "hallo";
    }
}

class s
{
    public $a;
    public $b;
    public $c;

    public function __construct()
    {
        $this->a = "hallo";
        $this->b = "php";
        $this->c = "world";
        $this->d = "!";
    }

    function __sleep()
    {
        echo "__sleep called\n";
        return array("a","c");
    }

    function __wakeup()
    {
        echo "__wakeup called\n";
    }
}


echo bin2hex(serialize(NULL))."\n";
echo bin2hex(serialize((bool) true))."\n";
echo bin2hex(serialize((bool) false))."\n";
echo bin2hex(serialize(1))."\n";
echo bin2hex(serialize(0))."\n";
echo bin2hex(serialize(-1))."\n";
echo bin2hex(serialize(2147483647))."\n";
echo bin2hex(serialize(-2147483647))."\n";
echo bin2hex(serialize(1.123456789))."\n";
echo bin2hex(serialize(1.0))."\n";
echo bin2hex(serialize(0.0))."\n";
echo bin2hex(serialize(-1.0))."\n";
echo bin2hex(serialize(-1.123456789))."\n";
echo bin2hex(serialize("hallo"))."\n";
echo bin2hex(serialize(array(1,1.1,"hallo",NULL,true,array())))."\n";

$t = new t();
$data = serialize($t);
echo bin2hex($data)."\n";
$t = unserialize($data);
var_dump($t);

$t = new s();
$data = serialize($t);
echo bin2hex($data)."\n";
$t = unserialize($data);
var_dump($t);

$a = array("a" => "test");
$a[ "b" ] = &$a[ "a" ];
var_dump($a);
$data = serialize($a);
echo bin2hex($data)."\n";
$a = unserialize($data);
var_dump($a);
?>
--EXPECTF--
c0
c3
c2
01
00
ff
ce7fffffff
d280000001
cb3ff1f9add3739636
cb3ff0000000000000
cb0000000000000000
cbbff0000000000000
cbbff1f9add3739636
a568616c6c6f
86000101cb3ff199999999999a02a568616c6c6f03c004c30580
c71701ad4d6573736167655061636b5c7401a161a568616c6c6f
object(MessagePack\t)#%d (1) {
  ["a"]=>
  string(5) "hallo"
}
__sleep called
c71f01ad4d6573736167655061636b5c7302a161a568616c6c6fa163a5776f726c64
__wakeup called
object(MessagePack\s)#%d (3) {
  ["a"]=>
  string(5) "hallo"
  ["b"]=>
  NULL
  ["c"]=>
  string(5) "world"
}
array(2) {
  ["a"]=>
  &string(4) "test"
  ["b"]=>
  &string(4) "test"
}
82a161c70503a474657374a162d40401
array(2) {
  ["a"]=>
  &string(4) "test"
  ["b"]=>
  &string(4) "test"
}
