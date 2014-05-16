--TEST--
Serializable interface breaks object references
--FILE--
<?php
namespace MessagePack;
echo "-TEST\n";

class a implements \Serializable {
    function serialize() {
        return serialize(get_object_vars($this));
    }
    function unserialize($s) {
        foreach (unserialize($s) as $p=>$v) {
            $this->$p=$v;
        }
    }
}
class b extends a {}
class c extends b {}

$c = new c;
$c->a = new a;
$c->a->b = new b;
$c->a->b->c = $c;
$c->a->c = $c;
$c->a->b->a = $c->a;
$c->a->a = $c->a;

$s = serialize($c);
printf("%s\n", bin2hex($s));

$d = unserialize($s);

var_dump(
    $d === $d->a->b->c,
    $d->a->a === $d->a,
    $d->a->b->a === $d->a,
    $d->a->c === $d
);

print_r($d);

echo "Done\n";

?>
--EXPECTF--
%aTEST
c75002ad4d6573736167655061636b5c63d94081a161c73a02ad4d6573736167655061636b5c61d92a83a162c71a02ad4d6573736167655061636b5c62ab82a163d40500a161d40502a163d40500a161d40502
bool(true)
bool(true)
bool(true)
bool(true)
MessagePack\c Object
(
    [a] => MessagePack\a Object
        (
            [b] => MessagePack\b Object
                (
                    [c] => MessagePack\c Object
 *RECURSION*
                    [a] => MessagePack\a Object
 *RECURSION*
                )

            [c] => MessagePack\c Object
 *RECURSION*
            [a] => MessagePack\a Object
 *RECURSION*
        )

)
Done
