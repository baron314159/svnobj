--TEST--
Delete a file via URL
--FILE--
<?php
function log_msg_callback($baton, $commit_items) {
	return array('log_msg' => 'log message.');
}
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$client->setLogMsgCallback('log_msg_callback');
$tmp_file_rel = uniqid();
$tmp_file = TEST_REPO_CO_DIR . DS . $tmp_file_rel;
file_put_contents($tmp_file, 'content');
$client->add($tmp_file);
$client->commit(TEST_REPO_CO_DIR);
echo_file_exists($tmp_file_rel);
cleanup();
$client->delete(TEST_REPO_URL . '/' . $tmp_file_rel);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
echo_file_exists($tmp_file_rel);
?>
--EXPECTF--
1
0
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
