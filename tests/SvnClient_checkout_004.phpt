--TEST--
Checkout by date
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$pre_commit_time = time();
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
list($file_rel, $file) = uniq_file_put_contents('content');
$client->add($file);
$client->commit(TEST_REPO_CO_DIR, "msg");
cleanup();
$client->checkout(TEST_REPO_URL,
	TEST_REPO_CO_DIR,
	SvnClient::OPT_REVISION_HEAD,
	array(SvnClient::OPT_REVISION_DATE, $pre_commit_time));
echo_file_exists($file_rel);
$client->update(TEST_REPO_CO_DIR,
	array(SvnClient::OPT_REVISION_DATE, time()));
echo_file_exists($file_rel);
?>
--EXPECT--
0
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
