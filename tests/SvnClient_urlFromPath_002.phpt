--TEST--
urlFromPath works with a url
--FILE--
<?php
include 'bootstrap.php';
$url = SvnClient::urlFromPath(TEST_REPO_URL . '/justaline.txt');
var_dump($url);
?>
--EXPECTF--
string(%d) "file:///%s/justaline.txt"
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
