# MessagePack for PHP

This extension provide API for communicating with [MessagePack][msgpack]
serialization.

[MessagePack][msgpack] is a binary-based efficient object serialization library.
It enables to exchange structured objects between many languages like JSON.
But unlike JSON, it is very fast and small.

[msgpack]: http://msgpack.org/

## Requirement

* PHP 5.3 +

## Build

```
% git clone --depth=1 https://github.com/karakurihiden/msgpack-php.git
% cd msgpack-php
% phpize
% ./configure
% make
% make test
% make install
```

## Configuration

msgpack.ini:

```
extension=msgpack.so
```


## Usage

```php
$data = ['a' => 1, 'b' => 'x', 'c' => true];
$msg = MessagePack\serialize($data);
$data = MessagePack\unserialize($msg);
```

## Related

* [Introductory article(JA)](http://hiden.samurai-factory.jp/php/msgpack)
