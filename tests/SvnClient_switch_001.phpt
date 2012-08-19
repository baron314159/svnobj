--TEST--
Simple switch
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, 
	array(SvnClient::OPT_REVISION_NUMBER, 1));
echo_file_exists('numbers.txt');
echo_file_exists('sports/soccer');
$client->switch(TEST_REPO_CO_DIR, TEST_REPO_URL, 
	array(SvnClient::OPT_REVISION_NUMBER, 2));
echo_file_exists('numbers.txt');
echo_file_exists('sports/soccer');
$client->switch(TEST_REPO_CO_DIR, TEST_REPO_URL);
echo_file_exists('numbers.txt');
echo_file_exists('sports/soccer');
?>
--EXPECT--
0
0
1
0
1
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
