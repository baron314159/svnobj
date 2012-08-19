--TEST--
Ensure anonymous functions work with callbacks
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '5.3.0RC1') < 0) {
	die("skip - test is for 5.3.0 and above\n");
}
?>
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$x = "In callback\n";
$client->setLogMsgCallback(function($baton, $commit_items) use ($x) {
	echo $x;
	return array(
		'log_msg' => 'this is a message.'
	);
});
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$tmp_file_rel = uniqid();
$tmp_file = TEST_REPO_CO_DIR . DS . $tmp_file_rel;
file_put_contents($tmp_file, 'content');
$client->add($tmp_file);
var_dump($client->commit(TEST_REPO_CO_DIR));
?>
--EXPECTF--
In callback
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
