--TEST--
Set and then get a property
--FILE--
<?php
include 'bootstrap.php';
function log_msg_callback() { return array('log_msg' => 'a msg'); }
$client = new SvnClient();
set_test_auth_baton($client);
$client->setLogMsgCallback('log_msg_callback');
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$prop_name = 'svnobj:' . uniqid();
$prop_value = "line1\nline2"; 
$target = TEST_REPO_CO_DIR . DS . 'justaline.txt';
$client->propSet($prop_name, $prop_value, $target);
$client->commit(TEST_REPO_CO_DIR);
cleanup();
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
var_dump($client->propGet($prop_name, $target));
?>
--EXPECTF--
array(1) {
  ["%sjustaline.txt"]=>
  string(11) "line1
line2"
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
