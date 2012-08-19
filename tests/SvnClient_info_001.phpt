--TEST--
Info via URL
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
var_dump($client->info(TEST_REPO_URL . '/justaline.txt'));
?>
--EXPECTF--
array(1) {
  ["justaline.txt"]=>
  array(10) {
    ["URL"]=>
    string(%d) "%s/justaline.txt"
    ["rev"]=>
    int(%d)
    ["kind"]=>
    int(1)
    ["repos_root_URL"]=>
    string(%d) "%s"
    ["repos_UUID"]=>
    string(%d) "%s"
    ["last_changed_rev"]=>
    int(%d)
    ["last_changed_date"]=>
    string(%d) "%s"
    ["last_changed_author"]=>
    string(%d) "%s"
    ["lock"]=>
    NULL
    ["has_wc_info"]=>
    bool(false)
  }
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
