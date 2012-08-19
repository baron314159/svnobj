--TEST--
Compute file difference summary (peg revision)
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
var_dump($client->diffSummarizePeg(
	TEST_REPO_URL,
	array(SvnClient::OPT_REVISION_NUMBER, 5),
	array(SvnClient::OPT_REVISION_NUMBER, 5),
	array(SvnClient::OPT_REVISION_NUMBER, 6)));
?>
--EXPECTF--
array(1) {
  [0]=>
  array(4) {
    ["path"]=>
    string(%d) "%s"
    ["summarize_kind"]=>
    int(%d)
    ["prop_changed"]=>
    bool(%s)
    ["node_kind"]=>
    int(%d)
  }
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
