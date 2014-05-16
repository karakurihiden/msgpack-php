--TEST--
Unserialize array of objects whose class can't be autoloaded fail
--FILE--
<?php
class A {
    public function __wakeup() {
        throw new Exception("Failed");
    }
}

spl_autoload_register(
    function($class) {
        throw new Exception("Failed");
    }
);

try {
    var_dump(MessagePack\unserialize(pack('H*','8200c70301a1410001c70301a14200')));
} catch (Exception $e) {
    var_dump($e->getMessage());
}
?>
--EXPECTF--
string(6) "Failed"
