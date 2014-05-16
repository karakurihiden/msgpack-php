--TEST--
__autoload() not invoked for interfaces
--SKIPIF--
<?php
    if (class_exists('autoload_root', false)) die('skip Autoload test classes exist already');
?>
--FILE--
<?php

function __autoload($class_name)
{
    $c = strrchr($class_name, '\\');
    if ($c !== false) {
        $class_name = substr($c, 1);
    }
    require_once(dirname(__FILE__) . '/' . strtolower($class_name) . '.p5c');
    echo __FUNCTION__ . '(' . $class_name . ")\n";
}

var_dump(interface_exists('MessagePack\autoload_interface', false));
var_dump(class_exists('MessagePack\autoload_implements', false));

$o = MessagePack\unserialize(pack('H*','c72101bf4d6573736167655061636b5c4175746f6c6f61645f496d706c656d656e747300'));

var_dump($o);
var_dump($o instanceof MessagePack\autoload_interface);
unset($o);

var_dump(interface_exists('MessagePack\autoload_interface', false));
var_dump(class_exists('MessagePack\autoload_implements', false));

?>
===DONE===
--EXPECTF--
bool(false)
bool(false)
__autoload(autoload_interface)
__autoload(Autoload_Implements)
object(MessagePack\autoload_implements)#%d (0) {
}
bool(true)
bool(true)
bool(true)
===DONE===
