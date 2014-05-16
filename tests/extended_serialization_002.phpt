--TEST--
extended serialization
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

$ser = MessagePack\Extended\serialize(11, 'test');
MessagePack\Extended\register_unserialize_function(11, array(new Unpack, 'func1'));
var_dump(MessagePack\unserialize($ser));

$ser = MessagePack\Extended\serialize(12, 'test');
$unpack = new Unpack;
MessagePack\Extended\register_unserialize_function(12, array($unpack, 'func2'));
var_dump(MessagePack\unserialize($ser));

$ser = MessagePack\Extended\serialize(13, 'test');
MessagePack\Extended\register_unserialize_function(13, 'Unpack::func3');
var_dump(MessagePack\unserialize($ser));

$ser = MessagePack\Extended\serialize(14, 'test');
MessagePack\Extended\register_unserialize_function(14, function ($data) { return $data; });
var_dump(MessagePack\unserialize($ser));
?>
--EXPECTF--
func
string(4) "test"
Unpack::func1
string(4) "test"
Unpack::func2
string(4) "test"
Unpack::func3
string(4) "test"
string(4) "test"
