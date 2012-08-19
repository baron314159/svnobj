--TEST--
Make multiple directories via URL
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$dir_name1 = uniqid();
$dir_name2 = uniqid();
$new_urls = array(TEST_REPO_URL . '/' . $dir_name1, TEST_REPO_URL . '/' . $dir_name2); 
var_dump($client->mkdir($new_urls, "new directories"));
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
echo_file_exists($dir_name1);
echo_file_exists($dir_name2);
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
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
