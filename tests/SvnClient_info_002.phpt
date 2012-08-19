--TEST--
Info in WC
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
var_dump($client->info(TEST_REPO_CO_DIR . DS . 'justaline.txt'));
?>
--EXPECTF--
array(1) {
  ["/tmp/test_repo_co/justaline.txt"]=>
  array(20) {
    ["URL"]=>
    string(%d) "%s/justaline.txt"
    ["rev"]=>
    int(%s)
    ["kind"]=>
    int(%d)
    ["repos_root_URL"]=>
    string(%d) "%s"
    ["repos_UUID"]=>
    string(36) "%s"
    ["last_changed_rev"]=>
    int(%s)
    ["last_changed_date"]=>
    string(%d) "%s"
    ["last_changed_author"]=>
    string(%d) "%s"
    ["lock"]=>
    NULL
    ["has_wc_info"]=>
    bool(true)
    ["schedule"]=>
    int(%d)
    ["copyfrom_url"]=>
    NULL
    ["copyfrom_rev"]=>
    int(%s)
    ["text_time"]=>
    string(%d) "%s"
    ["prop_time"]=>
    string(%d) "%s"
    ["checksum"]=>
    string(%d) "%s"
    ["conflict_old"]=>
    NULL
    ["conflict_new"]=>
    NULL
    ["conflict_wrk"]=>
    NULL
    ["prejfile"]=>
    NULL
  }
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>
