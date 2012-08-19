--TEST--
Ensure getChildErrors works
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
try {
	$client->checkout('file:///zzz' . uniqid(), TEST_REPO_CO_DIR);
} catch (SvnLibraryException $e) {
	var_dump($e->getChildErrors());
}
?>
--EXPECTF--
array(1) {
  [0]=>
  array(2) {
    ["message"]=>
    string(%d) "%s"
    ["code"]=>
    int(%d)
  }
}
