--TEST--
Copy via URL
--FILE--
<?php
include 'bootstrap.php';
function log_msg_callback() { return array('log_msg' => 'a msg'); }
$client = new SvnClient();
set_test_auth_baton($client);
$client->setLogMsgCallback('log_msg_callback');
$file = uniqid();
var_dump($client->copy(TEST_REPO_URL . '/justaline.txt', TEST_REPO_URL . '/' . $file));
?>
--EXPECTF--
array(4) {
  ["revision"]=>
  int(%d)
  ["date"]=>
  string(%d) "%d-%d-%dT%d:%d:%d.%dZ"
  ["author"]=>
  string(%d) "%s"
  ["post_commit_err"]=>
  NULL
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
