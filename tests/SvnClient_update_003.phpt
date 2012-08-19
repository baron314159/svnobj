--TEST--
Update to specific revision
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, 
	array(SvnClient::OPT_REVISION_NUMBER, 1));
echo_file_exists('numbers.txt');
$client->update(TEST_REPO_CO_DIR, 
	array(SvnClient::OPT_REVISION_NUMBER, 2));
echo_file_exists('numbers.txt');
echo_file_exists('sports/soccer');
?>
--EXPECT--
0
1
0
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
