--TEST--
View blame on a file
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
var_dump($client->blame(TEST_REPO_URL . '/' . 'justaline.txt'));
?>
--EXPECTF--
array(1) {
  [0]=>
  array(5) {
    ["line_no"]=>
    int(0)
    ["revision"]=>
    int(1)
    ["author"]=>
    string(%d) "%s"
    ["date"]=>
    string(%d) "%d-%d-%dT%d:%d:%d.%dZ"
    ["line"]=>
    string(14) "this is a line"
  }
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
