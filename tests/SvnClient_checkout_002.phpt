--TEST--
Checkout a specific revision
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, 1);
echo_file_exists('numbers.txt');
?>
--EXPECT--
0
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
