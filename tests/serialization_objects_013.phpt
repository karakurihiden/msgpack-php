--TEST--
Object serialization / unserialization: references amongst properties
--INI--
error_reporting = E_ALL & ~E_STRICT
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

function check(&$obj) {
    var_dump($obj);
    $ser = serialize($obj);
    var_dump(bin2hex($ser));

    $uobj = unserialize($ser);
    var_dump($uobj);
    $uobj->a = "obj->a.changed";
    var_dump($uobj);
    $uobj->b = "obj->b.changed";
    var_dump($uobj);
    $uobj->c = "obj->c.changed";
    var_dump($uobj);
}

echo "\n\n--- a refs b:\n";
$obj = new \stdClass;
$obj->a = &$obj->b;
$obj->b = 1;
$obj->c = 1;
check($obj);

echo "\n\n--- a refs c:\n";
$obj = new \stdClass;
$obj->a = &$obj->c;
$obj->b = 1;
$obj->c = 1;
check($obj);

echo "\n\n--- b refs a:\n";
$obj = new \stdClass;
$obj->a = 1;
$obj->b = &$obj->a;
$obj->c = 1;
check($obj);

echo "\n\n--- b refs c:\n";
$obj = new \stdClass;
$obj->a = 1;
$obj->b = &$obj->c;
$obj->c = 1;
check($obj);

echo "\n\n--- c refs a:\n";
$obj = new \stdClass;
$obj->a = 1;
$obj->b = 1;
$obj->c = &$obj->a;
check($obj);

echo "\n\n--- c refs b:\n";
$obj = new \stdClass;
$obj->a = 1;
$obj->b = 1;
$obj->c = &$obj->b;
check($obj);

echo "\n\n--- a,b refs c:\n";
$obj = new \stdClass;
$obj->a = &$obj->c;
$obj->b = &$obj->c;
$obj->c = 1;
check($obj);

echo "\n\n--- a,c refs b:\n";
$obj = new \stdClass;
$obj->a = &$obj->b;
$obj->b = 1;
$obj->c = &$obj->b;
check($obj);

echo "\n\n--- b,c refs a:\n";
$obj = new \stdClass;
$obj->a = 1;
$obj->b = &$obj->a;
$obj->c = &$obj->a;
check($obj);

echo "Done";
?>
--EXPECTF--

--- a refs b:
object(stdClass)#%d (3) {
  ["b"]=>
  &int(1)
  ["a"]=>
  &int(1)
  ["c"]=>
  int(1)
}
string(52) "c71701a8737464436c61737303a162d40301a161d40401a16301"
object(stdClass)#%d (3) {
  ["b"]=>
  &int(1)
  ["a"]=>
  &int(1)
  ["c"]=>
  int(1)
}
object(stdClass)#%d (3) {
  ["b"]=>
  &string(14) "obj->a.changed"
  ["a"]=>
  &string(14) "obj->a.changed"
  ["c"]=>
  int(1)
}
object(stdClass)#%d (3) {
  ["b"]=>
  &string(14) "obj->b.changed"
  ["a"]=>
  &string(14) "obj->b.changed"
  ["c"]=>
  int(1)
}
object(stdClass)#%d (3) {
  ["b"]=>
  &string(14) "obj->b.changed"
  ["a"]=>
  &string(14) "obj->b.changed"
  ["c"]=>
  string(14) "obj->c.changed"
}


--- a refs c:
object(stdClass)#%d (3) {
  ["c"]=>
  &int(1)
  ["a"]=>
  &int(1)
  ["b"]=>
  int(1)
}
string(52) "c71701a8737464436c61737303a163d40301a161d40401a16201"
object(stdClass)#%d (3) {
  ["c"]=>
  &int(1)
  ["a"]=>
  &int(1)
  ["b"]=>
  int(1)
}
object(stdClass)#%d (3) {
  ["c"]=>
  &string(14) "obj->a.changed"
  ["a"]=>
  &string(14) "obj->a.changed"
  ["b"]=>
  int(1)
}
object(stdClass)#%d (3) {
  ["c"]=>
  &string(14) "obj->a.changed"
  ["a"]=>
  &string(14) "obj->a.changed"
  ["b"]=>
  string(14) "obj->b.changed"
}
object(stdClass)#%d (3) {
  ["c"]=>
  &string(14) "obj->c.changed"
  ["a"]=>
  &string(14) "obj->c.changed"
  ["b"]=>
  string(14) "obj->b.changed"
}


--- b refs a:
object(stdClass)#%d (3) {
  ["a"]=>
  &int(1)
  ["b"]=>
  &int(1)
  ["c"]=>
  int(1)
}
string(52) "c71701a8737464436c61737303a161d40301a162d40401a16301"
object(stdClass)#%d (3) {
  ["a"]=>
  &int(1)
  ["b"]=>
  &int(1)
  ["c"]=>
  int(1)
}
object(stdClass)#%d (3) {
  ["a"]=>
  &string(14) "obj->a.changed"
  ["b"]=>
  &string(14) "obj->a.changed"
  ["c"]=>
  int(1)
}
object(stdClass)#%d (3) {
  ["a"]=>
  &string(14) "obj->b.changed"
  ["b"]=>
  &string(14) "obj->b.changed"
  ["c"]=>
  int(1)
}
object(stdClass)#%d (3) {
  ["a"]=>
  &string(14) "obj->b.changed"
  ["b"]=>
  &string(14) "obj->b.changed"
  ["c"]=>
  string(14) "obj->c.changed"
}


--- b refs c:
object(stdClass)#%d (3) {
  ["a"]=>
  int(1)
  ["c"]=>
  &int(1)
  ["b"]=>
  &int(1)
}
string(52) "c71701a8737464436c61737303a16101a163d40301a162d40401"
object(stdClass)#%d (3) {
  ["a"]=>
  int(1)
  ["c"]=>
  &int(1)
  ["b"]=>
  &int(1)
}
object(stdClass)#%d (3) {
  ["a"]=>
  string(14) "obj->a.changed"
  ["c"]=>
  &int(1)
  ["b"]=>
  &int(1)
}
object(stdClass)#%d (3) {
  ["a"]=>
  string(14) "obj->a.changed"
  ["c"]=>
  &string(14) "obj->b.changed"
  ["b"]=>
  &string(14) "obj->b.changed"
}
object(stdClass)#%d (3) {
  ["a"]=>
  string(14) "obj->a.changed"
  ["c"]=>
  &string(14) "obj->c.changed"
  ["b"]=>
  &string(14) "obj->c.changed"
}


--- c refs a:
object(stdClass)#%d (3) {
  ["a"]=>
  &int(1)
  ["b"]=>
  int(1)
  ["c"]=>
  &int(1)
}
string(52) "c71701a8737464436c61737303a161d40301a16201a163d40401"
object(stdClass)#%d (3) {
  ["a"]=>
  &int(1)
  ["b"]=>
  int(1)
  ["c"]=>
  &int(1)
}
object(stdClass)#%d (3) {
  ["a"]=>
  &string(14) "obj->a.changed"
  ["b"]=>
  int(1)
  ["c"]=>
  &string(14) "obj->a.changed"
}
object(stdClass)#%d (3) {
  ["a"]=>
  &string(14) "obj->a.changed"
  ["b"]=>
  string(14) "obj->b.changed"
  ["c"]=>
  &string(14) "obj->a.changed"
}
object(stdClass)#%d (3) {
  ["a"]=>
  &string(14) "obj->c.changed"
  ["b"]=>
  string(14) "obj->b.changed"
  ["c"]=>
  &string(14) "obj->c.changed"
}


--- c refs b:
object(stdClass)#%d (3) {
  ["a"]=>
  int(1)
  ["b"]=>
  &int(1)
  ["c"]=>
  &int(1)
}
string(52) "c71701a8737464436c61737303a16101a162d40301a163d40401"
object(stdClass)#%d (3) {
  ["a"]=>
  int(1)
  ["b"]=>
  &int(1)
  ["c"]=>
  &int(1)
}
object(stdClass)#%d (3) {
  ["a"]=>
  string(14) "obj->a.changed"
  ["b"]=>
  &int(1)
  ["c"]=>
  &int(1)
}
object(stdClass)#%d (3) {
  ["a"]=>
  string(14) "obj->a.changed"
  ["b"]=>
  &string(14) "obj->b.changed"
  ["c"]=>
  &string(14) "obj->b.changed"
}
object(stdClass)#%d (3) {
  ["a"]=>
  string(14) "obj->a.changed"
  ["b"]=>
  &string(14) "obj->c.changed"
  ["c"]=>
  &string(14) "obj->c.changed"
}


--- a,b refs c:
object(stdClass)#%d (3) {
  ["c"]=>
  &int(1)
  ["a"]=>
  &int(1)
  ["b"]=>
  &int(1)
}
string(56) "c71901a8737464436c61737303a163d40301a161d40401a162d40401"
object(stdClass)#%d (3) {
  ["c"]=>
  &int(1)
  ["a"]=>
  &int(1)
  ["b"]=>
  &int(1)
}
object(stdClass)#%d (3) {
  ["c"]=>
  &string(14) "obj->a.changed"
  ["a"]=>
  &string(14) "obj->a.changed"
  ["b"]=>
  &string(14) "obj->a.changed"
}
object(stdClass)#%d (3) {
  ["c"]=>
  &string(14) "obj->b.changed"
  ["a"]=>
  &string(14) "obj->b.changed"
  ["b"]=>
  &string(14) "obj->b.changed"
}
object(stdClass)#%d (3) {
  ["c"]=>
  &string(14) "obj->c.changed"
  ["a"]=>
  &string(14) "obj->c.changed"
  ["b"]=>
  &string(14) "obj->c.changed"
}


--- a,c refs b:
object(stdClass)#%d (3) {
  ["b"]=>
  &int(1)
  ["a"]=>
  &int(1)
  ["c"]=>
  &int(1)
}
string(56) "c71901a8737464436c61737303a162d40301a161d40401a163d40401"
object(stdClass)#%d (3) {
  ["b"]=>
  &int(1)
  ["a"]=>
  &int(1)
  ["c"]=>
  &int(1)
}
object(stdClass)#%d (3) {
  ["b"]=>
  &string(14) "obj->a.changed"
  ["a"]=>
  &string(14) "obj->a.changed"
  ["c"]=>
  &string(14) "obj->a.changed"
}
object(stdClass)#%d (3) {
  ["b"]=>
  &string(14) "obj->b.changed"
  ["a"]=>
  &string(14) "obj->b.changed"
  ["c"]=>
  &string(14) "obj->b.changed"
}
object(stdClass)#%d (3) {
  ["b"]=>
  &string(14) "obj->c.changed"
  ["a"]=>
  &string(14) "obj->c.changed"
  ["c"]=>
  &string(14) "obj->c.changed"
}


--- b,c refs a:
object(stdClass)#%d (3) {
  ["a"]=>
  &int(1)
  ["b"]=>
  &int(1)
  ["c"]=>
  &int(1)
}
string(56) "c71901a8737464436c61737303a161d40301a162d40401a163d40401"
object(stdClass)#%d (3) {
  ["a"]=>
  &int(1)
  ["b"]=>
  &int(1)
  ["c"]=>
  &int(1)
}
object(stdClass)#%d (3) {
  ["a"]=>
  &string(14) "obj->a.changed"
  ["b"]=>
  &string(14) "obj->a.changed"
  ["c"]=>
  &string(14) "obj->a.changed"
}
object(stdClass)#%d (3) {
  ["a"]=>
  &string(14) "obj->b.changed"
  ["b"]=>
  &string(14) "obj->b.changed"
  ["c"]=>
  &string(14) "obj->b.changed"
}
object(stdClass)#%d (3) {
  ["a"]=>
  &string(14) "obj->c.changed"
  ["b"]=>
  &string(14) "obj->c.changed"
  ["c"]=>
  &string(14) "obj->c.changed"
}
Done
