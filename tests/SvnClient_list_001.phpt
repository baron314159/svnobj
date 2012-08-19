--TEST--
Listing a directory
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);

$contents = $client->list(TEST_REPO_URL . '/sports', null, array(SvnClient::OPT_REVISION_NUMBER, 3));
var_dump($contents);
?>
--EXPECTF--
array(3) {
  [0]=>
  array(7) {
    ["path"]=>
    string(0) ""
    ["kind"]=>
    int(2)
    ["size"]=>
    int(0)
    ["has_props"]=>
    bool(%s)
    ["created_rev"]=>
    int(3)
    ["time"]=>
    int(%d)
    ["last_author"]=>
    string(%d) "%s"
  }
  [1]=>
  array(7) {
    ["path"]=>
    string(8) "baseball"
    ["kind"]=>
    int(1)
    ["size"]=>
    int(9)
    ["has_props"]=>
    bool(%s)
    ["created_rev"]=>
    int(3)
    ["time"]=>
    int(%d)
    ["last_author"]=>
    string(%d) "%s"
  }
  [2]=>
  array(7) {
    ["path"]=>
    string(6) "soccer"
    ["kind"]=>
    int(1)
    ["size"]=>
    int(7)
    ["has_props"]=>
    bool(%s)
    ["created_rev"]=>
    int(3)
    ["time"]=>
    int(%d)
    ["last_author"]=>
    string(%d) "%s"
  }
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
