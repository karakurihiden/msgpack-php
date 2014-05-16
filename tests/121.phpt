--TEST--
Seg fault or broken object references on unserialize()
--FILE--
<?php
$serialized_object=pack('H*','c71f01a14104a162c70301a14200a26231d40501a163c70301a14200a26331d40502');

ini_set('unserialize_callback_func','mycallback');

function mycallback($classname) {
    MessagePack\unserialize(pack('H*', '04'));
    eval ("class $classname {} ");
}

print_r(MessagePack\unserialize($serialized_object));
echo "okey";
?>
--EXPECT--
A Object
(
    [b] => B Object
        (
        )

    [b1] => B Object
        (
        )

    [c] => B Object
        (
        )

    [c1] => B Object
        (
        )

)
okey
