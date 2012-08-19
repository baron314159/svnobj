--TEST--
Revert local modifications
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$path = TEST_REPO_CO_DIR . DS . 'sports' . DS . 'soccer';
file_put_contents($path, 'xyz');
echo_file_contents($path);
echo " ";
$client->revert(TEST_REPO_CO_DIR, true);
echo_file_contents($path);
?>
--EXPECT--
xyz soccer
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
