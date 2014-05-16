--TEST--
serialize followed by unserialize with numeric object prop. gives integer prop
--FILE--
<?php
namespace MessagePack;
$a = new \stdClass();
$a->{0} = 'X';
$a->{1} = 'Y';
var_dump(bin2hex(serialize($a)));
var_dump($a->{0});
$b = unserialize(serialize($a));
var_dump(bin2hex(serialize($b)));
var_dump($b->{0});
--EXPECT--
string(42) "c71201a8737464436c61737302a130a158a131a159"
string(1) "X"
string(42) "c71201a8737464436c61737302a130a158a131a159"
string(1) "X"
