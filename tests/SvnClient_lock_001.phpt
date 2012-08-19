--TEST--
Lock and unlock a file
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$file_url = TEST_REPO_URL . '/justaline.txt';
$client->lock($file_url, "comment here.");
$info = $client->info($file_url);
var_dump($info['justaline.txt']['lock']);
$client->unlock($file_url);
$info = $client->info($file_url);
var_dump($info['justaline.txt']['lock']);
?>
--EXPECTF--
array(7) {
  ["path"]=>
  string(14) "/justaline.txt"
  ["token"]=>
  string(%d) "%s"
  ["owner"]=>
  string(%d) "%s"
  ["comment"]=>
  string(13) "comment here."
  ["is_dav_comment"]=>
  bool(false)
  ["creation_date"]=>
  string(%d) "%s"
  ["expiration_date"]=>
  string(%d) "%s"
}
NULL
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
