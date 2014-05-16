--TEST--
extended serialization: exist type
--INI--
--FILE--
<?php
function func($data) {
    echo __FUNCTION__, PHP_EOL;
    return $data;
}

class Unpack {
    public function func1($data) {
        echo __METHOD__, PHP_EOL;
        return $data;
    }
    public function func2($data) {
        echo __METHOD__, PHP_EOL;
        return $data;
    }
    static public function func3($data) {
        echo __METHOD__, PHP_EOL;
        return $data;
    }
}

$ser = MessagePack\Extended\serialize(10, 'test');
MessagePack\Extended\register_unserialize_function(10, 'func');
var_dump(MessagePack\unserialize($ser));

MessagePack\Extended\register_unserialize_function(10, array(new Unpack, 'func1'));
var_dump(MessagePack\unserialize($ser));

$unpack = new Unpack;
MessagePack\Extended\register_unserialize_function(10, array($unpack, 'func2'));
var_dump(MessagePack\unserialize($ser));

MessagePack\Extended\register_unserialize_function(10, 'Unpack::func3');
var_dump(MessagePack\unserialize($ser));

MessagePack\Extended\register_unserialize_function(10, function ($data) { return $data; });
var_dump(MessagePack\unserialize($ser));
?>
--EXPECTF--
func
string(4) "test"

Warning: MessagePack\Extended\register_unserialize_function(): override extended type serializer: 10 in %s on line %d
Unpack::func1
string(4) "test"

Warning: MessagePack\Extended\register_unserialize_function(): override extended type serializer: 10 in %s on line %d
Unpack::func2
string(4) "test"

Warning: MessagePack\Extended\register_unserialize_function(): override extended type serializer: 10 in %s on line %d
Unpack::func3
string(4) "test"

Warning: MessagePack\Extended\register_unserialize_function(): override extended type serializer: 10 in %s on line %d
string(4) "test"
