--TEST--
set extended type
--INI--
--FILE--
<?php
class a {}

class b {
    public $a = 'a';
    protected $b = 'b';
    private $c = 'c';
}

class c implements Serializable {
    public function serialize() {
        return '';
    }
    public function unserialize($data) {
    }
}

echo "== extended class\n";
var_dump(bin2hex(MessagePack\serialize(new a)));

ini_set("msgpack.extended_class", "10");
var_dump(bin2hex(MessagePack\serialize(new a)));


echo "== extended class property\n";
var_dump(bin2hex(MessagePack\serialize(new b)));

ini_set("msgpack.extended_public", "11"); //no used
ini_set("msgpack.extended_protected", "12");
ini_set("msgpack.extended_private", "13");
var_dump(bin2hex(MessagePack\serialize(new b)));


echo "== extended class serializable\n";
var_dump(bin2hex(MessagePack\serialize(new c)));

ini_set("msgpack.extended_serializable", "14");
var_dump(bin2hex(MessagePack\serialize(new c)));


echo "== extended reference\n";
$a = array('a');
$v = array(&$a, &$a);
var_dump(bin2hex(MessagePack\serialize($v)));

ini_set("msgpack.extended_reference", "15");
var_dump(bin2hex(MessagePack\serialize($v)));


echo "== extended reference object\n";
$v = new stdClass;
$v->a = $v;
var_dump(bin2hex(MessagePack\serialize($v)));

ini_set("msgpack.extended_reference_object", "16");
var_dump(bin2hex(MessagePack\serialize($v)));
?>
--EXPECTF--
== extended class
string(12) "c70301a16100"
string(12) "c7030aa16100"
== extended class property
string(40) "c7110aa16203a161a16101a162a16202a163a163"
string(40) "c7110aa16203a161a16101a162a16202a163a163"
== extended class serializable
string(12) "c70302a163a0"
string(12) "c7030ea163a0"
== extended reference
string(20) "82008100a16101d40401"
string(20) "82008100a16101d40401"
== extended reference object
string(36) "c70f0aa8737464436c61737301a161d40500"
string(36) "c70f0aa8737464436c61737301a161d40500"
