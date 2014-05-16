--TEST--
serialize()/unserialize() with exotic letters
--FILE--
<?php
namespace MessagePack;
    $������ = array('������' => '������');

    class �berK��li��
    {
        public $��������ber = '������';
    }

    $foo = new �berk��li��();

    var_dump(bin2hex(serialize($foo)));
//    var_dump(unserialize(serialize($foo)));
/*
object(�berK��li��)#2 (1) {
  ["��������ber"]=>
  string(6) "������"
}
*/
    var_dump(bin2hex(serialize($������)));
//    var_dump(unserialize(serialize($������)));
/*
array(1) {
  ["������"]=>
  string(6) "������"
}
*/
?>
--EXPECT--
string(94) "c72c01b74d6573736167655061636b5cdc6265724bf6f66c69e4e501abe5e4f6c5c4d6fcdc626572a6e5e4f6c5c4d6"
string(30) "81a6e5e4f6c5c4d6a6e5e4f6c5c4d6"
