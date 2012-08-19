--TEST--
urlFromPath works with a directory
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$url = SvnClient::urlFromPath(TEST_REPO_CO_DIR . DS . 'justaline.txt');
var_dump($url);
cleanup();
?>
--EXPECTF--
string(%d) "file:///%s/justaline.txt"
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
