--TEST--
set extended type exsist
--INI--
--FILE--
<?php
ini_set("msgpack.extended_class", "3");
var_dump(ini_get("msgpack.extended_class"));

ini_set("msgpack.extended_reference", "13");
ini_set("msgpack.extended_class", "3");
var_dump(ini_get("msgpack.extended_reference"));
var_dump(ini_get("msgpack.extended_class"));

ini_set("msgpack.extended_reference", "3");
var_dump(ini_get("msgpack.extended_reference"));
?>
--EXPECTF--
Warning: ini_set(): exsist msgpack extended type in %s on line 2
string(1) "1"
string(2) "13"
string(1) "3"

Warning: ini_set(): exsist msgpack extended type in %s on line 10
string(2) "13"
