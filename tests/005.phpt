--TEST--
serialize()/unserialize() objects
--SKIPIF--
<?php if (!interface_exists('Serializable')) die('skip Interface Serialzable not defined');
?>
--FILE--
<?php
namespace MessagePack;
// This test verifies that old and new style (un)serializing do not interfere.

function do_autoload($class_name)
{
    if ($class_name != __NAMESPACE__.'\autoload_not_available')
    {
        $c = strrchr($class_name, '\\');
        if ($c !== false) {
            $class_name = substr($c, 1);
        }
        require_once(dirname(__FILE__) . '/' . strtolower($class_name) . '.p5c');
    }
    echo __FUNCTION__ . "($class_name)\n";
}

function unserializer($class_name)
{
    echo __METHOD__ . "($class_name)\n";
    switch($class_name)
    {
    case __NAMESPACE__.'\TestNAOld':
        eval("namespace MessagePack; class TestNAOld extends TestOld {}");
        break;
    case __NAMESPACE__.'\TestNANew':
        eval("namespace MessagePack; class TestNANew extends TestNew {}");
        break;
    case __NAMESPACE__.'\TestNANew2':
        eval("namespace MessagePack; class TestNANew2 extends TestNew {}");
        break;
    default:
        echo "Try __autoload()\n";
        if (!function_exists('__autoload'))
        {
            eval('function __autoload($class_name) { '.__NAMESPACE__.'\do_autoload($class_name); }');
        }
        __autoload($class_name);
        break;
    }
}

ini_set('unserialize_callback_func', __NAMESPACE__.'\unserializer');

class TestOld
{
    function serialize()
    {
        echo __METHOD__ . "()\n";
    }

    function unserialize($serialized)
    {
        echo __METHOD__ . "()\n";
    }

    function __wakeup()
    {
        echo __METHOD__ . "()\n";
    }

    function __sleep()
    {
        echo __METHOD__ . "()\n";
        return array();
    }
}

class TestNew implements \Serializable
{
    protected static $check = 0;

    function serialize()
    {
        echo __METHOD__ . "()\n";
        switch(++self::$check)
        {
        case 1:
            return NULL;
        case 2:
            return "2";
        }
    }

    function unserialize($serialized)
    {
        echo __METHOD__ . "()\n";
    }

    function __wakeup()
    {
        echo __METHOD__ . "()\n";
    }

    function __sleep()
    {
        echo __METHOD__ . "()\n";
    }
}

echo "===O1===\n";
$ser = serialize(new TestOld);
var_dump(bin2hex($ser));
var_dump(unserialize($ser));

echo "===N1===\n";
$ser = serialize(new TestNew);
var_dump(bin2hex($ser));
var_dump(unserialize($ser));

echo "===N2===\n";
$ser = serialize(new TestNew);
var_dump(bin2hex($ser));
var_dump(unserialize($ser));

echo "===NAOld===\n";
var_dump(unserialize(pack('H*','c71701b54d6573736167655061636b5c546573744e414f6c6400')));

echo "===NANew===\n";
var_dump(unserialize(pack('H*','c71701b54d6573736167655061636b5c546573744e414e657700')));

echo "===NANew2===\n";
var_dump(unserialize(pack('H*','c71802b64d6573736167655061636b5c546573744e414e65773200')));

echo "===AutoOld===\n";
var_dump(unserialize(pack('H*','c72101bf4d6573736167655061636b5c6175746f6c6f61645f696d706c656d656e747300')));

// Now we have __autoload(), that will be called before the old style header.
// If the old style handler also fails to register the class then the object
// becomes an incomplete class instance.

echo "===AutoNA===\n";
var_dump(unserialize(pack('H*','c72501d9224d6573736167655061636b5c6175746f6c6f61645f6e6f745f617661696c61626c6500')));
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
===O1===
MessagePack\TestOld::__sleep()
string(48) "c71501b34d6573736167655061636b5c546573744f6c6400"
MessagePack\TestOld::__wakeup()
object(MessagePack\TestOld)#%d (0) {
}
===N1===
MessagePack\TestNew::serialize()
string(2) "c0"
NULL
===N2===
MessagePack\TestNew::serialize()
string(50) "c71602b34d6573736167655061636b5c546573744e6577a132"
MessagePack\TestNew::unserialize()
object(MessagePack\TestNew)#%d (0) {
}
===NAOld===
MessagePack\unserializer(MessagePack\TestNAOld)
MessagePack\TestOld::__wakeup()
object(MessagePack\TestNAOld)#%d (0) {
}
===NANew===
MessagePack\unserializer(MessagePack\TestNANew)
MessagePack\TestNew::__wakeup()
object(MessagePack\TestNANew)#%d (0) {
}
===NANew2===
MessagePack\unserializer(MessagePack\TestNANew2)
MessagePack\TestNew::unserialize()
object(MessagePack\TestNANew2)#%d (0) {
}
===AutoOld===
MessagePack\unserializer(MessagePack\autoload_implements)
Try __autoload()
MessagePack\do_autoload(autoload_interface)
MessagePack\do_autoload(autoload_implements)
object(MessagePack\autoload_implements)#%d (0) {
}
===AutoNA===
MessagePack\do_autoload(MessagePack\autoload_not_available)
MessagePack\unserializer(MessagePack\autoload_not_available)
Try __autoload()
MessagePack\do_autoload(MessagePack\autoload_not_available)
MessagePack\do_autoload(MessagePack\autoload_not_available)

Warning: MessagePack\unserialize(): Function MessagePack\unserializer() hasn't defined the class it was called for in %s on line %d
object(__PHP_Incomplete_Class)#%d (1) {
  ["__PHP_Incomplete_Class_Name"]=>
  string(34) "MessagePack\autoload_not_available"
}
===DONE===
