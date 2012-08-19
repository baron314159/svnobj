--TEST--
Checkout recursive and non-recursive
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, array(SvnClient::OPT_REVISION_NUMBER, '3'), false);
echo_file_exists('sports' . DS . 'soccer');
cleanup();
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, array(SvnClient::OPT_REVISION_NUMBER, '3'), true);
echo_file_exists('sports' . DS . 'soccer');
?>
--EXPECT--
0
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
