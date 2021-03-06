--TEST--
Test serialize() & unserialize() functions: objects (variations)
--INI--
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

echo "\n--- Testing Variations in objects ---\n";

class members
{
  private $var_private = 10;
  protected $var_protected = "string";
  public $var_public = array(-100.123, "string", TRUE);
}

class nomembers { }

class C {
  var $a, $b, $c, $d, $e, $f, $g, $h;
  function __construct() {
    $this->a = 10;
    $this->b = "string";
    $this->c = TRUE;
    $this->d = -2.34444;
    $this->e = array(1, 2.22, "string", TRUE, array(),
                     new members(), null);
    $this->f = new nomembers();
    $this->g = $GLOBALS['file_handle'];
    $this->h = NULL;
  }
}

class D extends C {
  function __construct( $w, $x, $y, $z ) {
    $this->a = $w;
    $this->b = $x;
    $this->c = $y;
    $this->d = $z;
  }
}

$variation_obj_arr = array(
  new C(),
  new D( 1, 2, 3333, 444444 ),
  new D( .5, 0.005, -1.345, 10.005e5 ),
  new D( TRUE, true, FALSE, false ),
  new D( "a", 'a', "string", 'string' ),
  new D( array(),
         array(1, 2.222, TRUE, FALSE, "string"),
         array(new nomembers(), $file_handle, NULL, ""),
         array(array(1,2,3,array()))
       ),
  new D( NULL, null, "", "\0" ),
  new D( new members, new nomembers, $file_handle, NULL),
);

/* Testing serialization on all the objects through loop */
foreach( $variation_obj_arr as $object) {

  echo "After Serialization => ";
  $serialize_data = serialize( $object );
  var_dump( bin2hex($serialize_data) );

  echo "After Unserialization => ";
  $unserialize_data = unserialize( $serialize_data );
  var_dump( $unserialize_data );
}

echo "\nDone";
?>
--EXPECTF--
--- Testing Variations in objects ---

Notice: Undefined index: file_handle in %s on line 35

Notice: Undefined variable: file_handle in %s on line 57

Notice: Undefined variable: file_handle in %s on line 61
After Serialization => string(400) "c7c501ad4d6573736167655061636b5c4308a1610aa162a6737472696e67a163c3a164cbc002c169c23b7953a16587000101cb4001c28f5c28f5c302a6737472696e6703c3048005c75901b34d6573736167655061636b5c6d656d626572730302ab7661725f707269766174650a01ad7661725f70726f746563746564a6737472696e67aa7661725f7075626c69638300cbc05907df3b645a1d01a6737472696e6702c306c0a166c71701b54d6573736167655061636b5c6e6f6d656d6265727300a167c0a168c0"
After Unserialization => object(MessagePack\C)#%d (8) {
  ["a"]=>
  int(10)
  ["b"]=>
  string(6) "string"
  ["c"]=>
  bool(true)
  ["d"]=>
  float(-2.34444)
  ["e"]=>
  array(7) {
    [0]=>
    int(1)
    [1]=>
    float(2.22)
    [2]=>
    string(6) "string"
    [3]=>
    bool(true)
    [4]=>
    array(0) {
    }
    [5]=>
    object(MessagePack\members)#%d (3) {
      ["var_private":"MessagePack\members":private]=>
      int(10)
      ["var_protected":protected]=>
      string(6) "string"
      ["var_public"]=>
      array(3) {
        [0]=>
        float(-100.123)
        [1]=>
        string(6) "string"
        [2]=>
        bool(true)
      }
    }
    [6]=>
    NULL
  }
  ["f"]=>
  object(MessagePack\nomembers)#%d (0) {
  }
  ["g"]=>
  NULL
  ["h"]=>
  NULL
}
After Serialization => string(96) "c72d01ad4d6573736167655061636b5c4408a16101a16202a163cd0d05a164ce0006c81ca165c0a166c0a167c0a168c0"
After Unserialization => object(MessagePack\D)#%d (8) {
  ["a"]=>
  int(1)
  ["b"]=>
  int(2)
  ["c"]=>
  int(3333)
  ["d"]=>
  int(444444)
  ["e"]=>
  NULL
  ["f"]=>
  NULL
  ["g"]=>
  NULL
  ["h"]=>
  NULL
}
After Serialization => string(148) "c74701ad4d6573736167655061636b5c4408a161cb3fe0000000000000a162cb3f747ae147ae147ba163cbbff5851eb851eb85a164cb412e886800000000a165c0a166c0a167c0a168c0"
After Unserialization => object(MessagePack\D)#%d (8) {
  ["a"]=>
  float(0.5)
  ["b"]=>
  float(0.005)
  ["c"]=>
  float(-1.345)
  ["d"]=>
  float(1000500)
  ["e"]=>
  NULL
  ["f"]=>
  NULL
  ["g"]=>
  NULL
  ["h"]=>
  NULL
}
After Serialization => string(84) "c72701ad4d6573736167655061636b5c4408a161c3a162c3a163c2a164c2a165c0a166c0a167c0a168c0"
After Unserialization => object(MessagePack\D)#%d (8) {
  ["a"]=>
  bool(true)
  ["b"]=>
  bool(true)
  ["c"]=>
  bool(false)
  ["d"]=>
  bool(false)
  ["e"]=>
  NULL
  ["f"]=>
  NULL
  ["g"]=>
  NULL
  ["h"]=>
  NULL
}
After Serialization => string(112) "c73501ad4d6573736167655061636b5c4408a161a161a162a161a163a6737472696e67a164a6737472696e67a165c0a166c0a167c0a168c0"
After Unserialization => object(MessagePack\D)#%d (8) {
  ["a"]=>
  string(1) "a"
  ["b"]=>
  string(1) "a"
  ["c"]=>
  string(6) "string"
  ["d"]=>
  string(6) "string"
  ["e"]=>
  NULL
  ["f"]=>
  NULL
  ["g"]=>
  NULL
  ["h"]=>
  NULL
}
After Serialization => string(218) "c76a01ad4d6573736167655061636b5c4408a16180a16285000101cb4001c6a7ef9db22d02c303c204a6737472696e67a1638400c71701b54d6573736167655061636b5c6e6f6d656d626572730001c002c003a0a1648100840001010202030380a165c0a166c0a167c0a168c0"
After Unserialization => object(MessagePack\D)#%d (8) {
  ["a"]=>
  array(0) {
  }
  ["b"]=>
  array(5) {
    [0]=>
    int(1)
    [1]=>
    float(2.222)
    [2]=>
    bool(true)
    [3]=>
    bool(false)
    [4]=>
    string(6) "string"
  }
  ["c"]=>
  array(4) {
    [0]=>
    object(MessagePack\nomembers)#%d (0) {
    }
    [1]=>
    NULL
    [2]=>
    NULL
    [3]=>
    string(0) ""
  }
  ["d"]=>
  array(1) {
    [0]=>
    array(4) {
      [0]=>
      int(1)
      [1]=>
      int(2)
      [2]=>
      int(3)
      [3]=>
      array(0) {
      }
    }
  }
  ["e"]=>
  NULL
  ["f"]=>
  NULL
  ["g"]=>
  NULL
  ["h"]=>
  NULL
}
After Serialization => string(86) "c72801ad4d6573736167655061636b5c4408a161c0a162c0a163a0a164a100a165c0a166c0a167c0a168c0"
After Unserialization => object(MessagePack\D)#%d (8) {
  ["a"]=>
  NULL
  ["b"]=>
  NULL
  ["c"]=>
  string(0) ""
  ["d"]=>
  string(1) " "
  ["e"]=>
  NULL
  ["f"]=>
  NULL
  ["g"]=>
  NULL
  ["h"]=>
  NULL
}
After Serialization => string(316) "c79b01ad4d6573736167655061636b5c4408a161c75901b34d6573736167655061636b5c6d656d626572730302ab7661725f707269766174650a01ad7661725f70726f746563746564a6737472696e67aa7661725f7075626c69638300cbc05907df3b645a1d01a6737472696e6702c3a162c71701b54d6573736167655061636b5c6e6f6d656d6265727300a163c0a164c0a165c0a166c0a167c0a168c0"
After Unserialization => object(MessagePack\D)#%d (8) {
  ["a"]=>
  object(MessagePack\members)#%d (3) {
    ["var_private":"MessagePack\members":private]=>
    int(10)
    ["var_protected":protected]=>
    string(6) "string"
    ["var_public"]=>
    array(3) {
      [0]=>
      float(-100.123)
      [1]=>
      string(6) "string"
      [2]=>
      bool(true)
    }
  }
  ["b"]=>
  object(MessagePack\nomembers)#%d (0) {
  }
  ["c"]=>
  NULL
  ["d"]=>
  NULL
  ["e"]=>
  NULL
  ["f"]=>
  NULL
  ["g"]=>
  NULL
  ["h"]=>
  NULL
}

Done
