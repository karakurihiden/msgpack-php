--TEST--
Object serialization / unserialization with inherited and hidden properties.
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

Class A {
    private $APriv = "A.APriv";
    protected $AProt = "A.AProt";
    public $APub = "A.APub";

    function audit() {
        return isset($this->APriv, $this->AProt, $this->APub);
    }
}

Class B extends A {
    private $BPriv = "B.BPriv";
    protected $BProt = "B.BProt";
    public $BPub = "B.BPub";

    function audit() {
        return  parent::audit() && isset($this->AProt, $this->APub,
                     $this->BPriv, $this->BProt, $this->BPub);
    }
}

Class C extends B {
    private $APriv = "C.APriv";
    protected $AProt = "C.AProt";
    public $APub = "C.APub";

    private $CPriv = "C.CPriv";
    protected $CProt = "C.BProt";
    public $CPub = "C.CPub";

    function audit() {
        return parent::audit() && isset($this->APriv, $this->AProt, $this->APub,
                     $this->BProt, $this->BPub,
                     $this->CPriv, $this->CProt, $this->CPub);
    }
}

function prettyPrint($obj) {
    echo "\n\nBefore serialization:\n";
    var_dump($obj);

    echo "Serialized form:\n";
    $ser = serialize($obj);
    var_dump(bin2hex($ser));

    echo "Unserialized:\n";
    $uobj = unserialize($ser);
    var_dump($uobj);

    echo "Sanity check: ";
    var_dump($uobj->audit());
}

echo "-- Test instance of A --\n";
prettyPrint(new A);
echo "\n\n-- Test instance of B --\n";
prettyPrint(new B);
echo "\n\n-- Test instance of C --\n";
prettyPrint(new C);

echo "Done";
?>
--EXPECTF--
-- Test instance of A --


Before serialization:
object(MessagePack\A)#%d (3) {
  ["APriv":"MessagePack\A":private]=>
  string(7) "A.APriv"
  ["AProt":protected]=>
  string(7) "A.AProt"
  ["APub"]=>
  string(6) "A.APub"
}
Serialized form:
string(120) "c73901ad4d6573736167655061636b5c410302a54150726976a7412e415072697601a54150726f74a7412e4150726f74a441507562a6412e41507562"
Unserialized:
object(MessagePack\A)#%d (3) {
  ["APriv":"MessagePack\A":private]=>
  string(7) "A.APriv"
  ["AProt":protected]=>
  string(7) "A.AProt"
  ["APub"]=>
  string(6) "A.APub"
}
Sanity check: bool(true)


-- Test instance of B --


Before serialization:
object(MessagePack\B)#%d (6) {
  ["BPriv":"MessagePack\B":private]=>
  string(7) "B.BPriv"
  ["BProt":protected]=>
  string(7) "B.BProt"
  ["BPub"]=>
  string(6) "B.BPub"
  ["APriv":"MessagePack\A":private]=>
  string(7) "A.APriv"
  ["AProt":protected]=>
  string(7) "A.AProt"
  ["APub"]=>
  string(6) "A.APub"
}
Serialized form:
string(232) "c77101ad4d6573736167655061636b5c420602a54250726976a7422e425072697601a54250726f74a7422e4250726f74a442507562a6422e42507562b4004d6573736167655061636b5c41004150726976a7412e415072697601a54150726f74a7412e4150726f74a441507562a6412e41507562"
Unserialized:
object(MessagePack\B)#%d (6) {
  ["BPriv":"MessagePack\B":private]=>
  string(7) "B.BPriv"
  ["BProt":protected]=>
  string(7) "B.BProt"
  ["BPub"]=>
  string(6) "B.BPub"
  ["APriv":"MessagePack\A":private]=>
  string(7) "A.APriv"
  ["AProt":protected]=>
  string(7) "A.AProt"
  ["APub"]=>
  string(6) "A.APub"
}
Sanity check: bool(true)


-- Test instance of C --


Before serialization:
object(MessagePack\C)#%d (10) {
  ["APriv":"MessagePack\C":private]=>
  string(7) "C.APriv"
  ["AProt":protected]=>
  string(7) "C.AProt"
  ["APub"]=>
  string(6) "C.APub"
  ["CPriv":"MessagePack\C":private]=>
  string(7) "C.CPriv"
  ["CProt":protected]=>
  string(7) "C.BProt"
  ["CPub"]=>
  string(6) "C.CPub"
  ["BPriv":"MessagePack\B":private]=>
  string(7) "B.BPriv"
  ["BProt":protected]=>
  string(7) "B.BProt"
  ["BPub"]=>
  string(6) "B.BPub"
  ["APriv":"MessagePack\A":private]=>
  string(7) "A.APriv"
}
Serialized form:
string(374) "c7b801ad4d6573736167655061636b5c430a02a54150726976a7432e415072697601a54150726f74a7432e4150726f74a441507562a6432e4150756202a54350726976a7432e435072697601a54350726f74a7432e4250726f74a443507562a6432e43507562b4004d6573736167655061636b5c42004250726976a7422e425072697601a54250726f74a7422e4250726f74a442507562a6422e42507562b4004d6573736167655061636b5c41004150726976a7412e4150726976"
Unserialized:
object(MessagePack\C)#%d (10) {
  ["APriv":"MessagePack\C":private]=>
  string(7) "C.APriv"
  ["AProt":protected]=>
  string(7) "C.AProt"
  ["APub"]=>
  string(6) "C.APub"
  ["CPriv":"MessagePack\C":private]=>
  string(7) "C.CPriv"
  ["CProt":protected]=>
  string(7) "C.BProt"
  ["CPub"]=>
  string(6) "C.CPub"
  ["BPriv":"MessagePack\B":private]=>
  string(7) "B.BPriv"
  ["BProt":protected]=>
  string(7) "B.BProt"
  ["BPub"]=>
  string(6) "B.BPub"
  ["APriv":"MessagePack\A":private]=>
  string(7) "A.APriv"
}
Sanity check: bool(true)
Done
