--TEST--
Test serialize() & unserialize() functions: error conditions - wrong number of args.
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

echo "*** Testing serialize()/unserialize() : error conditions ***\n";

// Zero arguments
var_dump( serialize() );
var_dump( unserialize() );

//Test serialize with one more than the expected number of arguments
var_dump( serialize(1,2) );
var_dump( unserialize(1,2) );

echo "Done";
?>
--EXPECTF--
*** Testing serialize()/unserialize() : error conditions ***

Warning: MessagePack\serialize() expects exactly 1 parameter, 0 given in %s on line 17
NULL

Warning: MessagePack\unserialize() expects exactly 1 parameter, 0 given in %s on line 18
bool(false)

Warning: MessagePack\serialize() expects exactly 1 parameter, 2 given in %s on line 21
NULL

Warning: MessagePack\unserialize() expects exactly 1 parameter, 2 given in %s on line 22
bool(false)
Done
