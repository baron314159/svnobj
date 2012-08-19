--TEST--
List properties via URL
--FILE--
<?php
include 'bootstrap.php';
function log_msg_callback() { return array('log_msg' => 'a msg'); }
$client = new SvnClient();
set_test_auth_baton($client);
var_dump($client->propList(TEST_REPO_URL . '/sports', null, null, true));
?>
--EXPECTF--
array(2) {
  ["%s/sports/baseball"]=>
  array(1) {
    ["svnobj:foo"]=>
    string(3) "foo"
  }
  ["%s/sports/soccer"]=>
  array(2) {
    ["svnobj:foo"]=>
    string(3) "foo"
    ["svnobj:bar"]=>
    string(3) "bar"
  }
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
