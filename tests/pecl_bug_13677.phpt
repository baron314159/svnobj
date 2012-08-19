--TEST--
Most paths are not canonicalized (pecl bug 13677)
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$tmp_file = TEST_REPO_CO_DIR . '/./' . uniqid();
file_put_contents($tmp_file, "zzz");
$client->add($tmp_file);
var_dump($client->commit(TEST_REPO_CO_DIR, "adding something."));
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
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
