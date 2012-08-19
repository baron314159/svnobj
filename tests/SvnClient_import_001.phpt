--TEST--
Import a file into a repository
--FILE--
<?php
function log_msg_callback() { return array('log_msg'=>'foo'); }
include 'bootstrap.php';
$client = new SvnClient();
$client->setLogMsgCallback('log_msg_callback');
set_test_auth_baton($client);
$tmp_file_rel = uniqid();
$tmp_url = TEST_REPO_URL . '/' . $tmp_file_rel;
$client->import(dirname(__FILE__) . DS .'bootstrap.php', $tmp_url);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
echo_file_exists($tmp_file_rel);
?>
--EXPECTF--
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
