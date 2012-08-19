--TEST--
Move via URL
--FILE--
<?php
include 'bootstrap.php';
function log_msg_callback() { return array('log_msg' => 'a msg'); }
$client = new SvnClient();
set_test_auth_baton($client);
$client->setLogMsgCallback('log_msg_callback');
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
list($file_rel, $file) = uniq_file_put_contents('content');
$client->add($file);
$client->commit(TEST_REPO_CO_DIR);
cleanup();
$new_loc = uniqid();
var_dump($client->move(TEST_REPO_URL . '/' . $file_rel, TEST_REPO_URL . '/' . $new_loc));
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
