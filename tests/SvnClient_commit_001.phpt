--TEST--
Commit 2 repos
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$ci_files = array();
for ($i=0; $i < 2; $i++) {
	$tmp_file = TEST_REPO_CO_DIR . DS . uniqid();
	file_put_contents($tmp_file, 'content');
	$client->add($tmp_file);
	$ci_files[] = $tmp_file;
}
var_dump($client->commit($ci_files, "adding something."));
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
