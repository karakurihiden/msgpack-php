--TEST--
phpinfo() displays msgpack info
--FILE--
<?php
phpinfo();
--EXPECTF--
%a
msgpack

MessagePack support => enabled
Extension Version => %d.%d.%d%a
