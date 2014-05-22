# MessagePack for PHP

This extension provide API for communicating with [MessagePack][msgpack]
serialization.

[MessagePack][msgpack] is a binary-based efficient object serialization library.
It enables to exchange structured objects between many languages like JSON.
But unlike JSON, it is very fast and small.

[msgpack]: http://msgpack.org/

## Requirement

* PHP 5.3 +

## RPM Package

[Related article](http://hiden.samurai-factory.jp/php/msgpack/download)

* CentOS 6: [php-pecl-msgpack-0.1.1-0.el6.sf.x86_64.rpm](http://hiden.samurai-factory.jp/download/rpm/el6/php-pecl-msgpack-0.1.1-0.el6.sf.x86_64.rpm)
* Fedora 20: [php-pecl-msgpack-0.1.1-0.fc20.sf.x86_64.rpm](http://hiden.samurai-factory.jp/download/rpm/fc20/php-pecl-msgpack-0.1.1-0.fc20.sf.x86_64.rpm)

## Windows DLL

* VC11 x86 Non Thread Safe: PHP 5.5 (5.5.12): [php-msgpack-0.1.1-5.5-nts-Win32-VC11-x86.zip](http://hiden.samurai-factory.jp/download/dll/php-msgpack-0.1.1-5.5-nts-Win32-VC11-x86.zip)
* VC11 x86 Thread Safe: PHP 5.5 (5.5.12): [php-msgpack-0.1.1-5.5-Win32-VC11-x86.zip](http://hiden.samurai-factory.jp/download/dll/php-msgpack-0.1.1-5.5-Win32-VC11-x86.zip)

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
