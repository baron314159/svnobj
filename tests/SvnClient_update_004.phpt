--TEST--
Update non-recursively
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, 1);
$client->update(TEST_REPO_CO_DIR, null, false);
echo_file_exists('sports/soccer');
?>
--EXPECT--
0
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
