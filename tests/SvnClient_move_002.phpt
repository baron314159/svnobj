--TEST--
Move in WC
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$file = TEST_REPO_CO_DIR . DS . uniqid();
$client->move(TEST_REPO_CO_DIR . DS . 'justaline.txt', $file);
echo_file_contents($file);
?>
--EXPECT--
this is a line
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
