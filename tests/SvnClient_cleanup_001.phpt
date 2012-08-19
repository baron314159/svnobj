--TEST--
Cleanup a working copy
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
var_dump($client->cleanup(TEST_REPO_CO_DIR));
?>
--EXPECT--
bool(true)
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
