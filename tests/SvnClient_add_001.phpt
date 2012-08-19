--TEST--
Simple add / commit
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$tmp_file_rel = uniqid();
$tmp_file = TEST_REPO_CO_DIR . DS . $tmp_file_rel;
file_put_contents($tmp_file, 'content');
$client->add($tmp_file);
var_dump($client->commit(TEST_REPO_CO_DIR, "adding something."));
cleanup();
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
echo_file_contents($tmp_file_rel);
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
content
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
