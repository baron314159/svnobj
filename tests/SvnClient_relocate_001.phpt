--TEST--
Relocate to a branch
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$client->relocate(TEST_REPO_CO_DIR, TEST_REPO_URL, TEST_REPO_RELOC_URL);
?>
--EXPECT--
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
