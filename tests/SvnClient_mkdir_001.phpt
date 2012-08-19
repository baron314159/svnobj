--TEST--
Make a directory via URL
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$dir_name = uniqid();
var_dump($client->mkdir(TEST_REPO_URL . '/' . $dir_name, "new directory"));
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
echo_file_exists($dir_name);
?>
--EXPECTF--
array(4) {
  ["revision"]=>
  int(%d)
  ["date"]=>
  string(%d) "%d-%d-%dT%d:%d:%d.%dZ"
  ["author"]=>
  string(%d) "%s"
  ["post_commit_err"]=>
  NULL
}
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
