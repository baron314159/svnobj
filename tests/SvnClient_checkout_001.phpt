--TEST--
Simple checkout
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
echo_file_contents('justaline.txt');
?>
--EXPECT--
this is a line

--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
