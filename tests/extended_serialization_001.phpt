--TEST--
extended serialization
--INI--
--FILE--
<?php
$ser = MessagePack\Extended\serialize(10, 'test');
var_dump(bin2hex($ser));

var_dump(MessagePack\unserialize($ser));

function func($data) {
    echo __FUNCTION__, PHP_EOL;
    var_dump($data);
    return $data;
}

var_dump(MessagePack\Extended\register_unserialize_function(10, 'func'));

var_dump(MessagePack\unserialize($ser));
?>
--EXPECTF--
string(12) "d60a74657374"

Warning: MessagePack\unserialize(): not found unserializer type: 10 in %s on line 5

Notice: MessagePack\unserialize(): Error at offset 2 of 6 bytes in %s on line 5
bool(false)
bool(true)
func
string(4) "test"
string(4) "test"
