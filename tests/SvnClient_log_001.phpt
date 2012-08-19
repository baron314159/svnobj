--TEST--
Log via URL
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
var_dump($client->log(TEST_REPO_URL, 
	null, 
	array(SvnClient::OPT_REVISION_NUMBER, 1), 
	array(SvnClient::OPT_REVISION_NUMBER, 1), 
	0, 
	true));
?>
--EXPECTF--
array(1) {
  [0]=>
  array(5) {
    ["changed_paths"]=>
    array(1) {
      ["/justaline.txt"]=>
      array(3) {
        ["action"]=>
        string(1) "A"
        ["copyfrom_path"]=>
        NULL
        ["copyfrom_rev"]=>
        int(-1)
      }
    }
    ["revision"]=>
    int(1)
    ["author"]=>
    string(%d) "%s"
    ["date"]=>
    string(%d) "%s"
    ["message"]=>
    string(14) "mktestrepo - 1"
  }
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
