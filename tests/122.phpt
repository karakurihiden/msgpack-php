--TEST--
serialize incorrectly saving objects when they are cloned
--SKIPIF--
<?php
if (PHP_INT_SIZE != 8) {
    die("skip this test is for 64bit platform only");
}
?>
--FILE--
<?php
echo "Test\n";

class A
{
    public $a = array();

    public function __construct()
    {
        $this->a[] = new B(1);
        $this->a[] = new B(2);
    }
}

class B implements \Serializable
{
    public $b;

    public function __construct($c)
    {
        $this->b = new C($c);
    }

    public function serialize()
    {
        return MessagePack\serialize(clone $this->b);
    }

    public function unserialize($data)
    {
        $this->b = MessagePack\unserialize($data);
    }
}

class C
{
    public $c;

    public function __construct($c)
    {
        $this->c = $c;
    }
}

$a = MessagePack\unserialize(MessagePack\serialize(new A()));

print $a->a[0]->b->c . "\n";
print $a->a[1]->b->c . "\n";
?>
Done
--EXPECTF--
Test
1
2
Done
