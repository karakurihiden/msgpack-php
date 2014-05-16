--TEST--
Test serialize() & unserialize() functions: objects - ensure that COW references of objects are not serialized separately (unlike other types).
--FILE--
<?php
namespace MessagePack;
/* Prototype  : proto string serialize(mixed variable)
 * Description: Returns a string representation of variable (which can later be unserialized)
 * Source code: ext/standard/var.c
 * Alias to functions:
 */
/* Prototype  : proto mixed unserialize(string variable_representation)
 * Description: Takes a string representation of variable and recreates it
 * Source code: ext/standard/var.c
 * Alias to functions:
 */

$x = new \stdClass;
$ref = &$x;
var_dump(bin2hex(serialize(array($x, $x))));

$x = 1;
$ref = &$x;
var_dump(bin2hex(serialize(array($x, $x))));

$x = "a";
$ref = &$x;
var_dump(bin2hex(serialize(array($x, $x))));

$x = true;
$ref = &$x;
var_dump(bin2hex(serialize(array($x, $x))));

$x = null;
$ref = &$x;
var_dump(bin2hex(serialize(array($x, $x))));

$x = array();
$ref = &$x;
var_dump(bin2hex(serialize(array($x, $x))));

echo "Done";
?>
--EXPECTF--
string(38) "8200c70a01a8737464436c6173730001d40501"
string(10) "8200010101"
string(14) "8200a16101a161"
string(10) "8200c301c3"
string(10) "8200c001c0"
string(10) "8200800180"
Done
