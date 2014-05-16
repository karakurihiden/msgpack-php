--TEST--
serialize()/unserialize() with exotic letters
--FILE--
<?php
namespace MessagePack;
    $åäöÅÄÖ = array('åäöÅÄÖ' => 'åäöÅÄÖ');

    class ÜberKööliäå
    {
        public $åäöÅÄÖüÜber = 'åäöÅÄÖ';
    }

    $foo = new Überkööliäå();

    var_dump(bin2hex(serialize($foo)));
//    var_dump(unserialize(serialize($foo)));
/*
object(ÜberKööliäå)#2 (1) {
  ["åäöÅÄÖüÜber"]=>
  string(6) "åäöÅÄÖ"
}
*/
    var_dump(bin2hex(serialize($åäöÅÄÖ)));
//    var_dump(unserialize(serialize($åäöÅÄÖ)));
/*
array(1) {
  ["åäöÅÄÖ"]=>
  string(6) "åäöÅÄÖ"
}
*/
?>
--EXPECT--
string(94) "c72c01b74d6573736167655061636b5cdc6265724bf6f66c69e4e501abe5e4f6c5c4d6fcdc626572a6e5e4f6c5c4d6"
string(30) "81a6e5e4f6c5c4d6a6e5e4f6c5c4d6"
