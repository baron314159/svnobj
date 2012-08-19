--TEST--
Commit items array
--FILE--
<?php
include 'bootstrap.php';
function log_msg_callback($baton, $commit_items) {
	var_dump($commit_items);
	return array('log_msg' => 'msg');
}
$client = new SvnClient();
set_test_auth_baton($client);
$client->setLogMsgCallback('log_msg_callback');
$target = TEST_REPO_CO_DIR . DS . 'numbers.txt';
$prop_name = 'svnobj:' . uniqid();
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$client->propSet($prop_name, 'test', $target);
$client->commit(TEST_REPO_CO_DIR);
?>
--EXPECTF--
array(1) {
  [0]=>
  array(8) {
    ["path"]=>
    string(%d) "%snumbers.txt"
    ["kind"]=>
    int(1)
    ["url"]=>
    string(%d) "%snumbers.txt"
    ["revision"]=>
    int(%d)
    ["copyfrom_url"]=>
    NULL
    ["copyfrom_rev"]=>
    int(-1)
    ["state_flags"]=>
    int(%d)
    ["wcprop_changes"]=>
    array(0) {
    }
  }
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
